#define _XOPEN_SOURCE
#define	_DEFAULT_SOURCE

#include <config.h>

#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "signature.h"
#include "quote.h"
#include "format.h"

/* name of default signature file */

#define DEFAULT_FIFO ".signature"

/* *INDENT-OFF* */

/* text of default signature */
#define SIGNATURE							      \
"Linux, the choice          | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"  \
"of a GNU generation   -o)  | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"  \
"Kernel %-9.9s       /\\  | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"    \
"on a %-6.6s           _\\_v | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n" \
"                           | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"

/* *INDENT-ON* */

static char		fifo[ PATH_MAX + 1];
static	char		quote_file[ PATH_MAX + 1 ];
unsigned		debug = 0;

/* display version information */

void
version(
	void
)
{
    printf( "%s %s (%s %s)\n", PACKAGE, VERSION, __DATE__, __TIME__ );
    exit(EXIT_SUCCESS);
}

/* display help message */

void
help(
	int		status
)
{
	static char const * const	msgs[] =	{
		"usage: signature [-d] [-f FIFO] [-h] [-n] [-p program |-q qfile] [-t template] [-v]",
		"-d              print debugging messages on stderr",
		"-f <FIFO>       FIFO instead of ~/.signature",
		"-h              display this help message",
		"-n              don't fork into background as daemon",
		"-p <program>    use this program instead of fortune(6)",
		"-q <qfile>      quote from this file instead of fortune(6)",
		"-t <template>   use signature template instead of default",
		"-v              display version information",
	};
	static size_t		Nmsgs = sizeof( msgs ) / sizeof( msgs[0] );
	char const * const * const lmp = msgs + Nmsgs;
	char const * const *	mp;

	for( mp = msgs; mp < lmp; ++mp )	{
		printf( "%s\n", *mp );
	}
	exit(status);
}

/* remove the FIFO */

void
unlink_fifo(
	void
)
{
	if( debug )	{
		fprintf( stderr, "Unlinking FIFO...\n" );
	}
	unlink( fifo );
}

/* when we catch a signal */

void
caught_signal(
	int		sig
)
{
	if( debug )	{
		fprintf( stderr, "Caught signal %d. Clearing up...\n", sig );
	}
	unlink_fifo();
	exit( EXIT_SUCCESS );
}

/* ignore SIGPIPE */

void
ignore_sigpipe(
	int		sig
)
{
	if (debug) {
		fprintf(stderr, "Caught a SIGPIPE. Using Pine?\n");
	}
}

/* set up FIFO */

void
init_fifo(
	void
)
{
	unlink( fifo );
	if (mkfifo(fifo, 0644) == -1) {
		fprintf(stderr, "Couldn't create %s as FIFO\n", fifo);
		exit(EXIT_FAILURE);
	}
}

/* install signal handler */

void
install_sig_handler(
	void
)
{
	struct sigaction	act;

	/* Install signal handler					 */
	act.sa_handler = caught_signal;
	sigemptyset( &act.sa_mask );
	act.sa_flags = 0;
	sigaction( SIGINT, &act, 0 );
	sigaction( SIGHUP, &act, 0 );
	sigaction( SIGTERM, &act, 0 );
	act.sa_handler = ignore_sigpipe;
	sigemptyset( &act.sa_mask );
	sigaction( SIGPIPE, &act, 0 );
}

int
main(
	int		argc,
	char *		argv[]
)
{
	char		signature[ BUF_SIZE + 1 ];
	char		sig_buf[ BUF_SIZE + 1 ];
	char		fortune[ BUF_SIZE + 1 ];
	char *		producer = NULL;
	char *		prodarg = NULL;
	char *		prodargs[MAX_PROG_ARGS+1];
	char *		slash_ptr;
	char *		cwd_ptr;
	int		prodargc = 0;
	unsigned short int no_fork = 0;
	int		exit_status = 0;
	int		sig_fd;
	int		sig_buf_len = 0;
	int		opt;
	int		pid;
	int		file_pipes[2];
	int		fifo_fd;
	int		num_read;
	int		total = 0;
	FILE *		quote_stream;
	struct sig_node * first_node;
	struct sig_node * * first_node_ptr = &first_node;

	memset( &quote_file, 0, BUF_SIZE );
	/* Get option switches						 */
	while( ( opt = getopt( argc, argv, "df:hnp:q:t:v" )) != -1 )	{
		switch( opt )	{
		case 'd':
			/* Set debugging flag				 */
			debug = 1;
			break;
		case 'f':
			/* Get location of FIFO				 */
			slash_ptr = strrchr( optarg, '/' );
			if( slash_ptr == NULL || *optarg == '.' )	{
				/* Make path absolute			 */
				cwd_ptr = getcwd( NULL, BUF_SIZE );
				sprintf( fifo, "%s/%s", cwd_ptr, optarg );
				free( cwd_ptr );
			} else	{
				strcpy( fifo, optarg );
			}
			if( debug )	{
				fprintf( stderr, "FIFO is %s\n", fifo );
			}
			break;
		case 'h':
			help( EXIT_SUCCESS );
			break;
		case 'n':
			no_fork = 1;
			break;
		case 'p':
			if( quote_file[0] != '\0' )	{
				help( EXIT_FAILURE );
			}
			producer = optarg;
			prodargs[prodargc++] = strtok( producer, " " );
			while( ( prodarg = strtok( NULL, " " )) != NULL )	{
				prodargs[prodargc++] = prodarg;
				if( prodargc > MAX_PROG_ARGS )	{
					fprintf( stderr, "More than %d arguments to %s. "
					"Excess arguments ignored...\n",
					MAX_PROG_ARGS, producer );
					break;
				}
			}
			prodargs[prodargc] = NULL;
			/*
			 * check for existence of program by forking and
			 * then trying to exec() it in the child
			 */
			pid = fork();
			switch( pid )	{
			default:
				/* We are the parent			 */
				waitpid( pid, &exit_status, 0 );
				if( exit_status != EXIT_SUCCESS )	{
					fprintf( stderr, "Program %s doesn't seem to exist\n",
					producer );
					exit( EXIT_FAILURE );
				}
			case -1:	/* Oh well			 */
				perror("Couldn't fork() a child process" );
				exit( EXIT_FAILURE );
			case 0:		/* We are the children		 */
				/* Close stdout				 */
				close( 1 );
				execlp(producer, producer, (char *) 0 );
				exit( EXIT_FAILURE );
			}
			break;
		case 'q':
			if( producer )	{
				help( EXIT_FAILURE );
			}
			slash_ptr = strrchr( optarg, '/' );
			if( slash_ptr == NULL || *optarg == '.' )	{
				/* Make path absolute			 */
				/* Get the current directory		 */
				cwd_ptr = getcwd( NULL, BUF_SIZE );
				/* Get absolute path of quote_file	 */
				sprintf( quote_file, "%s/%s", cwd_ptr, optarg );
				free( cwd_ptr );
			} else	{
				/* Get a quote from an external file instead of fortune */
				strcpy( quote_file, optarg );
			}
			break;
		case 't':
			if( debug )	{
				fprintf( stderr, "Opening template file %s...\n", optarg );
			}
			sig_fd = open( optarg, O_RDONLY );
			if( sig_fd == -1 )	{
				fprintf( stderr, "Couldn't open template %s for reading\n",
				optarg );
				exit( EXIT_FAILURE );
			}
			memset( &sig_buf, 0, BUF_SIZE );
			sig_buf_len = read( sig_fd, &sig_buf, BUF_SIZE );
			if( sig_buf_len == -1 ) {
				perror("Failed to read() template file" );
				exit( EXIT_FAILURE );
			}
			close( sig_fd );
			break;
		case 'v':
			version();
			break;
		case ':':
			fprintf( stderr, "Option needs a value\n" );
			break;
		case '?':
			exit( EXIT_FAILURE );
		}
	}
	if( fifo[0] == '\0' )	{
		/* Set up the path to default FIFO			 */
		sprintf( fifo, "%s/%s", getenv( "HOME" ), DEFAULT_FIFO );
	}
	/* Initialise FIFO						 */
	init_fifo();
	/* Install signal handler					 */
	install_sig_handler();

	if( !no_fork )	{
		if( daemon( 0, 0 ) )	{
			perror( "daemon()" );
			unlink_fifo();
			exit( EXIT_FAILURE );
		}
	}

	quote_stream = quote_file[0] ?	index_quote_file(
			quote_file,
			&total,
			first_node_ptr
	) : NULL;

	/* Start main loop						 */
	for( ; ; )	{
		memset( &signature, 0, BUF_SIZE );
		if( sig_buf_len > 0 )	{
			/* Use user-supplied template			 */
			strcpy( signature, sig_buf );
		} else	{
			/* Use stock signature				 */
			strcpy( signature, SIGNATURE );
		}

		if(access( fifo, F_OK ) != 0 )	{
			fprintf( stderr, "%s seems to have disappeared\n", fifo );
			exit( EXIT_FAILURE );
		}

		/* Open FIFO for writing				 */
		fifo_fd = open( fifo, O_WRONLY );
		if( fifo_fd == -1 )	{
			fprintf( stderr, "Couldn't open %s for writing\n", fifo );
			unlink_fifo();
			exit( EXIT_FAILURE );
		}
		/* Create a pipe					 */
		if(pipe( file_pipes ) != 0 )	{
			perror("Couldn't create pipe() for passing signature" );
			unlink_fifo();
			exit( EXIT_FAILURE );
		}
		/* Fork off a child					 */
		pid = fork();
		switch( pid )	{
		case -1:		/* Oh well			 */
			perror("Couldn't fork() a child process" );
			unlink_fifo();
			exit( EXIT_FAILURE );
		case 0:			/* In child			 */
			/* Close stdout					 */
			close( 1 );
			/* Duplicate write fd as stdout			 */
			dup( file_pipes[1] );
			/* Close write fd				 */
			close( file_pipes[1] );
			/* Close read fd				 */
			close( file_pipes[0] );

			/* Check if we need to print from a quote file	 */
			if( quote_file[0] != '\0' )	{
				exit(print_quote( quote_stream, total, first_node ));
			} else if( !producer )	{/* Call fortune	 */
				if( debug )	{
					fprintf( stderr, "Calling %s...\n", FORTUNE );
				}
				/* Send fortune cookie to write fh	 */
				execl( FORTUNE, "fortune", "-s"
# ifdef OFFENSIVE_FORTUNES
					"a"
# endif /* OFFENSIVE_FORTUNES */
				, (char *) 0 );
				/* We only reach this point if exec() didn't work */
				fprintf( stderr, "Couldn't exec %s\n", FORTUNE );
				exit( EXIT_FAILURE );
			} else	{	/* External program		 */
				if( debug )	{
					fprintf( stderr, "Calling %s...\n", producer );
				}
				/* Send producer's output to write fh	 */
				execvp( producer, prodargs );
				/* We only reach this point if exec() didn't work */
				fprintf( stderr, "Couldn't exec %s\n", producer );
				exit( EXIT_FAILURE );
			}
			exit( EXIT_FAILURE );
			/* NOTREACHED					 */
		default:		/* In parent			 */
			if( debug )	{
				fprintf( stderr, "Child process PID %d...\n", pid );
			}
			/* Clean up after child				 */
			waitpid(pid, (int *) 0, 0 );
			/* Close write fd				 */
			close( file_pipes[1] );
			/* Read fortune cookie through pipe		 */
			num_read = read( file_pipes[0], fortune, PIPE_BUF );
			if( !num_read ) {
				/* Exec() didn't work: quit parent	 */
				perror("Error in read() of FIFO" );
				unlink_fifo();
				exit( EXIT_FAILURE );
			}
			/* Close read fd				 */
			close( file_pipes[0] );
			/* Do all the hard stuff			 */
			format_sig( signature, fortune, num_read );
			/* Send it to the FIFO				 */
			write(fifo_fd, signature, strlen( signature ));
			/* Close FIFO					 */
			close( fifo_fd );
		}
		/* Loop for next signature				 */
		sleep( 1 );
	}
	exit( 0 );
}
