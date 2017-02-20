#!/usr/local/bin/ici

cgi.start_reply_header();
cgi.content_type("text/html");
cgi.end_reply_header();

html.start_page(argv[0]);
html.h1(argv[0]);
html.end_page();
