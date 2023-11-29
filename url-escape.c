#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sysexits.h>
#include <err.h>
#include <curl/curl.h>

const char usage[] =
"usage: .. | url-decode [-d] [-n]\n";

int
main(int argc, char **argv)
{
	static char buf[4*1024*1024];	/* 4 MB */
	int opt_decode=0, opt_nolf=0, c;
	size_t nr;
	CURL *curl;
	char *output;

	if (!(curl = curl_easy_init()))
		errx(1, "failed to initialize libcurl");

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
		output = curl_easy_unescape(curl, buf, nr, NULL);
	else
		output = curl_easy_escape(curl, buf, nr);

	if (!output)
		errx(1, "failed to (un)escape");

	fputs(output, stdout);

	if (!opt_nolf)
		fputc('\n', stdout);

	curl_free(output);
	curl_easy_cleanup(curl);

	return 0;
}
