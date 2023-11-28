url-escape
==========
URL-escape and URL-unescape data

... | **url-escape** [**-d**] [**-n**]

Reads from standard input, URL-escapes (or unescapes, with **-d**), and
writes to standard output. A trailing newline is added unless **-n**
is given. If the input ends with a newline, it is ignored.

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
No dependencies. Should work on Linux, BSD, macOS, etc.

    $ make
    # make install   [DESTDIR=] [PREFIX=/usr/local]
    # make uninstall [DESTDIR=] [PREFIX=/usr/local]

Authors
-------
Sijmen J. Mulder (<ik@sjmulder.nl>)
