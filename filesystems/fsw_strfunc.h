/* Misc string encoding functions */

/*
 * Copyright (c) 2006 Christoph Pfisterer
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Refactored for RefindPlus
 * Copyright (c) 2025 Dayo Akanji (sf.net/u/dakanji/profile)
 *
 * Modifications distributed under the MIT License.
 */

// Helper Functions - Compare
static
fsw_u32 decode_utf8(
    fsw_u8 **p
) {
    fsw_u32 c = *(*p)++;
    if ((c & 0xe0) == 0xc0) {
        fsw_u32 c1 = *(*p)++;
        c = ((c & 0x1f) << 6) | (c1 & 0x3f);
    }
    else if ((c & 0xf0) == 0xe0) {
        fsw_u32 c1 = *(*p)++;
        fsw_u32 c2 = *(*p)++;
        c = ((c & 0x0f) << 12) | ((c1 & 0x3f) << 6) | (c2 & 0x3f);
    }
    else if ((c & 0xf8) == 0xf0) {
        fsw_u32 c1 = *(*p)++;
        fsw_u32 c2 = *(*p)++;
        fsw_u32 c3 = *(*p)++;
        c = ((c & 0x07) << 18) | ((c1 & 0x3f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f);
    }
    return c;
}

static
int compare_strings (
    void    *s1data,
    void    *s2data,
    int      len,
    int      encoding,
    int      swap
) {
    int      i;
    fsw_u32  c1, c2;
    fsw_u8  *p1 = (fsw_u8 *) s1data;
    fsw_u16 *p2 = (fsw_u16 *)s2data;

    for (i = 0; i < len; i++) {
        c1 = (encoding == 1) ? decode_utf8 (&p1) : *p1++;
        c2 = *p2++;
        if (swap) {
            c2 = FSW_SWAPVALUE_U16(c2);
        }

        if (c1 != c2) {
            return 0;
        }
    } // for

    return 1;
}
// Define Macro Base Functions - Compare
#define FSW_STREQ_FUNC(name, encoding, swap) \
static int name (void *s1data, void *s2data, int len) { \
    return compare_strings (s1data, s2data, len, encoding, swap); \
}
// Create Base Functions - Compare
FSW_STREQ_FUNC(fsw_streq_UTF8_UTF16, 1, 0)
FSW_STREQ_FUNC(fsw_streq_ISO88591_UTF8, 1, 0)
FSW_STREQ_FUNC(fsw_streq_ISO88591_UTF16, 0, 0)
FSW_STREQ_FUNC(fsw_streq_UTF8_UTF16_SWAPPED, 1, 1)
FSW_STREQ_FUNC(fsw_streq_UTF16_UTF16_SWAPPED, 0, 1)
FSW_STREQ_FUNC(fsw_streq_ISO88591_UTF16_SWAPPED, 0, 1)


// Helper Function - Coerce
static
fsw_status_t fsw_strcoerce_generic (
    void              *srcdata,
    int                srclen,
    struct fsw_string *dest,
    int                src_type,
    int                dest_type,
    int                swap
) {
    fsw_status_t       status;
    int                destsize, i;
    fsw_u8            *sp = (fsw_u8 *) srcdata;
    fsw_u8            *dp;
    fsw_u16           *dp16;
    fsw_u32            c;


    destsize = 0;

    if (dest_type == FSW_STRING_TYPE_UTF8) {
        for (i = 0; i < srclen; i++) {
            c = *sp++;
            if (0);
            else if (c < 0x000080) destsize += 1;
            else if (c < 0x000800) destsize += 2;
            else if (c < 0x010000) destsize += 3;
            else                   destsize += 4;
        } // for

        sp = (fsw_u8 *) srcdata;
    }

    dest->len  = srclen;
    dest->type = dest_type;
    dest->size = (dest_type == FSW_STRING_TYPE_UTF16)
        ? srclen * sizeof (fsw_u16)
        : (dest_type == FSW_STRING_TYPE_ISO88591)
            ? srclen
            : destsize; // FSW_STRING_TYPE_UTF8

    status = fsw_alloc (dest->size, &dest->data);
    if (status) {
        return status;
    }

    dp   = (fsw_u8 *)  dest->data;
    dp16 = (fsw_u16 *) dest->data;

    for (i = 0; i < srclen; i++) {
        c = *sp++;
        if (swap) {
            c = FSW_SWAPVALUE_U16(c);
        }

        if (dest_type == FSW_STRING_TYPE_UTF16) {
            *dp16++ = (fsw_u16) c;
        }
        else {
            if (c < 0x000080) {
                *dp++ = (fsw_u8) c;
            }
            else if (c < 0x000800) {
                *dp++ = (fsw_u8)(0xc0 | ((c >> 6) & 0x1f));
                *dp++ = (fsw_u8)(0x80 | (c & 0x3f));
            }
            else if (c < 0x010000) {
                *dp++ = (fsw_u8)(0xe0 | ((c >> 12) & 0x0f));
                *dp++ = (fsw_u8)(0x80 | ((c >> 6) & 0x3f));
                *dp++ = (fsw_u8)(0x80 | (c & 0x3f));
            }
            else {
                *dp++ = (fsw_u8)(0xf0 | ((c >> 18) & 0x07));
                *dp++ = (fsw_u8)(0x80 | ((c >> 12) & 0x3f));
                *dp++ = (fsw_u8)(0x80 | ((c >> 6) & 0x3f));
                *dp++ = (fsw_u8)(0x80 | (c & 0x3f));
            }
        }
    } // for

    return FSW_SUCCESS;
}
// Define Macro Base Functions - Coerce
#define FSW_STRCOERCE_FUNC(name, src_type, dest_type, swap) \
static fsw_status_t name(void *srcdata, int srclen, struct fsw_string *dest) { \
    return fsw_strcoerce_generic(srcdata, srclen, dest, src_type, dest_type, swap); \
}
// Create Base Functions - Coerce
FSW_STRCOERCE_FUNC(fsw_strcoerce_UTF16_UTF8, FSW_STRING_TYPE_UTF16, FSW_STRING_TYPE_UTF8, 0)
FSW_STRCOERCE_FUNC(fsw_strcoerce_UTF8_UTF16, FSW_STRING_TYPE_UTF8, FSW_STRING_TYPE_UTF16, 0)
FSW_STRCOERCE_FUNC(fsw_strcoerce_ISO88591_UTF8, FSW_STRING_TYPE_ISO88591, FSW_STRING_TYPE_UTF8, 0)
FSW_STRCOERCE_FUNC(fsw_strcoerce_UTF8_ISO88591, FSW_STRING_TYPE_UTF8, FSW_STRING_TYPE_ISO88591, 0)
FSW_STRCOERCE_FUNC(fsw_strcoerce_UTF16_ISO88591, FSW_STRING_TYPE_UTF16, FSW_STRING_TYPE_ISO88591, 0)
FSW_STRCOERCE_FUNC(fsw_strcoerce_ISO88591_UTF16, FSW_STRING_TYPE_ISO88591, FSW_STRING_TYPE_UTF16, 0)
FSW_STRCOERCE_FUNC(fsw_strcoerce_UTF16_SWAPPED_UTF8, FSW_STRING_TYPE_UTF16, FSW_STRING_TYPE_UTF8, 1)
FSW_STRCOERCE_FUNC(fsw_strcoerce_UTF16_SWAPPED_UTF16, FSW_STRING_TYPE_UTF16, FSW_STRING_TYPE_UTF16, 1)
FSW_STRCOERCE_FUNC(fsw_strcoerce_UTF16_SWAPPED_ISO88591, FSW_STRING_TYPE_UTF16, FSW_STRING_TYPE_ISO88591, 1)
