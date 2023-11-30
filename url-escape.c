#include <stdio.h>
#include <string.h>
#include <ctype.h>
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

static size_t
escape(const char *src, size_t count, char *dst, size_t *lenp)
{
	size_t len=0, i;

	for (i=0; i < count; i++)
		if (isalnum(src[i]) || strchr("_.-~", src[i]))
			dst[len++] = src[i];
		else {
			dst[len++] = '%';
			dst[len++] = "0123456789ABCDEF"[src[i]/16];
			dst[len++] = "0123456789ABCDEF"[src[i]%16];
		}
	
	*lenp = len;
	return i;
}

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
	static char src[1*1024*1024];	/* 1 MB */
	static char dst[3*1024*1024];	/* 3 MB */
	int opt_decode=0, opt_nolf=0, c;
	size_t nsrc, ndst, np;

#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");
#endif

	while ((c = getopt(argc, argv, "dnh")) != -1)
		switch (c) {
		case 'd': opt_decode = 1; break;
		case 'n': opt_nolf = 1; break;
		case 'h': fputs(usage, stderr); return 0;
		default: return EX_USAGE;
		}

	if (isatty(STDIN_FILENO))
		fputs("reading from stdin, EOF (^D) to end\n", stderr);

	nsrc = fread(src, 1, sizeof(src), stdin);
	if (ferror(stdin))
		err(1, "<stdin>");
	if (!feof(stdin))
		errx(1, "data too large");

	if (nsrc && src[nsrc-1] == '\n')
		nsrc--; /* ignore trailing newline */

	if (opt_decode)
		np = unescape(src, nsrc, dst, &ndst);
	else
		np = escape(src, nsrc, dst, &ndst);

	fwrite(dst, ndst, 1, stdout);
	fwrite(&src[np], nsrc-np, 1, stdout);	/* unprocessed tail */

	if (!opt_nolf)
		fputc('\n', stdout);

	return 0;
}
