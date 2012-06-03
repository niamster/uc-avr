#include <string.h>

#include <inttypes.h>
#include <ctype.h>

#include <uart/uart.h>
#include <sh/sh.h>

static void shHelp(int argc, char **argv);
static struct shCmd shHelpCmd = {
	"help",
	shHelp,
    NULL
};

static struct shCmd *shRegisteredCmd = &shHelpCmd;

void shRegisterCmd(struct shCmd *shCmd)
{
    struct shCmd *first = shRegisteredCmd;

    shCmd->next = first->next;
    first->next = shCmd;
}

static void shHelp(int argc, char **argv)
{
    struct shCmd *cmd = shRegisteredCmd;

    while (cmd) {
        usart_puts(cmd->cmd);
        cmd = cmd->next;
        if (cmd)
            usart_putc(' ');
    }
    usart_puts("\r\n");
}

void shProcessCmd(unsigned char *input, int len)
{
    char *argv[SH_MAX_ARGS+1];
    int argc = 0;
    struct shCmd *cmd = shRegisteredCmd;

    while (cmd) {
        int l = strlen(cmd->cmd);
        if (!strncmp(cmd->cmd, input, l)) {
            len -= l;
            input += l;

          parse_args:
            while (len && argc < SH_MAX_ARGS) {
                int i;
                for (i=0;i<len;++i) {
                    if (isblank(input[i])) {
                        if (i == 0) {
                            --len;
                            ++input;
                            goto parse_args;
                        }

                        input[i] = 0x0;

                        argv[argc] = input;
                        ++argc;

                        len -= i + 1;
                        input += i + 1;
                        goto parse_args;
                    }
                }

                if (i == len && i > 0) {
                    input[i] = 0x0;

                    argv[argc] = input;
                    ++argc;

                    break;
                }
            }
            argv[argc] = NULL;
            cmd->cbk(argc, argv);
            return;
        }

        cmd = cmd->next;
    }

    usart_puts("Unknown command. List of registered commands: ");
    shHelp(0, NULL);
}

void shProcessUart(void)
{
    static unsigned char cmd[SH_MAX_CMD_LEN];
    static unsigned char *pCmd = cmd;
    static unsigned char len = 0;
    static unsigned char first = 1;

    if (first) {
        usart_puts("\r\n>");
        first = 0;
    }

    if (usart_bytes_available() > 0) {
        usart_read(pCmd, 1);
        if (*pCmd == 0x08) {
            if (len > 0) {
                usart_putc(0x08);
                usart_putc(0x20);
                usart_putc(0x08);
                --pCmd;
                --len;
            }

            return;
        }

        usart_putc(*pCmd);

        ++pCmd;
        ++len;

        if (*(pCmd-1) == '\r') {
            usart_putc('\n');
            if (len > 1) {
                shProcessCmd(cmd, len-1);
            }
            usart_putc('>');
            pCmd = cmd;
            len = 0;
        }
    }
}
