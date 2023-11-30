#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sysexits.h>
#include <err.h>

const char usage[] =
"usage: .. | url-decode [-d] [-n]\n";

static int
fromhex(const char *s, size_t len)
{
	size_t i;
	int acc=0;

	for (i=0; i<len; i++) {
		acc *= 16;

		if (s[i]>='0' && s[i]<='9') acc += s[i]-'0'; else
		if (s[i]>='a' && s[i]<='f') acc += s[i]-'a'+10; else
		if (s[i]>='A' && s[i]<='F') acc += s[i]-'A'+10; else
			return -1;
	}

	return acc;
}

/*
 * Escapes src, returns number of chars processed which will be 'count'.
 * *lenp is set to the number of chars written to dst, which must be at
 * least 3*count long.
 */
static size_t
escape(const char *src, size_t count, char *dst, size_t *lenp)
{
	size_t len=0, i;

	for (i=0; i < count; i++)
		if (src[i] == '_' || src[i] == '.' ||
		    src[i] == '-' || src[i] == '~' ||
		    (src[i] >= '0' && src[i] <= '9') ||
		    (src[i] >= 'A' && src[i] <= 'Z') ||
		    (src[i] >= 'a' && src[i] <= 'z'))
			dst[len++] = src[i];
		else {
			dst[len++] = '%';
			dst[len++] = "0123456789ABCDEF"[src[i]/16];
			dst[len++] = "0123456789ABCDEF"[src[i]%16];
		}

	*lenp = len;
	return i;
}

/*
 * Unescape src, returns number of chars processed which may not be 'count'
 * if the input ends in an incomplete escape sequence. *lenp is set to
 * the number of chars written to dst, which must be at least 'count' long.
 */
static size_t
unescape(const char *src, size_t count, char *dst, size_t *lenp)
{
	size_t len=0, i;
	int c;

	for (i=0; i < count; i++)
		if (src[i] != '%')
			dst[len++] = src[i];	/* unescaped char */
		else if (i+2 >= count)
			break;			/* incomplete sequence */
		else if ((c = fromhex(&src[i+1], 2)) == -1)
			dst[len++] = src[i];	/* invalid sequence */
		else {
			dst[len++] = c;		/* decoded sequence */
			i += 2;
		}

	*lenp = len;
	return i;
}

int
main(int argc, char **argv)
{
	static char src[4096], dst[3*4096];
	int opt_nolf=0, eof=0, c;
	size_t nsrc=0;	/* length of src */
	size_t ndst=0;	/* length of dst */
	size_t ntop;	/* how many chars we want to encode/decode */
	size_t np;	/* how many chars were actually encoded/decoded */
	ssize_t nr;	/* number of chars read */
	size_t (*func)(const char*, size_t, char*, size_t*) = escape;

#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");
#endif

	while ((c = getopt(argc, argv, "dnh")) != -1)
		switch (c) {
		case 'd': func = unescape; break;
		case 'n': opt_nolf = 1; break;
		case 'h': fputs(usage, stderr); return 0;
		default: return EX_USAGE;
		}

	if (isatty(STDIN_FILENO))
		fputs("reading from stdin, EOF (^D) to end\n", stderr);
	
	do {
		/* top up buffer (it may hold unprocessed chars) */
		nr = read(STDIN_FILENO, src+nsrc, sizeof(src)-nsrc);
		if (nr == 0) eof = 1;
		if (nr ==-1) err(1, "<stdin>");
	
		nsrc += (size_t)nr;
		ntop = nsrc;

		/* skip trailing \n; picked up next round if not EOF */
		if (nsrc && src[nsrc-1] == '\n')
			ntop--; /* ignore trailing newline */

		np = func(src, ntop, dst, &ndst);
		fwrite(dst, ndst, 1, stdout);

		/* move unprocessed chars to front of 'src' */
		memmove(src, src+np, nsrc-np);
		nsrc -= np;
	} while (!eof);

	/* write out remaining unprocessed chars */
	fwrite(src, ntop-np, 1, stdout);

	if (!opt_nolf)
		fputc('\n', stdout);

	return 0;
}
