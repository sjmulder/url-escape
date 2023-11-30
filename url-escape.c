#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#ifdef _POSIX_VERSION
# include <unistd.h>
#endif

#define EX_USAGE	64

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
	int opt_decode=0, opt_nolf=0, i;
	size_t nr;

#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1) {
		fprintf(stderr, "url-escape: %s\n", strerror(errno));
		exit(1);
	}
#endif

	if (argc > 2) {
		fprintf(stderr, "url-escape: too many arguments "
		    "(try -h)\n");
		exit(EX_USAGE);
	}

	if (argv[1]) {
		if (argv[1][0] != '-') {
			fprintf(stderr, "url-escape: bad argument "
			    "(try -h)\n");
			exit(EX_USAGE);
		}

		for (i=1; argv[1][i]; i++)
			switch (argv[1][i]) {
			case 'd': opt_decode = 1; break;
			case 'n': opt_nolf = 1; break;
			case 'h': fputs(usage, stderr); return 0;
			default:
				fprintf(stderr, "url-escape: bad flag: -%c "
				    "(try -h)\n", argv[1][i]);
				exit(EX_USAGE);
			}
	}

#ifdef _POSIX_VERSION
	if (isatty(STDIN_FILENO))
		fputs("reading from stdin, EOF (^D) to end\n", stderr);
#endif

	nr = fread(buf, 1, sizeof(buf), stdin);
	if (ferror(stdin)) {
		fprintf(stderr, "url-escape: <stdin>: %s\n",
		    strerror(errno));
		exit(1);
	}

	if (!feof(stdin)) {
		fprintf(stderr, "url-escape: <stdin>: too large\n");
		exit(1);
	}

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
