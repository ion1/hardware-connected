/* hardware-connected: Indicates whether given hardware exists in the system.
 * Copyright © 2007  Johan Kiviniemi
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sysexits.h>
#include <fnmatch.h>

#include "list.h"

#define ALIAS_MAX 128
#define ALIAS_FMT "%127s"

#define MODINFO_PATH "/sbin/modinfo"

static List *devices;

static void
usage (const char *name)
{
	fprintf (stderr, "USAGE: %s [-hv] "
		 "[-m MODNAME | MODALIAS_PATTERN ...]\n", name);
	exit (EX_USAGE);
}

static void
read_modalias (const char *path)
{
	FILE *fh;
	char  alias[ALIAS_MAX];

	fh = fopen (path, "r");
	if (! fh) {
		perror ("open");
		return;
	}

	if (fscanf (fh, ALIAS_FMT, alias) == 1) {
		list_add_before (devices, strdup (alias));
	} else {
		fprintf (stderr, "Couldn't read full line from %s\n", path);
	}

	fclose (fh);
}

static void
dir_handler (const char *path, List *dirs)
{
	DIR           *dir;
	struct dirent *ent;
	char           ent_path[PATH_MAX];

	dir = opendir (path);
	if (! dir) {
		perror ("opendir");
		return;
	}

	while ((ent = readdir (dir))) {
		if (*ent->d_name == '.')
			continue;

		snprintf ((char *)&ent_path, PATH_MAX,
		          "%s/%s", path, ent->d_name);

		if (ent->d_type == DT_DIR)
			list_add_before (dirs, strdup (ent_path));
		else if (strcmp (ent->d_name, "modalias") == 0)
			read_modalias (ent_path);
	}

	closedir (dir);
}

static void
walk_devices (const char *path)
{
	List          *dirs;

	dirs = list_new();
	assert (dirs != NULL);

	list_add_before (dirs, strdup (path));

	while (! list_empty (dirs)) {
		list_foreach_safe (dirs, i) {
			list_cut (i);
			dir_handler (i->entry, dirs);
			free (i->entry);
			list_free (i);
		}
	}

	list_free (dirs);
}

/**
 * modinfo_pipe: Run modinfo and replace the stdin of self with its output.
 */
static void
modinfo_pipe (const char *modname)
{
	int   pfd[2];
	pid_t cpid;

	if (pipe (pfd) == -1) {
		perror ("pipe");
		exit (EX_OSERR);
	}

	cpid = fork ();
	if (cpid == -1) {
		perror ("fork");
		exit (EX_OSERR);
	}

	if (cpid == 0) {
		/* I'm the child. */
		close (pfd[0]);  /* Close the read end. */

		if (close (STDOUT_FILENO) == -1) {
			perror ("child: close stdout");
			_exit (EX_OSERR);
		}
		if (dup (pfd[1]) == -1) {
			perror ("child: dup output");
			_exit (EX_OSERR);
		}

		if (execl (MODINFO_PATH, MODINFO_PATH, "-F", "alias", "--",
			   modname, NULL) < 0)
			perror ("execl");

		_exit (EX_OSERR);
	}

	/* I'm the parent. */
	close (pfd[1]);  /* Close the write end. */

	if (close (STDIN_FILENO) == -1) {
		perror ("parent: close stdin");
		exit (EX_OSERR);
	}
	if (dup (pfd[0]) == -1) {
		perror ("parent: dup input");
		exit (EX_OSERR);
	}
}

static void
pipe_cleanup (void)
{
	close (STDIN_FILENO);
	wait (NULL);  /* Wait for child. */
}

static int
match_pattern (const char *pattern, const int verbose)
{
	int ret = 0;
	list_foreach_safe (devices, i) {
		if (fnmatch (pattern, i->entry, 0) == 0) {
			ret = 1;
			if (verbose) {
				printf ("%s\n", (char *)i->entry);
				/* Remove the device from the list, so that it
				 * doesn't get printed again. */
				list_cut (i);
				free (i->entry);
				list_free (i);
			} else {
				break;
			}
		}
	}
	return ret;
}

int
main (int    argc,
      char **argv)
{
	int   ret = 1;
	int   verbose = 0;
	char *modname = NULL;
	int   opt;
	char  pattern[ALIAS_MAX];

	while ((opt = getopt (argc, argv, "hm:v")) != -1) {
		switch (opt) {
		case 'm':
			modname = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			usage (argv[0]);
		}
	}

	devices = list_new ();
	assert (devices != NULL);

	walk_devices ("/sys/devices");

	if (modname)
		modinfo_pipe (modname);

	if (! modname && optind < argc) {
		/* Match devices against patterns from argument list. */
		for (int arg = optind; arg < argc; arg++) {
			if (match_pattern (argv[arg], verbose)) {
				ret = 0;
				if (! verbose) break;
			}
		}
	} else {
		/* Match devices against patterns from standard input. */
		while (scanf (ALIAS_FMT, pattern) == 1) {
			if (match_pattern (pattern, verbose)) {
				ret = 0;
				if (! verbose) break;
			}
		}
	}

	if (modname)
		pipe_cleanup ();

	list_foreach (devices, i)
		free (i->entry);
	list_free (devices);

	return ret;
}
