# $Id: Makefile,v 1.1.1.1 2002/01/29 16:35:36 andrew Exp $

PROG=	wait_on
SRCS=	${PROG}.c ${PROG}.h
LDADD=
MAN8=	${PROG}.1

PREFIX?=	/usr/local
BINDIR=	${PREFIX}/bin
MANDIR=	${PREFIX}/man/man

.if defined(DEBUG)
CFLAGS=	-W -Wall -Wbad-function-cast -Wcast-align -Wcast-qual \
	-Wchar-subscripts -Wmissing-prototypes -Wnested-externs \
	-Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-Wwrite-strings -g
.endif

CFLAGS+= -I${PREFIX}/include -L${PREFIX}/lib

.include <bsd.prog.mk>
