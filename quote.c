#define _XOPEN_SOURCE

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include "quote.h"
#include "signature.h"

/* get a quote from an external quote file */

FILE *
index_quote_file(
	char *			quote_file,
	int *			total_quotes,
	struct sig_node * *	first_node
)
{
	struct sig_node *	node;
	struct sig_node *	prev_node;
	unsigned short int	sep_flag = 0;
	unsigned short int	lf_flag = 0;
	FILE *			quotes = NULL;

	/* initialise quote count */
	*total_quotes = 0;

	if (debug) {
		fprintf(stderr, "Opening quote file %s...\n", quote_file);
	}

	/* open quote file */
	if ((quotes = fopen(quote_file, "r")) == NULL) {
		perror(quote_file);
		unlink_fifo();
		exit(EXIT_FAILURE);
	}
	/* create a linked list of quote offsets & lengths */
	node = malloc(sizeof( struct sig_node ));
	node->offset = 0;
	*first_node = node;
	while(!feof( quotes ))	{
		switch(fgetc( quotes )) {
		default:
			sep_flag = 0;
			lf_flag = 0;
			break;
		case '~':
		case '%':
			if( lf_flag )	{
				/* Set flag if this ~ follows a LF	 */
				sep_flag = 1;
				lf_flag = 0;
			}
			break;
		case '\n':
			if( sep_flag )	{
				/* We have a LF, preceded by a ~, preceded by a LF */
				node->length = ftell( quotes ) - node->offset - 2;
				if( debug )	{
					fprintf(stderr,
					"Quote # %5d:\toset: %8d\tlen: %8d\n",
						*total_quotes,
						node->offset,
						node->length
					);
				}
				/* make this node the previous one */
				prev_node = node;
				/* create a new node */
				node = malloc(sizeof(struct sig_node));
				node->offset = ftell(quotes);
				/* point previous node to this one */
				prev_node->next_node = node;
				/* increment quote count */
				++(*total_quotes);
				sep_flag = 0;
			}
			lf_flag = 1;
			break;
		}
	}
	return (quotes);
}

/* print a quote from the quote file */

int
print_quote(
	FILE *			quotes,
	int			total_quotes,
	struct sig_node *	first_node
)
{
	static	int		first = 1;
	struct sig_node *	node;
	char *			quote_buf;
	int			rand_sig;
	ssize_t			n;

	node = first_node;
	if( first )	{
		/*
		 * Seed the random number generator
		 */
		srand( getpid() + time(NULL) + getppid() );
		first = 0;
	}
	/*
	 * get a random number in the range 0 - total_quotes -- got this
	 * example from the NOTES section of rand( 3 )
	 */
	rand_sig = (int) ((float) total_quotes * rand() / ( RAND_MAX + 1.0 ));
	/* Skip to the correct index					 */
	for( ; rand_sig > 0; --rand_sig )	{
		node = node->next_node;
	}
	/* Seek to start of quote in file				 */
	fseek( quotes, node->offset, SEEK_SET );
	/* Create and zero a buffer to hold the quote)			 */
	quote_buf = malloc( node->length + 1 );
	/* Read the quote into the buffer				 */
	n = read(fileno( quotes ), quote_buf, node->length );
	if( n > 0 )	{
		char *	bp;

		for( bp = quote_buf + n; bp > quote_buf; --bp )	{
			if( !isspace( bp[-1] ) && !iscntrl( bp[-1] ) )	{
				break;
			}
		}
		*bp = '\0';
	}
	/* Print quote							 */
	puts( quote_buf );
	/* Free buffer							 */
	free( quote_buf );
	return( EXIT_SUCCESS );
}
