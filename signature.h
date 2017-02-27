# ifndef _SIGNATURE_H
#  define _SIGNATURE_H

void	version( void );
void	help( int );
void	unlink_fifo( void );
void	caught_signal( int );
void	ignore_sigpipe( int );
void	init_fifo( void );
void	install_sig_handler( void );

extern	unsigned	debug;
# endif /* !_SIGNATURE_H */
