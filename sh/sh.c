#include <string.h>

#include <inttypes.h>
#include <ctype.h>

#include <uart/uart.h>
#include <sh/sh.h>
#include <mi/mi.h>

static void sh_help(int argc, char **argv);
static struct sh_cmd sh_help_cmd = {
	"help",
	sh_help,
    NULL
};

static struct sh_cmd *sh_registered_cmd = &sh_help_cmd;

extern struct sh_cmd __sh_start[], __sh_end[];

static
void sh_init(void)
{
    struct sh_cmd *first = sh_registered_cmd;
    struct sh_cmd *s = __sh_start;
    struct sh_cmd *e = __sh_end;

    while (s < e) {
        s->next = first->next;
        first->next = s;
        ++s;
    }

    usart_puts("\r\n>");
}
MI_INIT_MODULE(999, sh_init);

static void sh_help(int argc, char **argv)
{
    struct sh_cmd *cmd = sh_registered_cmd;

    while (cmd) {
        usart_puts(cmd->cmd);
        cmd = cmd->next;
        if (cmd)
            usart_putc(' ');
    }
    usart_puts("\r\n");
}

void sh_process_cmd(unsigned char *input, int len)
{
    char *argv[SH_MAX_ARGS+1];
    int argc = 0;
    struct sh_cmd *cmd = sh_registered_cmd;

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
    sh_help(0, NULL);
}

void sh_process_uart(void)
{
    static unsigned char cmd[SH_MAX_CMD_LEN];
    static unsigned char *pCmd = cmd;
    static unsigned char len = 0;

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
                sh_process_cmd(cmd, len-1);
            }
            usart_putc('>');
            pCmd = cmd;
            len = 0;
        }
    }
}
