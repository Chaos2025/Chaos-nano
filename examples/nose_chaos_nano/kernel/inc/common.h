/* SPDX-License-Identifier: MIT */
/******************************************************************************
*
* Name: common.h - General definition
*
* Copyright (C) 2025 Lijun An
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*****************************************************************************/
/**
* @file    common.h
* @author  Lijun An
* @date    2025-09-18
* @brief   General definition
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __IAR_SYSTEMS_ICC__  /* for IAR Compiler */
//    #define STATIC_INLINE __STATIC_INLINE
  #define STATIC_INLINE 
#else
  #define STATIC_INLINE static inline
#endif

typedef unsigned char bool;

#define false   0
#define true    (!false)
#define NULL    0
#define nullptr ((void*)0)

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 5000000) /* ARM Compiler */
  #define WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__) /* for IAR Compiler */
  #define WEAK __weak
#elif defined(__MINGW32__) /* MINGW32 Compiler */
  #define WEAK
#elif defined(__GNUC__) /* GNU GCC Compiler */
  #define WEAK __attribute__((weak))
#endif

#endif