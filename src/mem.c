/*
 * BSD 3-Clause License
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
 * \file mem.c
 * Memory allocation and deallocation routines.
 *
 * \ingroup memory
 * \{
 **/


#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sysexits.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#include "defs.h"
#include "mem.h"

/**
 * Allocate a block of memory and set all entries to zero.
 * If there is an error in obtaining the memory err()
 * is called, terminating the program.
 *
 * \param[in] n The amount of memory in bytes.
 *
 * \return A pointer to the newly allocated memory.
 **/
ATT_MSIZE(1)
ATT_MALLOC
void *
xmalloc(size_t n)
{
	void *ptr = NULL;	/* New pointer to memory location */

	ptr = (void *) malloc(n);
	if (ptr) {
		memset(ptr, 0, n);
		return ptr;
	} else {
		errx(EX_SOFTWARE,
		     _("out of memory (unable to allocate %ld bytes)"), n);
	}
	/* should never get here */
	return NULL;
}

/**
 * \}
 **/
