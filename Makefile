DESTDIR?=
PREFIX?=	/usr/local
MANPREFIX?=	${PREFIX}/man

CFLAGS.pkg!=	pkg-config --cflags libcurl
LDLIBS.pkg!=	pkg-config --libs libcurl

CFLAGS+=	${CFLAGS.pkg} -Wall -Wextra
LDLIBS+=	${LDLIBS.pkg}

all: url-escape

clean:
	rm -f url-escape *.o

check: url-escape
	url-escape -n <Makefile \
	 | url-escape -dn \
	 | diff -u Makefile -

install: url-escape
	install -d ${DESTDIR}${PREFIX}/bin \
		   ${DESTDIR}${MANPREFIX}/man1
	install -m755 url-escape   ${DESTDIR}${PREFIX}/bin/
	install -m644 url-escape.1 ${DESTDIR}${MANPREFIX}/man1/

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/url-escape \
	      ${DESTDIR}${MANPREFIX}/man1/url-escape.1

.PHONY: all clean check install uninstall
