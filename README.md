url-escape
==========
URL-escape and URL-unescape data

... | **url-escape** [**-d**] [**-n**]

Reads from standard input, URL-escapes (or unescapes, with **-d**), and
writes to standard output. A trailing newline is added unless **-n**
is given. If the input ends with a newline, it is ignored.

Escaping and unescaping is performed with
[curl_easy_escape()](
  https://curl.se/libcurl/c/curl_easy_escape.html) and
[curl_easy_unescape()](
  https://curl.se/libcurl/c/curl_easy_unescape.html)
respectively.

Examples
--------
Escape a string:

    $ echo 'Hello, World!' | url-escape
    Hello%2C%20World%21

Escape a string and unescape it again:

    $ echo 'Hello, World!' | url-escape | url-escape -d
    Hello, World!

Installation
------------
Should work on Linux, BSD, macOS, etc. Requires [libcurl](
  https://curl.se/libcurl).

    $ make
    # make install   [DESTDIR=] [PREFIX=/usr/local]
    # make uninstall [DESTDIR=] [PREFIX=/usr/local]

Authors
-------
Sijmen J. Mulder (<ik@sjmulder.nl>)
