
//	     Copyright 2015 Johann 'Myrkraverk' Oskarsson
//		        <johann@myrkraverk.com>

// This XEmacs module is under the same MIT/BSD style license as cURL
// itself; the exact text is at the bottom of this file as well as the
// LICENSE file.

// So far, it provides a very limited binding for libcurl, only enough
// for a simple "hello, world" style demonstration.

// There are potentially a number of bugs and limitations in this
// demo, you have been warned.

// It is written for XEmacs 21.4.  To make it work 21.5, it needs a
// little porting.  As far as I know, the *_EXTERNAL_* macros have
// been renamed, so that's a place to start.

// So far, it has only been tested on OS X 10.10.3.  It may need some
// adjustments for other systems.

// The doc strings below have been adjusted from the libcurl
// documentation; copyright Daniel Stenberg.  They may still need some
// adjustment to be completely applicabel to elisp.

#include <emodules.h>

#include <curl/curl.h>

static void __attribute__ ((constructor)) _curl_init( void ) {
  curl_global_init( CURL_GLOBAL_SSL );
}

// And just in case it ever gets unloaded, somehow.
static void __attribute__ ((destructor)) _curl_fini( void ) {
  curl_global_cleanup();
}

// Prototypes.
static size_t
writefunction( void *buffer, size_t size, size_t nmemb, void *userp );

// Begin CURL type implementation.

struct Lisp_CURL
{
  struct lcrecord_header header;
  CURL *curl;
};
typedef struct Lisp_CURL Lisp_CURL;

DECLARE_EXTERNAL_LRECORD( curl, Lisp_CURL );

#define XCURL( x ) XRECORD( x, curl, Lisp_CURL )
#define XSETCURL( x, p ) XSETRECORD( x, p, curl )
#define CURLP( x ) RECORDP( x, curl )
#define CHECK_CURL( x ) CHECK_RECORD( x, curl )
#define CONCHECK_CURL( x ) CONCHECK_RECORD( x, curl )

static Lisp_Object
mark_curl( Lisp_Object obj )
{
  // So far, there are no sub-objects to mark.

  return Qnil;
}

static void
print_curl( Lisp_Object obj, Lisp_Object printcharfun, int escapeflag )
{
  Lisp_CURL *p = XCURL( obj );
  char buffer[ 200 ];
  snprintf( buffer, sizeof buffer, 
	    "#<CURL * 0x%lx>", (unsigned long) p->curl );
  write_c_string( buffer, printcharfun );
}

static void
nuke_curl( void *header, int for_disksave )
{
  // for_disksave should never be true, because this is an externally
  // developed module.  It is true only in the dumping XEmacs is
  // dumping itself.

  // FIXME: Add an assertion or something that tests for_disksave?

  Lisp_CURL *lisp_curl = (Lisp_CURL *) header;

  if ( lisp_curl->curl ) {
    // I read somewhere that this can potentially be called more than once,
    // hence the if and pointer cleanup.
    curl_easy_cleanup( lisp_curl->curl );
    lisp_curl->curl = 0 ;
  }
}

DEFINE_EXTERNAL_LRECORD_IMPLEMENTATION( "curl-curl", // name
					curl, // c name
					mark_curl, // marker
					print_curl, // printer
					nuke_curl, // nuker
					NULL, // equal
					NULL, // hash
					NULL, // desc
					Lisp_CURL ); // type

Lisp_Object Qcurlp; // This is needed for XEmacs 21.4; the macros in
		    // 21.5 take care of this declaration for us.  I think.

static Lisp_Object make_curl( CURL *curl )
{
  // The allocation below may also have been renamed in 21.5.
  Lisp_CURL *lisp_curl = alloc_lcrecord_type( Lisp_CURL,
					      &lrecord_curl );

  lisp_curl->curl = curl;
  
  Lisp_Object ret;
  XSETCURL( ret, lisp_curl );
  return ret;
  
}

// End CURL type.

DEFUN("curl-version", Fcurl_version, 0, 0, "", /*
Returns a human readable string with the version number of libcurl and
some of its important components (like OpenSSL version).
*/
       ())
{
  return make_string_nocopy( (Bufbyte *) curl_version(),
			     strlen( curl_version() ) );
}

DEFUN("curl-easy-init", Fcurl_easy_init, 0, 0, 0, /*
This function must be the first function to call, and it returns a
CURL easy handle that you must use as input to other functions in the
easy interface.

Unlike the C interface, curl_global_init() is called automatically
when the module is loaded and curl_easy_cleanup() when the CURL handle
is garbage collected.
*/
      ())
{
  CURL *curl = curl_easy_init();

  // If we allow signals, it'll interfere with XEmacs and segfault on
  // OS X at least.
  CURLcode code = curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );
  if ( code == CURLE_OK )
    return make_curl( curl );
  else
    error( "Curl error: %s", curl_easy_strerror( code ) );
}

typedef struct  {
  Lisp_Object function;
  Lisp_Object argument;
} lisp_curl_userp;

static lisp_curl_userp writefunction_userp; // Initialized in syms_of_curl().

DEFUN("curl-easy-setopt", Fcurl_easy_setopt, 3, 3, 0, /*
Used to tell libcurl how to behave. By setting the appropriate
options, the application can change libcurl's behavior. All options
are set with an option followed by a parameter.  Read this manual
carefully as bad input values may cause libcurl to behave badly! You
can only set one option in each function call. A typical application
uses many `curl-easy-setopt' calls in the setup phase.

Options set with this function call are valid for all forthcoming
transfers performed using this handle. The options are not in any way
reset between transfers, so if you want subsequent transfers with
different options, you must change them between the transfers. You can
optionally reset all options back to internal default with
curl_easy_reset.

Strings passed to libcurl are copied by the library; thus the string
storage associated to the pointer argument may be overwritten after
`curl-easy-setopt' returns. The only exception to this rule is really
`curlopt-postfields', but the alternative that copies the string
`curlopt-copypostfields' has some usage characteristics you need to
read up on.

The handle is the return code from a `curl-easy-init' or
`curl-easy-duphandle' call.
*/
      (handle, option, value))
{
  CHECK_CURL( handle );
  CHECK_SYMBOL( option );

  Lisp_CURL *lisp_curl = XCURL( handle );

  if ( option == intern( "curlopt-url" ) ) {
    CHECK_STRING( value );
    
    CURLcode code =  curl_easy_setopt( lisp_curl->curl,
				       CURLOPT_URL,
				       XSTRING_DATA( value ) );

    if ( code == CURLE_OK )
      return Qt;
    else
      error( "Curl error: %s", curl_easy_strerror( code ) );
  } else if ( option == intern( "curlopt-writefunction" ) ) {
    CHECK_FUNCTION( value );

    writefunction_userp.function = value;

    CURLcode code = curl_easy_setopt( lisp_curl->curl,
				      CURLOPT_WRITEFUNCTION,
				      writefunction );
    if ( code != CURLE_OK )
      error( "Curl error: %s", curl_easy_strerror( code ) );

    code = curl_easy_setopt( lisp_curl->curl,
			     CURLOPT_WRITEDATA,
			     &writefunction_userp );
    if ( code == CURLE_OK )
      return Qt;
    else
      error( "Curl error: %s", curl_easy_strerror( code ) );

    

  }

  return Qnil;
}

static
size_t writefunction( void *buffer, size_t size, size_t nmemb, void *userp )
{
  // This function can garbage collect, through the visitor.
  printf( "Inside the callback.\n" );
  //  message( "Inside the callback function" );

  lisp_curl_userp *arg = (lisp_curl_userp *) userp;

  Lisp_Object string = make_string( buffer, size * nmemb );

  printf( "After make_string.\n" );

  struct gcpro gcpro1;

  GCPRO1( string );

  printf( "before call2.\n" );
  Lisp_Object val = call2( arg->function, string, arg->argument );

  UNGCPRO;

  if ( INTP( val ) )
    return XINT( val );
  
  return val;
}

DEFUN("curl-easy-perform", Fcurl_easy_perform, 1, 1, 0, /*
Invoke this function after `curl-easy-init' and all the
`curl-easy-setopt' calls are made, and will perform the transfer as
described in the options. It must be called with the same easy_handle
as input as the curl_easy_init call returned.

`curl-easy-perform' performs the entire request in a blocking manner
and returns when done, or if it failed. For non-blocking behavior, see
`curl-multi-perform'.

You can do any amount of calls to `curl-easy-perform' while using the
same easy-handle. If you intend to transfer more than one file, you
are even encouraged to do so. libcurl will then attempt to re-use the
same connection for the following transfers, thus making the
operations faster, less CPU intense and using less network
resources. Just note that you will have to use `curl-easy-setopt'
between the invokes to set options for the following
`curl-easy-perform'.

You must never call this function simultaneously from two places using
the same easy-handle. Let the function return first before invoking it
another time. If you want parallel transfers, you must use several
curl easy-handles.

While the easy-handle is added to a multi handle, it cannot be used by
`curl-easy-perform'.
*/
      (handle))
{
  CHECK_CURL( handle );

  Lisp_CURL *lisp_curl = XCURL( handle );

  CURLcode code = curl_easy_perform( lisp_curl->curl );

  if ( code == CURLE_OK )
    return Qt;
  else
    error( "Curl error: %s", curl_easy_strerror( code ) );

  return Qnil;
}

void modules_of_curl()
{
}

void syms_of_curl()
{
  INIT_EXTERNAL_LRECORD_IMPLEMENTATION( curl );

  defsymbol( &Qcurlp, "curl-curl-p" );

  writefunction_userp.function = Qnil;
  writefunction_userp.argument = Qnil;
  
  DEFSUBR( Fcurl_version );
  DEFSUBR( Fcurl_easy_init );
  DEFSUBR( Fcurl_easy_setopt );
  DEFSUBR( Fcurl_easy_perform );
}

void vars_of_curl()
{
}

// COPYRIGHT AND PERMISSION NOTICE

// Copyright (c) 1996 - 2015, Daniel Stenberg, daniel@haxx.se.
// Copyright 2015 Johann 'Myrkraverk' Oskarsson <johann@myrkraverk.com>

// All rights reserved.

// Permission to use, copy, modify, and distribute this software for
// any purpose with or without fee is hereby granted, provided that
// the above copyright notice and this permission notice appear in all
// copies.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// Except as contained in this notice, the name of a copyright holder
// shall not be used in advertising or otherwise to promote the sale,
// use or other dealings in this Software without prior written
// authorization of the copyright holder.
