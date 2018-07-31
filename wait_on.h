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

#ifndef WAIT_ON_H
#define WAIT_ON_H

#include <sys/types.h>
#include <sys/event.h>

/* make us a teeny bit more portable */
#ifdef __GCC__
	#define ATTRIBUTE_NORETURN	__attribute__((__noreturn__))
#else
	#define ATTRIBUTE_NORETURN
#endif

/* option flags */
#define E_FLAG	0x01	/* exec a program instead of exiting */
#define H_FLAG	0x02	/* want output in human readable form as well */
#define I_FLAG	0x04	/* want to know which file the event occured to */
#define W_FLAG	0x08	/* only interested in writes */

/* return values for !-i */
#define EXIT_BASE		0
#define EXIT_TIMEOUT	0
#define EXIT_DELETE		1
#define EXIT_WRITE		2
#define EXIT_EXTEND		3
#define EXIT_ATTRIB		4
#define EXIT_LINK		5
#define EXIT_RENAME		6
#define EXIT_REVOKE		7

void display_human(struct kevent *event);
int what_happened(struct kevent *event);
int which_happened(struct kevent *event, int file_count, char *const filenames[]);

#endif
