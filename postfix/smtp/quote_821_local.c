/*++
/* NAME
/*	quote_821_local 3
/* SUMMARY
/*	quote local part of address
/* SYNOPSIS
/*	#include "quote_821_local.h"
/*
/*	VSTRING	*quote_821_local(dst, src)
/*	VSTRING	*dst;
/*	char	*src;
/* DESCRIPTION
/*	quote_821_local() quotes the local part of a mailbox address and
/*	returns a result that can be used in SMTP commands as specified
/*	by RFC 821.
/*
/*	Arguments:
/* .IP dst
/*	The result.
/* .IP src
/*	The input address.
/* STANDARDS
/*	RFC 821 (SMTP protocol)
/* BUGS
/*	The code assumes that the domain is RFC 821 clean.
/* LICENSE
/* .ad
/* .fi
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

/* System library. */

#include <sys_defs.h>
#include <string.h>
#include <ctype.h>

/* Utility library. */

#include <vstring.h>

/* Global library. */

#include "quote_821_local.h"

/* Application-specific. */

#define YES	1
#define	NO	0

/* is_821_dot_string - is this local-part an rfc 821 dot-string? */

static int is_821_dot_string(char *local_part, char *end)
{
    char   *cp;
    int     ch;

    /*
     * Detect any deviations from the definition of dot-string. We could use
     * lookup tables to speed up some of the work, but hey, how large can a
     * local-part be anyway?
     */
    if (local_part[0] == 0 || local_part[0] == '.')
	return (NO);
    for (cp = local_part; cp < end && (ch = *cp) != 0; cp++) {
	if (ch == '.' && cp[1] == '.')
	    return (NO);
	if (ch > 127)
	    return (NO);
	if (ch == ' ')
	    return (NO);
	if (ISCNTRL(ch))
	    return (NO);
	if (ch == '<' || ch == '>'
	    || ch == '(' || ch == ')'
	    || ch == '[' || ch == ']'
	    || ch == '\\' || ch == ','
	    || ch == ';' || ch == ':'
	    || ch == '@' || ch == '"')
	    return (NO);
    }
    if (cp[-1] == '.')
	return (NO);
    return (YES);
}

/* make_821_quoted_string - make quoted-string from local-part */

static VSTRING *make_821_quoted_string(VSTRING *dst, char *local_part, char *end)
{
    char   *cp;
    int     ch;

    /*
     * Put quotes around the result, and prepend a backslash to characters
     * that need quoting when they occur in a quoted-string.
     */
    VSTRING_RESET(dst);
    VSTRING_ADDCH(dst, '"');
    for (cp = local_part; cp < end && (ch = *cp) != 0; cp++) {
	if (ch > 127 || ch == '\r' || ch == '\n' || ch == '"' || ch == '\\')
	    VSTRING_ADDCH(dst, '\\');
	VSTRING_ADDCH(dst, ch);
    }
    VSTRING_ADDCH(dst, '"');
    VSTRING_TERMINATE(dst);
    return (dst);
}

/* quote_821_local - quote local part of address according to rfc 821 */

VSTRING *quote_821_local(VSTRING *dst, char *addr)
{
    char   *at;

    /*
     * According to RFC 821, a local-part is a dot-string or a quoted-string.
     * We first see if the local-part is a dot-string. If it is not, we turn
     * it into a quoted-string. Anything else would be too painful.
     */
    if ((at = strrchr(addr, '@')) == 0)		/* just in case */
	at = addr + strlen(addr);		/* should not happen */
    if (is_821_dot_string(addr, at)) {
	return (vstring_strcpy(dst, addr));
    } else {
	make_821_quoted_string(dst, addr, at);
	return (vstring_strcat(dst, at));
    }
}

#ifdef TEST

 /*
  * Test program for local-part quoting as per rfc 821
  */
#include <stdlib.h>
#include <vstream.h>
#include <vstring_vstream.h>
#include "quote_821_local.h"

main(void)
{
    VSTRING *src = vstring_alloc(100);
    VSTRING *dst = vstring_alloc(100);

    while (vstring_fgets_nonl(src, VSTREAM_IN)) {
	vstream_fprintf(VSTREAM_OUT, "%s\n",
			vstring_str(quote_821_local(dst, vstring_str(src))));
	vstream_fflush(VSTREAM_OUT);
    }
    exit(0);
}

#endif