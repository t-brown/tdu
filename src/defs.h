/* BSD 3-Clause License
 *
 * Copyright (c) 2018, Timothy Brown
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file defs.h
 * Internal definitions.
 *
 * \ingroup defs
 * \{
 **/

#ifndef DUT_DEFS_H
#define DUT_DEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* String localization */
#define _(x)              gettext(x)


/**
 * Compiler __attribute__ extensions
 **/
#ifdef HAVE___ATTRIBUTE__
  #define ATT_CONSTR       __attribute__((__constructor__))
  #define ATT_DESTR        __attribute__((__destructor__))
  #define ATT_PUBLIC       __attribute__((__visibility__("default")))
  #define ATT_LOCAL        __attribute__((__visibility__("hidden")))
  #define ATT_DEPRECATED   __attribute__((__deprecated__))
  #define ATT_MSIZE(x)     __attribute__((__alloc_size__(x)))
  #define ATT_MALLOC       __attribute__((__malloc__))
  #define ATT_FMT(x,y)     __attribute__((__format__(printf, x, y)))
  #define ATT_NORETURN     __attribute__((__noreturn__))
  #define ATT_INLINE       __attribute__((__always_inline__))
  #define ATT_ALIAS(x)     __attribute__((__weak__, __alias__(x)))
#else
  #define ATT_CONSTR
  #define ATT_DESTR
  #define ATT_PUBLIC
  #define ATT_LOCAL
  #define ATT_DEPRECATED(msg)
  #define ATT_MSIZE(x)
  #define ATT_MALLOC
  #define ATT_FMT(x,y)
  #define ATT_NORETURN
  #define ATT_INLINE
  #define ATT_ALIAS(x)
#endif


#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif                          /* DUT_DEFS_H */
/**
 * \}
 **/
