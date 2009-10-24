
/* format.h -- header file for format.c

   $Id: format.h,v 1.1.1.1 2009/10/24 17:48:09 reynolds Exp $

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


/* format signature for printing */
void format_sig(char *sig, char *buffer, int buf_size);

/* remove the FIFO and exit */
extern void unlink_fifo(int status);
