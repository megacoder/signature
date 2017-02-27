#define _XOPEN_SOURCE

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/utsname.h>

#include "format.h"
#include "signature.h"

/*
 *------------------------------------------------------------------------
 * format_sig: apply replacement text to multi-line signature
 *------------------------------------------------------------------------
 */

void
format_sig(
	char *		sig,		/* Holds signature (in, out)	 */
	char *		quote,		/* Holds replacement text (in)	 */
	int		quote_len	/* Length of replacement text	 */
)
{
	char * const	eoq = quote + quote_len;
	struct utsname	my_uname;
	char		tmp_sig[BUF_SIZE + 1];
	char *		sp;
	char *		qp;
	char *		tmp_qp;
	int		quoteCopied;
	int		word_len;
	int		c;
	int		n;
	char *		start;

	qp = quote;
	quoteCopied = 0;
	for( sp = sig; (c = *sp & 0xFF) != '\0'; )	{
		if( c != '<' )	{
			quoteCopied = 0;
			++sp;
			continue;
		}
		/* Placement field begins, find out how long it is	 */
		for( tmp_qp = sp; *tmp_qp == '<'; ++tmp_qp )	{
			/*NOTHING*/
		}
		n = tmp_qp - sp;
		/* Skip quote spaces					 */
		while( (qp < eoq) && isspace( *qp ) )	{
			++qp;
		}
		/* Find end of word					 */
		start = qp;
		while( (qp < eoq) && !isspace( *qp ) )	{
			++qp;
		}
		word_len = qp - start;
		/* Place word if there is room				 */
		if( word_len <= n )	{
			memcpy( sp, start, word_len );
			qp += word_len;
			sp += word_len;
			n  -= word_len;
			if( n )	{
				*sp++ = ' ';
				--n;
			}
		} else	{
			memset( sp, ' ', n );
			sp += n;
		}
	}
	/* Get OS information						 */
	if( uname( &my_uname ) == -1 )	{
		perror("uname() failed" );
		unlink_fifo();
		exit( EXIT_FAILURE );
	}
	/* Make a copy of the sig for use as a format string		 */
	strcpy( tmp_sig, sig );
	/* Add OS information to signature				 */
	n = sprintf( sig, tmp_sig, my_uname.release, my_uname.machine );
}
