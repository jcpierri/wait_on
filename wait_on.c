/*
###########################################################################
### wait_on : Enable shell scripts to monitor directories for new files ###
###########################################################################

Copyright 2002 Andrew Stevenson <andrew@ugh.net.au>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1.  Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

2.  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

3.  Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

###########################################################################
That said I'd appreciate a message if you use this software in anyway.
                                              Andrew <andrew@ugh.net.au>
###########################################################################
*/

#ifndef lint
static const char rcsid[] =
	"$Id: wait_on.c,v 1.4 2003/05/31 16:20:32 andrew Exp $";
#endif

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "wait_on.h"

static void usage(void)
	ATTRIBUTE_NORETURN;
static void version(void)
	ATTRIBUTE_NORETURN;

int main(int argc, char *const argv[]) {
	int c,	/* function returns + misc */
		ev,	/* value to exit with */
		i,	/* loop counter + misc */
		kq;	/* kernel queue descriptor */
	struct kevent *events;
	struct timespec *pts,	/* pointer to ts or NULL */
					ts;		/* timeout */
	u_int8_t options;

	/* make sure we remember our program name */
	setprogname(argv[0]);

	/* defaults */
	options = ts.tv_nsec = 0;
	pts = NULL;

	/* handle command line options */
	while ((c = getopt(argc, argv, "chit:vw")) != -1) {
		switch (c) {
			case 'c':
				/* copyright/version */
				version();
				break;
			case 'h':
				/* human output */
				options |= H_FLAG;
				break;
			case 'i':
				/* set exit value to indicate which file/directory changed */
				options |= I_FLAG;
				break;
			case 't':
				/* timeout */
				pts = &ts;
				ts.tv_sec = (time_t)strtol(optarg, (char **)NULL, 10);
				if (ts.tv_sec < 0 || (ts.tv_sec == LONG_MAX && errno == ERANGE)) {
					warnc(ERANGE, "invalid timeout");
					usage();
				}
				break;
			case 'v':
				/* copyright/version */
				version();
				break;
			case 'w':
				/* only interested in writes */
				options |= W_FLAG;
				break;
			case '?':
			default:
				/* unknown option */
				usage();
		}
	}
	argc -= optind;
	argv += optind;

	/* make sure there is at least one file/directory to watch */
	if (argc < 1) {
		usage();
	}

	/* see if there are soo many files/directories to watch that that our exit
	 * value may be ambiguous */
	if ((options & I_FLAG) && (argc >= EX__BASE)) {
		warnx("-i specified with >= %d files - exit code may be ambiguous", EX__BASE);
	}

	/* create the event array */
	if ((events = (struct kevent *)malloc(argc * sizeof(struct kevent))) == NULL) {
		err(EX_UNAVAILABLE, "malloc");
	}

	/* fill the event array */
	for (i = 0; i < argc; ++i) {
		/* optain a file descriptor for the file/directory */
		if ((c = open(argv[i], O_RDONLY | O_NONBLOCK)) == -1) {
			err(EX_NOINPUT, "can't open \"%s\" for reading", argv[i]);
		}
		events[i].ident = c;
		events[i].filter = EVFILT_VNODE;
		events[i].flags = EV_ADD | EV_ENABLE | EV_CLEAR;
		if (options & W_FLAG) {
			/* only interested in writes */
			events[i].fflags = NOTE_WRITE;
		} else {
			/* interested in everything */
			events[i].fflags = NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE;
		}
		/* store filename in udata for later retrieval */
		events[i].udata = (void *)argv[i];
	}
	
	/* obtain a kq */
	if ((kq = kqueue()) == -1) {
		err(EX_OSERR, "kqueue");
	}

	/* thats the preamble - now do the waiting
	 *
	 * we are only interested in one event as we can only signal one event
	 * so in the hope of efficiency we lie the second time about the size of
	 * the events array */
	switch (c = kevent(kq, events, argc, events, 1, pts)) {
		case -1:
			/* kevent failed */
			err(EX_UNAVAILABLE, "kevent");
			break;
		case 0:
			/* timeout */
			exit(EXIT_TIMEOUT);
			break;
		default:
			/* something for us to look at in events */
			for (i = 0; i < c; ++i) {
				if (events[i].flags & EV_ERROR) {
					/* error occured while processing a filter */
					warnc(events[i].data, "%s", (char *)events[i].udata);
					/* set ev in case this is all we get */
					ev = EX_UNAVAILABLE;
					continue;
				} else {
					/* something reportable must have happened */
					if (options & H_FLAG) {
						/* human readable output */
						display_human(&events[i]);
					}
					if (options & I_FLAG) {
						/* user wants to know which file */
						ev = which_happened(&events[i], argc, argv);
					} else {
						/* user wants to know what happened */
						ev = what_happened(&events[i]);
					}
					exit(ev);
				}
			}
			break;
	}

	/* NOT REACHED */
	return 0;
}

void display_human(struct kevent *event) {

	/* display event to stdout */
	printf("%s:", (char *)event->udata);
	if (event->fflags & NOTE_DELETE) {
		printf(" deleted");
	}
	if (event->fflags & NOTE_WRITE) {
		printf(" written");
	}
	if (event->fflags & NOTE_EXTEND) {
		printf(" extended");
	}
	if (event->fflags & NOTE_ATTRIB) {
		printf(" attributed");
	}
	if (event->fflags & NOTE_LINK) {
		printf(" linked");
	}
	if (event->fflags & NOTE_RENAME) {
		printf(" renamed");
	}
	if (event->fflags & NOTE_REVOKE) {
		printf(" revoked");
	}
	printf("\n");
}

/* indicate what sort of event occured (returns an EXIT_ value) */
int what_happened(struct kevent *event) {
	
	if (event->fflags & NOTE_DELETE) {
		return(EXIT_DELETE);
	}
	if (event->fflags & NOTE_WRITE) {
		return(EXIT_WRITE);
	}
	if (event->fflags & NOTE_EXTEND) {
		return(EXIT_EXTEND);
	}
	if (event->fflags & NOTE_ATTRIB) {
		return(EXIT_ATTRIB);
	}
	if (event->fflags & NOTE_LINK) {
		return(EXIT_LINK);
	}
	if (event->fflags & NOTE_RENAME) {
		return(EXIT_RENAME);
	}
	if (event->fflags & NOTE_REVOKE) {
		return(EXIT_REVOKE);
	}

	/* unknown event */
	warnx("unknown event of type %u occured to %s", event->fflags, (char *)event->udata);
	return EX_OSERR;
}

/* indicate which file the event occured to (index starts at 1) */
int which_happened(struct kevent *event, int file_count, char *const filenames[]) {
	int i;
	
	for (i = 0; i < file_count; ++i) {
		if (strcmp((char *)event->udata, filenames[i]) == 0) {
			/* we have a match */
			return (i + 1);
		}
	}

	/* it was a file we didn't ask about...*/
	warnx("event on unknown file (%s)", (char *)event->udata);
	return EX_OSERR;
}

static void usage(void) {

	fprintf(stderr, "usage: %s [-chivw] [-t timeout] file1 [...]\n", getprogname());
	exit(EX_USAGE);
}

static void version(void) {

	printf("%s version 1.0 Copyright 2002 Andrew Stevenson <andrew@ugh.net.au>\n", getprogname());
	exit(EX_OK);
}
