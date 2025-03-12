/*
 * BootMaster/pointer.h
 * pointer device functions header file
 *
 * Copyright (c) 2018 CJ Vaughter
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __REFINDPLUS_POINTERDEVICE_H_
#define __REFINDPLUS_POINTERDEVICE_H_

#ifdef __MAKEWITH_GNUEFI
#include "efi.h"
#include "efilib.h"
#else
#include "../include/tiano_includes.h"
#endif

#ifndef _EFI_POINT_H
#include "../EfiLib/AbsolutePointer.h"
#endif

typedef struct PointerStateStruct {
    UINTN X, Y;
    BOOLEAN Press;
    BOOLEAN Holding;
} POINTER_STATE;

#define INT32_MIN    ((INT32) 0x80000000)         // -2,147,483,648
#define INT32_MAX    ((INT32) 0x7FFFFFFF)         //  2,147,483,647

#define UINTN_MIN    ((UINTN) 0)                  //  Always 0
#if defined(EFI32)
#   define UINTN_MAX ((UINTN) 0xFFFFFFFF)         //  4,294,967,295
#else
#   define UINTN_MAX ((UINTN) 0xFFFFFFFFFFFFFFFF) //  18,446,744,073,709,551,615
#endif

VOID pdInitialize();
VOID pdCleanup();
BOOLEAN pdAvailable();
UINTN pdCount();
EFI_EVENT pdWaitEvent(IN UINTN Index);
EFI_STATUS pdUpdateState();
POINTER_STATE pdGetState();

VOID pdDraw();
VOID pdClear();

#endif

/* EOF */
