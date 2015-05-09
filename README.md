# xemacs-libcurl

XEmacs Binding for libcurl

This XEmacs module is under the same MIT/BSD style license as cURL
itself.

So far, it provides a very limited binding for libcurl, only enough
for a simple "hello, world" style demonstration.

There are potentially a number of bugs and limitations in this
demo, you have been warned.

It is written for XEmacs 21.4.  To make it work 21.5, it needs a
little porting.  As far as I know, the *_EXTERNAL_* macros have
been renamed, so that's a place to start.

So far, it has only been tested on OS X 10.10.3.  It may need some
adjustments for other systems.

The doc strings below have been adjusted from the libcurl
documentation; copyright Daniel Stenberg.  They may still need some
adjustment to be completely applicabel to elisp.

