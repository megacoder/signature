/* format.c -- formatting routine for signature

   $Id: format.c,v 1.1.1.1 2009/10/24 17:48:09 reynolds Exp $

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
#include <sys/utsname.h>

#include "format.h"

/* format signature for printing */
void format_sig(char *sig, char *buffer, int buf_size)
{
    struct utsname my_uname;
    char tmp_sig[BUF_SIZE];
    char *sig_ptr, *buf_ptr, *tmp_buf_ptr;
    int space_left, word_len, lf_flag;

    /* bollocks, this part is much easier in Perl! */
    lf_flag = 0;
    buf_ptr = buffer;
    /* terminate signature */
    *(sig + strlen(sig) - 1) = '\0';
    for (sig_ptr = sig; sig_ptr < sig + strlen(sig); ++sig_ptr) {
	if (buf_ptr >= buffer + buf_size) {
	    /* all of fortune printed, so shunt trailing white space away */
	    if (*sig_ptr == '<') {
		strcpy(sig_ptr, sig_ptr + 1);
		--sig_ptr;
	    }
	    continue;
	} else if (*sig_ptr != '<') {
	    /* in set text area, so fast forward to placeholder */
	    lf_flag = 1;
	    continue;
	} else if (*buf_ptr == '\b' || *buf_ptr == '_') {
	    ++buf_ptr;
	    --sig_ptr;
	    continue;
	} else if (*buf_ptr == '\n' || *buf_ptr == '\t' || *buf_ptr == ' ') {
	    if (!lf_flag) {
		/* replace a tab, space or LF with a space */
		*sig_ptr = ' ';
	    } else {
		/* on first placeholder char of line, so backtrack and don't
		   print anything */
		--sig_ptr;
	    }
	    ++buf_ptr;
	    continue;
	}

	lf_flag = 0;
	/* look ahead and calculate length of next word of fortune */
	tmp_buf_ptr = buf_ptr;
	while (*tmp_buf_ptr != ' ' && *tmp_buf_ptr != '\t'
	       && *tmp_buf_ptr != '\n' && *tmp_buf_ptr != '\b'
	       && *tmp_buf_ptr != '_') {
	    ++tmp_buf_ptr;
	}
	word_len = tmp_buf_ptr - buf_ptr;

	/* look ahead and calculate how much space we have on the current
	   line for the next word of fortune */
	tmp_buf_ptr = sig_ptr;
	space_left = 0;
	while (*tmp_buf_ptr++ == '<') {
	    if (++space_left == word_len) {
		break;
	    }
	}

	/* if next word won't fit on current line, shunt trailing white space 
	   away */
	if (space_left < word_len) {
	    strcpy(sig_ptr - 1, sig_ptr + space_left);
	    continue;
	}

	/* copy word from fortune to signature */
	strncpy(sig_ptr, buf_ptr, word_len);
	sig_ptr += word_len - 1;
	buf_ptr += word_len;
    }

    /* get OS information */
    if (uname(&my_uname) == -1) {
	perror("uname() failed");
	unlink_fifo(EXIT_FAILURE);
    }

    /* make a copy of the sig for use as a format string */
    strcpy(tmp_sig, sig);
    /* add OS information to signature */
    sprintf(sig, tmp_sig, my_uname.release, my_uname.machine);
}
