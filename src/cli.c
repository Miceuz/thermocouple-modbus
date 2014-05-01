#include <inttypes.h>
#include <string.h>
#include "cli.h"

#define CMD_BUFF_LENGTH 64
static volatile unsigned char cmdCharIndex = 0;
static volatile unsigned char cmdBuff[CMD_BUFF_LENGTH+1];

void cmdParseBuffer() {
    uint8_t argc, i = 0;
    char *argv[6];

    argv[i] = strtok(cmdBuff, " ");
    do
    {
        argv[++i] = strtok(NULL, " ");
    } while ((i < 6) && (argv[i] != NULL));
    
    // save off the number of arguments for the particular command.
    argc = i;

    for(i = 0; i < CMD_COUNT; i++) {
       if (!strcmp(argv[0], commands[i].cmd)) {
            commands[i].func(argc, argv);
            return;
        }
    }

    // command not recognized. print message and re-generate prompt.
	usartPuts(argv[0]);
	usartPuts(" - unknown command.\r\n");
}

void cmdPoll(uint8_t rx) {
	if('\n' == rx || '\0' == rx) {
		//NOTHING
	} else if('\b' == rx || 127 == rx) {
		if(cmdCharIndex > 0) {
			cmdCharIndex --;
			uartPutc(8);
			uartPutc(' ');
			uartPutc(8);
		}
	} else if(rx == '\r') {
		usartPuts("\r\n");
		cmdBuff[cmdCharIndex] = '\0';
		cmdParseBuffer();
		cmdCharIndex = 0;
	} else {
		if(cmdCharIndex < CMD_BUFF_LENGTH) {
			//~ uartPutc(cmdCharIndex+48);
			//~ uartPutc('[');
			//~ usartPuts(itoa(rx, buffer, 10));
			//~ uartPutc(']');
			cmdBuff[cmdCharIndex++] = rx;
			uartPutc(rx);
		}
	}
}
