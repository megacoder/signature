/* quote.c -- quote file routines for signature

   $Id: quote.c,v 1.1.1.1 2009/10/24 17:48:09 reynolds Exp $

   Copyright (C) Ian Macdonald <ian@caliban.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


#define _XOPEN_SOURCE

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "quote.h"

extern unsigned short int debug;

/* get a quote from an external quote file */
FILE *index_quote_file(char *quote_file, int *total_quotes,
		       struct sig_node **first_node)
{
    struct sig_node *node, *prev_node;
    unsigned short int sep_flag = 0, lf_flag = 0;
    FILE *quotes = NULL;

    /* initialise quote count */
    *total_quotes = 0;

    if (debug) {
	fprintf(stderr, "Opening quote file %s...\n", quote_file);
    }

    /* open quote file */
    if ((quotes = fopen(quote_file, "r")) == NULL) {
	perror(quote_file);
	unlink_fifo(EXIT_FAILURE);
    }
    /* create a linked list of quote offsets & lengths */
    node = (struct sig_node *) malloc(sizeof(struct sig_node));
    node->offset = 0;
    *first_node = node;
    while (!feof(quotes)) {
	switch (fgetc(quotes)) {
	case '~':
	case '%':
	    if (lf_flag) {
		/* set flag if this ~ follows a LF */
		sep_flag = 1;
		lf_flag = 0;
	    }
	    break;
	case '\n':
	    if (sep_flag) {
		/* we have a LF, preceded by a ~, preceded by a LF */
		node->length = ftell(quotes) - node->offset - 2;
		if (debug) {
		    fprintf(stderr,
			    "Quote # %5d:\toffset: %8d\tlength: %8d\n",
			    *total_quotes, node->offset, node->length);
		}
		/* make this node the previous one */
		prev_node = node;
		/* create a new node */
		node = (struct sig_node *) malloc(sizeof(struct sig_node));
		node->offset = ftell(quotes);
		/* point previous node to this one */
		prev_node->next_node = node;
		/* increment quote count */
		++(*total_quotes);
		sep_flag = 0;
	    }
	    lf_flag = 1;
	    break;
	default:
	    sep_flag = 0;
	    lf_flag = 0;
	}
    }
    return (quotes);
}

/* print a quote from the quote file */
int
print_quote(FILE * quotes, int total_quotes, struct sig_node *first_node)
{
    struct sig_node *node;
    char *quote_buf;
    int rand_sig;

    node = first_node;
    /* seed the random number generator on this PID -- this should be random
       enough for our purposes */
    srand(getpid());
    /* get a random number in the range 0 - total_quotes -- got this example
       from the NOTES section of rand(3) */
    rand_sig = (int) ((float) total_quotes * rand() / (RAND_MAX + 1.0));
    /* skip to the correct index */
    for (; rand_sig > 0; --rand_sig) {
	node = node->next_node;
    }
    /* seek to start of quote in file */
    fseek(quotes, node->offset, SEEK_SET);
    /* create and zero a buffer to hold the quote) */
    quote_buf = (char *) malloc(node->length + 1);
    memset(quote_buf, 0, node->length + 1);
    /* read the quote into the buffer */
    read(fileno(quotes), quote_buf, node->length);
    /* print quote */
    puts(quote_buf);
    /* free buffer */
    free(quote_buf);
    return (EXIT_SUCCESS);
}
