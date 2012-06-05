#ifndef _SH_H_
#define _SH_H_

/* The structure that defines command line commands.  A command line command
should be defined by declaring a const structure of this type. */
struct sh_cmd {
	const unsigned char * const cmd;     /* Command name, case sensitive */
	void (*cbk)(int argc, char **argv);  /* Callback function */
    struct sh_cmd *next;                 /* Pointer to next command */
};

#define SH_LINK_SECTION __attribute__ ((section (".sh"))) __attribute__ ((used))

#define SH_DECLARE_CMD(c, f)                                         \
    static struct sh_cmd __sh_ ## f SH_LINK_SECTION = {              \
        .cmd = c,                                                    \
        .cbk = f,                                                    \
        .next = NULL,                                                \
    }

/*
 * Runs the command interpreter for the command input string.
 * Input string is modified during argument parsing
 */
void sh_process_cmd(unsigned char *input, int len);

/*
 * Runs the command interpreter for the UART.
 */
void sh_process_uart(void);

#endif /* _SH_H_ */
