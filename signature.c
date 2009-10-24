/* signature.c -- create a dynamic signature facility.

   $Id: signature.c,v 1.1.1.1 2009/10/24 17:48:09 reynolds Exp $

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

/* name of default signature file */
#define DEFAULT_FIFO ".signature"

/* *INDENT-OFF* */

/* text of default signature */
#define SIGNATURE "\
Linux, the choice          | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\
of a GNU generation   -o)  | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\
Kernel %-9.9s       /\\  | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\
on a %-6.6s           _\\_v | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\
                           | <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"

/* *INDENT-ON* */

static char fifo[BUF_SIZE];
static char quote_file[BUF_SIZE];
unsigned short int debug = 0;

/* display version information */
void version(void)
{
    printf("%s version %s ("
	   __DATE__ " " __TIME__ ")\n\n"
	   "Copyright (C) 1999-2000 Ian Macdonald <ian@caliban.org>\n"
	   "This is free software; see the source for copying conditions.\n"
	   "There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
	   "FOR A PARTICULAR PURPOSE, to the extent permitted by law.\n",
	   PACKAGE, VERSION);
    exit(EXIT_SUCCESS);
}

/* display help message */
void help(int status)
{
    printf
	("Usage: %s [ -d ] [ -f FIFO ] [ -n ] [ -p program | -q quote file ]\n"
	 "                 [ -r ] [ -t template ]\n"
	 "       %s -h | -v\n\n"
	 "-d              print debugging messages on stderr\n"
	 "-f <FIFO>       use this file as FIFO instead of ~/.signature\n"
	 "-h              display this help message\n"
	 "-n              don't fork into background as daemon\n"
	 "-p <program>    use this program instead of fortune(6)\n"
	 "-q <quote file> pluck a random quote from this file instead of from fortune(6)\n"
	 "-r              force removal of old signature FIFO if present\n"
	 "-t <template>   use this signature template instead of default\n"
	 "-v              display version information\n\n"
	 "E-mail bug reports and diffs to: ian@caliban.org\n", PACKAGE,
	 PACKAGE);
    exit(status);
}

/* remove the FIFO and exit */
void unlink_fifo(int status)
{
    if (debug) {
	fprintf(stderr, "Unlinking FIFO...\n");
    }
    unlink(fifo);
    exit(status);
}

/* when we catch a signal */
void caught_signal(int sig)
{
    if (debug) {
	fprintf(stderr, "Caught signal %d. Clearing up...\n", sig);
    }
    unlink_fifo(EXIT_SUCCESS);
}

/* ignore SIGPIPE */
void ignore_sigpipe(int sig)
{
    if (debug) {
	fprintf(stderr, "Caught a SIGPIPE. Using Pine by any chance?\n");
    }
}

/* set up FIFO */
void init_fifo(unsigned short int remove_flag)
{
    if (access(fifo, F_OK) == 0 && remove_flag) {
	/* FIFO already exists */
	if (unlink(fifo) == -1) {
	    fprintf(stderr, "Couldn't unlink %s\n", fifo);
	    exit(EXIT_FAILURE);
	}
    }
    if (mkfifo(fifo, 0644) == -1) {
	fprintf(stderr, "Couldn't create %s as FIFO\n", fifo);
	exit(EXIT_FAILURE);
    }
}

/* install signal handler */
void install_sig_handler(void)
{
    struct sigaction act;

    /* install signal handler */
    act.sa_handler = caught_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
    sigaction(SIGHUP, &act, 0);
    sigaction(SIGTERM, &act, 0);
    act.sa_handler = ignore_sigpipe;
    sigemptyset(&act.sa_mask);
    sigaction(SIGPIPE, &act, 0);
}

int main(int argc, char *argv[])
{
    char signature[BUF_SIZE], sig_buf[BUF_SIZE], fortune[BUF_SIZE];
    char *producer = NULL, *prodarg = NULL;
    char *prodargs[MAX_PROG_ARGS];
    char *slash_ptr, *cwd_ptr;
    int prodargc = 0;
    unsigned short int no_fork = 0;
    int exit_status = 0;
    int sig_fd, sig_buf_len = 0;
    int opt, pid;
    int file_pipes[2], fifo_fd, num_read, remove_flag = 0;
    int total = 0;
    FILE *quote_stream;
    struct sig_node *first_node;
    /* got to use a pointer to a pointer to a struct here. Don't like this -- 
       too complicated */
    struct sig_node **first_node_ptr = &first_node;

    memset(&fifo, 0, BUF_SIZE);
    memset(&quote_file, 0, BUF_SIZE);
    /* get option switches */
    while ((opt = getopt(argc, argv, "df:hnp:q:rt:v")) != -1) {
	switch (opt) {
	case 'd':
	    /* set debugging flag */
	    debug = 1;
	    break;
	case 'f':
	    /* get location of FIFO */
	    slash_ptr = strrchr(optarg, '/');
	    if (slash_ptr == NULL || *optarg == '.') {
		/* make path absolute */
		cwd_ptr = getcwd(NULL, BUF_SIZE);
		sprintf(fifo, "%s/%s", cwd_ptr, optarg);
		free(cwd_ptr);
	    } else {
		strcpy(fifo, optarg);
	    }
	    if (debug) {
		fprintf(stderr, "FIFO is %s\n", fifo);
	    }
	    break;
	case 'h':
	    help(EXIT_SUCCESS);
	    break;
	case 'n':
	    no_fork = 1;
	    break;
	case 'p':
	    if (quote_file[0] != '\0') {
		help(EXIT_FAILURE);
	    }
	    producer = optarg;
	    prodargs[prodargc++] = strtok(producer, " ");
	    while ((prodarg = strtok(NULL, " ")) != NULL) {
		prodargs[prodargc++] = prodarg;
		if (prodargc > MAX_PROG_ARGS) {
		    fprintf(stderr, "More than %d arguments to %s. "
			    "Excess arguments ignored...\n",
			    MAX_PROG_ARGS, producer);
		    break;
		}
	    }
	    prodargs[prodargc] = NULL;
	    /* check for existence of program by forking and then trying to
	       exec() it in the child */
	    pid = fork();
	    switch (pid) {
	    case -1:		/* oh well */
		perror("Couldn't fork() a child process");
		exit(EXIT_FAILURE);
	    case 0:		/* in child */
		/* close stdout */
		close(1);
		execlp(producer, producer, (char *) 0);
		exit(EXIT_FAILURE);
	    default:
		waitpid(pid, &exit_status, 0);
		if (exit_status != EXIT_SUCCESS) {
		    fprintf(stderr, "Program %s doesn't seem to exist\n",
			    producer);
		    exit(EXIT_FAILURE);
		}
	    }
	    break;
	case 'q':
	    if (producer) {
		help(EXIT_FAILURE);
	    }
	    slash_ptr = strrchr(optarg, '/');
	    if (slash_ptr == NULL || *optarg == '.') {
		/* make path absolute */
		/* get the current directory */
		cwd_ptr = getcwd(NULL, BUF_SIZE);
		/* get absolute path of quote_file */
		sprintf(quote_file, "%s/%s", cwd_ptr, optarg);
		free(cwd_ptr);
	    } else {
		/* get a quote from an external file instead of fortune */
		strcpy(quote_file, optarg);
	    }
	    break;
	case 'r':
	    remove_flag = 1;
	    break;
	case 't':
	    if (debug) {
		fprintf(stderr, "Opening template file %s...\n", optarg);
	    }
	    sig_fd = open(optarg, O_RDONLY);
	    if (sig_fd == -1) {
		fprintf(stderr, "Couldn't open template %s for reading\n",
			optarg);
		exit(EXIT_FAILURE);
	    }
	    memset(&sig_buf, 0, BUF_SIZE);
	    sig_buf_len = read(sig_fd, &sig_buf, BUF_SIZE);
	    if (sig_buf_len == -1) {
		perror("Failed to read() template file");
		exit(EXIT_FAILURE);
	    }
	    close(sig_fd);
	    break;
	case 'v':
	    version();
	    break;
	case ':':
	    fprintf(stderr, "Option needs a value\n");
	    break;
	case '?':
	    exit(EXIT_FAILURE);
	}
    }
    if (fifo[0] == '\0') {
	/* set up the path to default FIFO */
	sprintf(fifo, "%s/%s", getenv("HOME"), DEFAULT_FIFO);
    }
    /* initialise FIFO */
    init_fifo(remove_flag);
    /* install signal handler */
    install_sig_handler();

    if (!no_fork) {
	/* fork off into background */
	pid = fork();
	switch (pid) {
	case -1:		/* oh well */
	    perror("Couldn't fork() a child process");
	    unlink_fifo(EXIT_FAILURE);
	case 0:		/* in child */
	    /* become a process group and session group leader */
	    setsid();
	    /* fork off into background */
	    pid = fork();
	    switch (pid) {
	    case -1:		/* oh well */
		perror("Couldn't fork a child process");
		unlink_fifo(EXIT_FAILURE);
	    case 0:		/* in child */
		if (debug) {
		    fprintf(stderr, "PID is %d\n", getpid());
		}
		/* ensure our process doesn't keep any directory in use */
		chdir("/");
		break;
	    default:
		exit(EXIT_SUCCESS);
	    }
	    break;
	default:
	    exit(EXIT_SUCCESS);
	}
    }

    if (quote_file[0] != '\0') {
	quote_stream =
	    index_quote_file(quote_file, &total, first_node_ptr);
    }

    /* start main loop */
    while (1) {
	memset(&signature, 0, BUF_SIZE);
	if (sig_buf_len > 0) {
	    /* use user-supplied template */
	    strcpy(signature, sig_buf);
	} else {
	    /* use stock signature */
	    strcpy(signature, SIGNATURE);
	}

	if (access(fifo, F_OK) != 0) {
	    fprintf(stderr, "%s seems to have disappeared\n", fifo);
	    exit(EXIT_FAILURE);
	}

	/* open FIFO for writing */
	fifo_fd = open(fifo, O_WRONLY);
	if (fifo_fd == -1) {
	    fprintf(stderr, "Couldn't open %s for writing\n", fifo);
	    unlink_fifo(EXIT_FAILURE);
	}
	/* create a pipe */
	if (pipe(file_pipes) != 0) {
	    perror("Couldn't create pipe() for passing signature");
	    unlink_fifo(EXIT_FAILURE);
	}
	/* fork off a child */
	pid = fork();
	switch (pid) {
	case -1:		/* oh well */
	    perror("Couldn't fork() a child process");
	    unlink_fifo(EXIT_FAILURE);
	case 0:		/* in child */
	    /* close stdout */
	    close(1);
	    /* duplicate write fd as stdout */
	    dup(file_pipes[1]);
	    /* close write fd */
	    close(file_pipes[1]);
	    /* close read fd */
	    close(file_pipes[0]);

	    /* check if we need to print from a quote file */
	    if (quote_file[0] != '\0') {
		exit(print_quote(quote_stream, total, first_node));
	    } else if (!producer) {	/* call fortune */
		if (debug) {
		    fprintf(stderr, "Calling %s...\n", FORTUNE);
		}
		/* send fortune cookie to write fh */
		execl(FORTUNE, "fortune", "-s"
#ifdef OFFENSIVE_FORTUNES
		      "a"
#endif
		      , (char *) 0);
		/* we only reach this point if exec() didn't work */
		fprintf(stderr, "Couldn't exec %s\n", FORTUNE);
		exit(EXIT_FAILURE);
	    } else {		/* external program */
		if (debug) {
		    fprintf(stderr, "Calling %s...\n", producer);
		}
		/* send producer's output to write fh */
		execvp(producer, prodargs);
		/* we only reach this point if exec() didn't work */
		fprintf(stderr, "Couldn't exec %s\n", producer);
		exit(EXIT_FAILURE);
	    }
	default:		/* in parent */
	    if (debug) {
		fprintf(stderr, "Child process PID %d...\n", pid);
	    }
	    /* clean up after child */
	    waitpid(pid, (int *) 0, 0);
	    /* close write fd */
	    close(file_pipes[1]);
	    /* read fortune cookie through pipe */
	    num_read = read(file_pipes[0], fortune, PIPE_BUF);
	    if (!num_read) {
		/* exec() didn't work: quit parent */
		perror("Error in read() of FIFO");
		unlink_fifo(EXIT_FAILURE);
	    }
	    /* close read fd */
	    close(file_pipes[0]);
	    /* do all the hard stuff */
	    format_sig(signature, fortune, num_read);
	    /* send it to the FIFO */
	    write(fifo_fd, signature, strlen(signature));
	    /* close FIFO */
	    close(fifo_fd);
	}
	/* loop for next signature */
	sleep(1);
    }
}
