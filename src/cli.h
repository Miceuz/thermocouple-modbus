#ifndef CLI_H
#define CLI_H

#include<inttypes.h>

#define CMD_COUNT 3

typedef struct {
    char *cmd;
    void (*func)(int argc, char **argv);
} CliCommand;

extern CliCommand commands[CMD_COUNT];
void cmdPoll(uint8_t rx);
#endif
