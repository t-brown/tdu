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
 * \file main.c
 * Main entry point and generic functions for program.
 **/

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <getopt.h>
#include <locale.h>
#include <err.h>
#include <sysexits.h>
#include <assert.h>
#include <time.h>

#include "gettext.h"
#include "defs.h"
#include "extern.h"
#include "walk.h"

#define DEFAULT_ATIME    45
#define SECONDS_IN_DAY   60 * 60 * 24

/* Internal functions */
static void print_usage(void);
static void print_version(void);
static const char * program_name(void);
static int32_t parse_argv(int32_t , char **);

struct opts options = {0}; /**< Program options */

/**
 * The main entry point of the program.
 *
 * \param argc   Number of command line arguments.
 * \param argv  Reference to the pointer to the argument array list.
 *
 * \retval 0 If there were no errors.
 * \retval 1 If an error was encounted.
 **/
int32_t
main(int32_t argc, char **argv)
{

	uint32_t verbose = 0;   /**< Program verbosity **/

	/* initialise gettext */
#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif
#ifdef ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	/* set defaults */
	options.maxdepth = 2;
	if ((options.atime = time(NULL)) == (time_t)-1) {
		errx(EX_SOFTWARE, "unable to obtain the current time");
	}
	options.atime -= DEFAULT_ATIME * SECONDS_IN_DAY;
	options.atime_days = DEFAULT_ATIME;
	strcpy(options.units, "GB");

	/* parse command line arguments */
	if (parse_argv(argc, argv)) {
		exit(EXIT_FAILURE);
	}

	/* Walk the directory tree */
	if (walk()) {
		return(EXIT_FAILURE);
	}

	return(EXIT_SUCCESS);
}

/**
 * Parse the command line arguments.
 *
 * \param[in]  argc     Number of command line arguments.
 * \param[in]  argv     Reference to the pointer to the argument array list.
 *
 * \retval 0 If there were no errors.
 **/
static int32_t
parse_argv(int32_t argc, char **argv)
{
	int32_t i = 0;
	int32_t opt = 0;
	int32_t opt_index = 0;
	uint32_t atime = 0;
	char *soptions = "hVva:m:u:";		/* short options structure */
	static struct option loptions[] = {	/* long options structure */
		{"help",     no_argument,       NULL, 'h'},
		{"version",  no_argument,       NULL, 'V'},
		{"verbose",  no_argument,       NULL, 'v'},
		{"atime",    required_argument, NULL, 'a'},
		{"maxdepth", required_argument, NULL, 'm'},
		{"units",    required_argument, NULL, 'u'},
		{NULL,       0,                 NULL,  0}
	};
	time_t now = {0};

	while ((opt = getopt_long(argc, argv, soptions, loptions,
			    &opt_index)) != -1) {
		switch (opt) {
			case 'V':
				print_version();
				break;
			case 'h':
				print_usage();
				break;
			case 'v':
				options.verbose = 1;
				break;
			case 'a':
				atime = (uint32_t)strtoul(optarg, NULL, 10);
				break;
			case 'm':
				options.maxdepth = (uint32_t)strtoul(optarg, NULL, 10);
				break;
			case 'u':
				if (optarg[0] == 'k' ||
				    optarg[0] == 'K') {
					strcpy(options.units, "kB");
				} else if (optarg[0] == 'm' ||
					   optarg[0] == 'M') {
					strcpy(options.units, "MB");
				} else if (optarg[0] == 'g' ||
					   optarg[0] == 'G') {
					strcpy(options.units, "GB");
				} else if (optarg[0] == 't' ||
					   optarg[0] == 'T') {
					strcpy(options.units, "TB");
				} else if (optarg[0] == 'p' ||
					   optarg[0] == 'P') {
					strcpy(options.units, "PB");
				} else if (optarg[0] == 'e' ||
					   optarg[0] == 'E') {
					strcpy(options.units, "EB");
				} else {
					warnx(_("unknown units: %s"), optarg);
					print_usage();
				}
				break;
			default:
				print_usage();
				break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		warnx(_("error: must specify a destination"));
		print_usage();
	} else {
		options.path = argv[0];
	}
	assert(options.path != NULL);
	assert(options.maxdepth > 0);

	/* Remove a trailing / from the path */
	i = strlen(options.path);
	if (options.path[i-1] == '/') {
		options.path[i-1] = '\0';
	}

	if (atime != 0) {
		/* Convert a number of days ago into a time_t */
		if ((now = time(NULL)) == (time_t)-1) {
			errx(EX_SOFTWARE, "unable to obtain the current time");
		}
		options.atime = now - (atime * SECONDS_IN_DAY);
		options.atime_days = atime;
		assert(options.atime > 0);
	}


	return(EXIT_SUCCESS);
}


/**
 * Prints a short program usage statement, explaining the
 * command line arguments and flags expected.
 **/
static void
print_usage(void)
{
	printf(_(\
"usage: %s [-h] [-V] [-v] [-a] [-m] [-u k|M|G|T|P|E] directory\n\
  -h, --help       display this help and exit.\n\
  -V, --version    display version information and exit.\n\
  -v, --verbose    verbose mode.\n\
  -a, --atime      last access time in days.\n\
  -m, --maxdepth   maximum depth to report on.\n\
  -u, --units      the units to report in.\n\
  directory        the directory to report on.\n\
"), program_name());
	exit(EXIT_FAILURE);
}

/**
 * Prints the program version number and compile date.
 **/
static void
print_version(void)
{
	printf(_("%s: %s %s\n"), program_name(), PACKAGE, VERSION);
	printf(_("Compiled on %s at %s.\n\n"), __DATE__, __TIME__);

	exit(EXIT_SUCCESS);
}

/**
 * Obtain the program name.
 **/
static const char *
program_name(void)
{
#if HAVE_GETPROGNAME
	return getprogname();
#else
#if HAVE_PROGRAM_INVOCATION_SHORT_NAME
	return program_invocation_short_name;
#else
	return "unknown";
#endif /* HAVE_PROGRAM_INVOCATION_SHORT_NAME */
#endif /* HAVE_GETPROGNAME */
}
