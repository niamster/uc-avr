/** Shell based on freertos Command Interpreter
 */

#ifndef _SH_H_
#define _SH_H_

#define SH_MAX_ARGS 10

/* The structure that defines command line commands.  A command line command
should be defined by declaring a const structure of this type. */
struct shCmd {
	const unsigned char * const cmd;           /* Command name, case sensitive */
	void (*cbk)(int argc, char **argv);  /* Callback function. */
    struct shCmd *next;                        /* Pointer to next command */
};

/*
 * Register the command passed in using the shCmd parameter.
 * Registering a command adds the command to the list of commands that are
 * handled by the command interpreter.  Once a command has been registered it
 * can be executed from the command line.
 * shCmd must be global or static as no copy is made
 */
void shRegisterCmd(struct shCmd *shCmd);

/*
 * Runs the command interpreter for the command input string.
 * Input string is modified during argument parsing
 */
void shProcessCmd(unsigned char *input, int len);

/*
 * Runs the command interpreter for the UART.
 */
void shProcessUart(void);

#endif /* _SH_H_ */
