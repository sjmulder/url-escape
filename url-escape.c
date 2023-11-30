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

static void
escape(const char *s, size_t len, FILE *f)
{
	size_t i;

	for (i=0; i<len; i++) {
		if (isalnum(s[i]) || strchr("_.-~", s[i]))
			fputc(s[i], f);
		else
			printf("%%%02X", s[i]);
	}
}

static void
unescape(const char *s, size_t len, FILE *f)
{
	size_t i;
	int c;

	for (i=0; i<len; i++) {
		if (i+2<len && s[i]=='%' && (c=fromhex(s+i+1, 2))!=-1) {
			fputc(c, f);
			i += 2;
		} else
			fputc(s[i], stdout);
	}
}

int
main(int argc, char **argv)
{
	static char buf[4*1024*1024];	/* 4 MB */
	int opt_decode=0, opt_nolf=0, c;
	size_t nr;

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

	nr = fread(buf, 1, sizeof(buf), stdin);
	if (ferror(stdin))
		err(1, "<stdin>");
	if (!feof(stdin))
		errx(1, "data too large");

	if (nr && buf[nr-1] == '\n')
		nr--; /* ignore trailing newline */

	if (opt_decode)
		unescape(buf, nr, stdout);
	else
		escape(buf, nr, stdout);

	if (!opt_nolf)
		fputc('\n', stdout);

	return 0;
}
