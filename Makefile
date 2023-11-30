DESTDIR?=
PREFIX?=	/usr/local
MANPREFIX?=	${PREFIX}/man

CFLAGS+=	 -Wall -Wextra -O3 -g

all: url-escape

clean:
	rm -f url-escape *.o

check: url-escape
	./url-escape     <test.bin | diff -q test.txt -
	./url-escape -dn <test.txt | diff -q test.bin -

install: url-escape
	install -d ${DESTDIR}${PREFIX}/bin \
		   ${DESTDIR}${MANPREFIX}/man1
	install -m755 url-escape   ${DESTDIR}${PREFIX}/bin/
	install -m644 url-escape.1 ${DESTDIR}${MANPREFIX}/man1/

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/url-escape \
	      ${DESTDIR}${MANPREFIX}/man1/url-escape.1

.PHONY: all clean check install uninstall
