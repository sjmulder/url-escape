#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sysexits.h>
#include <err.h>

const char usage[] =
"usage: .. | url-escape [-d] [-n]\n";

const char map[][4] = {
	"%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
	"%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
	"%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
	"%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
	"%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
	"%28", "%29", "%2A", "%2B", "%2C", "-",   ".",   "%2F",
	"%30", "%31", "%32", "%33", "%34", "%35", "%36", "%37",
	"%38", "%39", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
	"%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
	"H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
	"P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
	"X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
	"%61", "a",   "b",   "c",   "d",   "e",   "f",   "g",
	"h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
	"p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
	"x",   "y",   "z",   "%7B", "%7C", "%7D", "~",   "%7F",
	"%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
	"%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
	"%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
	"%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
	"%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
	"%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
	"%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
	"%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
	"%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
	"%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
	"%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
	"%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
	"%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
	"%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
	"%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
	"%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

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
	const char *p;

	for (i=0; i < count; i++)
		for (p = map[(unsigned char)src[i]]; *p; p++)
			dst[len++] = *p;

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
