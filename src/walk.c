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
 * \file walk.c
 * Routines to walk a file system.
 **/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ftw.h>
#include <err.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <poll.h>
#include <time.h>
#include <sysexits.h>
#include <string.h>
#include <search.h>
#include <unistd.h>
#include <libgen.h>
#include <sysexits.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#include "defs.h"
#include "extern.h"
#include "mem.h"
#include "walk.h"


/* Internal functions */
static void       action(const void *, VISIT, int);
static int        cmp(const void *, const void *);
static int        dir_size(const char *, const struct stat *, int, struct FTW *);
static uint64_t   max_openfds(void);
static char      *pabs(const char *);
static char      *pname(const char *, int);
static char      *ppath(const char *, uint32_t);
static int        summary(void);
static char      *tformat(uint64_t);

/* Tree root node */
void *root = NULL;

/**
 * Walk a file system
 *
 * \retval 0 If there were no errors.
 * \retval 1 If an error was encounted.
 **/
int32_t
walk()
{

	uint64_t nopenfd = 0;           /**< Max open files **/
	char *adir = NULL;              /**< Absolute path **/

	if ((nopenfd = max_openfds()) <= 0) {
		return(EXIT_FAILURE);
	}

	if (nftw(options.path, dir_size, nopenfd, FTW_PHYS|FTW_MOUNT) != 0) {
		warnx(_("walking %s failed."), options.path);
		return(EXIT_FAILURE);
	}

	summary();

	return(EXIT_SUCCESS);
}

/**
 * Calculte the maximum number of open file discriptors.
 *
 * \retval -1 If there was an error.
 * \retval The maximum number of open file discriptors allowed.
 **/
static uint64_t
max_openfds(void)
{
	uint64_t i = 0;                 /**< Loop iterator **/
	uint64_t nopenfd = 0;           /**< Max open files **/
	struct rlimit rlp = {0};        /**< Resource limits **/
	struct pollfd *fds = NULL;      /**< File descriptor polls **/

	if (getrlimit(RLIMIT_NOFILE, &rlp) != 0) {
		warnx(_("unable to obtain the limit for open files"));
		return(-1);
	}

	/* find out how many files are currently open */
	if ((fds = xmalloc(rlp.rlim_cur * sizeof(struct pollfd))) == NULL) {
		warnx(_("unable to allocate polling file descriptors"));
		return(-1);
	}

	for (i = 0; i < rlp.rlim_cur; ++i) {
		fds[i].fd = i;
		fds[i].events = 0;
	}

	poll(fds, (nfds_t)rlp.rlim_cur, 0);

	for (i = 0; i < rlp.rlim_cur; ++i) {
		if (fds[i].revents == POLLNVAL) {
			++nopenfd;
		}
	}

	free(fds);
	fds = NULL;

	return(nopenfd);
}

/**
 * Calculate the directory size for old files.
 *
 * This is the call back function from nftw().
 *
 * \param[in] fpath   Name of the current entry.
 * \param[in] sb      Stat buffer of the current entry.
 * \param[in] tflag   File type flags.
 * \param[in] ftwbuf  FTW struct.
 *
 * \retval 0 If there were no errors.
 * \retval 1 If an error was encounted.
 **/
static int
dir_size(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
	struct pinfo *cur = NULL;
	struct pinfo **ptr = NULL;

	cur = xmalloc(sizeof(struct pinfo));

	cur->path = pname(fpath, tflag);
	cur->level = -1;

	ptr = tsearch(cur, &root, cmp);
	(*ptr)->total += sb->st_size;
	if ((*ptr)->level == -1) {
		(*ptr)->level = ftwbuf->level;
	}

	if (difftime(sb->st_atime, options.atime) < 0.0) {
		(*ptr)->greater += sb->st_size;
	}

	return(EXIT_SUCCESS);
}

/**
 * Absolute path
 *
 * Figure out the absolute path name when given a relative path.
 *
 * \param[in] rel The relative path name.
 *
 * \retval abs The absolute path name.
 **/
static char *
pabs(const char *rel)
{

	size_t bsize = 0;
	char *buf = NULL;
	struct stat sbuf = {0};

	/* make sure we were given a relative path */
	if (rel == NULL) {
		return(NULL);
	}

	/* make sure relative path actually exists */
	if (stat(rel, &sbuf) < 0) {
		errx(EX_IOERR, _("unable to stat %s"), rel);
	}

	bsize = pathconf(".", _PC_PATH_MAX);
	buf = (char *)xmalloc((bsize + 1) * sizeof(char));

	/* find the absolute path */
	if (realpath(rel, buf) == NULL) {
		errx(EX_IOERR, _("unable to resolve %s an error occurred at %s"),
		     rel, buf);
	}

	return(buf);
}

/**
 * Path name.
 *
 * Truncate a full path name down to the options
 * maxdepth path length.
 *
 * \param[in] path   The full path name
 * \param[in] tflag  The path type flag
 * \retval    str    The truncated path
 **/
static char *
pname(const char *path, int tflag)
{
	int i = 0;
	int j = 0;
	int n = 0;
	static int m = 0;
	char *tmp = NULL;
	char *dir = NULL;
	char *str = NULL;

	if (m == 0) {
		m = strlen(options.path);
	}

	tmp = strdup(path);
	if (tflag == FTW_F || tflag == FTW_SL) {
		dir = dirname(tmp);
	} else {
		dir = tmp;
	}

	n = strlen(dir);
	if (dir[n-1] == '/') {
		dir[n-1] = '\0';
	}

	for (i = m; i < n; ++i) {
		if (dir[i] == '/') {
			++j;
		}
		if (j > options.maxdepth) {
			break;
		}
	}


	str = xmalloc((i+1)*sizeof(char));
	strncpy(str, dir, i);

	if (tmp != NULL) {
		free(tmp);
		tmp = NULL;
	}

	return(str);
}

/**
 * Binary tree comparison routine.
 *
 * This uses strcmp on the struct pinfo path entry.
 *
 * \param[in] a  Tree node a.
 * \param[in] b  Tree node b.
 *
 * \retval   Integer greater than, equal to, or less than 0.
 **/
static int
cmp(const void *a, const void *b)
{
	struct pinfo *x = (struct pinfo *)a;
	struct pinfo *y = (struct pinfo *)b;

	return(strcmp(x->path, y->path));
}

/**
 * Walk the binary tree printing a summary.
 *
 * \retval 0 If there were no errors.
 * \retval 1 If an error was encounted.
 **/
static int32_t
summary(void)
{
	struct pinfo *cur = NULL;
	struct pinfo **ptr = NULL;

	if (options.cost > 0.0) {
		printf(ngettext("Cost [$]       >%d day[%%]     Directory\n",
				"Cost [$]       >%d days[%%]    Directory\n",
				options.atime_days),
		       options.atime_days);
	} else {
		printf(ngettext("Size [%s]      >%d day[%%]     Directory\n",
				"Size [%s]      >%d days[%%]    Directory\n",
				options.atime_days),
		       options.units, options.atime_days);
	}

	cur = xmalloc(sizeof(struct pinfo));
	cur->path = options.path;

	ptr = tfind(cur, &root, cmp);
	action((void *)ptr, postorder, 0);
	tdelete(cur, &root, cmp);

	twalk(root, action);

	return(EXIT_SUCCESS);
}

/**
 * Action to be taken for each tree element.
 *
 * \param[in] node  The current node.
 * \param[in] v     The traversal type.
 * \param[in] level The current node level.
 *
 * \retval 0 If there were no errors.
 * \retval 1 If an error was encounted.
 */
static void
action(const void *node, VISIT v, int level)
{
	struct pinfo *n = *(struct pinfo * const *)node;
	int i = 0;
	float size = 0.0;
	float percentage = 0.0;
	static size_t scale = 0;
	char *path = NULL;

	if (scale == 0) {
		switch (options.units[0]) {
			case 'k':
				scale = kB;
				break;
			case 'M':
				scale = MB;
				break;
			case 'G':
				scale = GB;
				break;
			case 'T':
				scale = TB;
				break;
			case 'P':
				scale = PB;
				break;
			case 'E':
				scale = EB;
				break;
		}
	}

	if (v == postorder || v == leaf) {

		size = (float)n->greater / (float)scale;
		/* Cost overrides size */
		if (options.cost > 0.0) {
			size *=  options.cost * options.atime_days;
		}
		percentage = (float)(n->greater / (float)n->total) * 100.0;
		path = ppath(n->path, n->level);

		printf(_("%12.2f  %12.0f    %s\n"),
		       size, percentage, path);

	}

}

/**
 * Pretty print a path.
 *
 * \param[in] path     The path to print.
 * \param[in] level    The path level under the top-level.
 * \param[out] ppath   The pretty printed path.
 * \param[out] pad     The amount of extra padding added.
 *
 * \retval 0 If there were no errors.
 * \retval 1 If an error was encounted.
 **/
static char *
ppath(const char *path, uint32_t level)
{
	uint32_t n = 0;
	size_t bsize = 0;
	const char indent[] = u8"│  ";
	const char tofile[] = u8"├──";
	const char last[]   = u8"└──";
	char *ptr = NULL;
	static char *tmp = NULL;


	bsize = pathconf(".", _PC_PATH_MAX);
	if (tmp == NULL) {
		tmp = xmalloc(bsize);
	} else {
		memset(tmp, 0, bsize);
	}

	if (level == 0) {
		strcpy(tmp, path);
		return(tmp);
	}

	for (n = 1; n < level; ++n) {
		strcat(tmp, indent);
	}

	strcat(tmp, tofile);

	ptr = strdup(path);
	strcat(tmp, basename(ptr));

	if (ptr != NULL) {
		free(ptr);
		ptr = NULL;
	}

	return(tmp);
}
