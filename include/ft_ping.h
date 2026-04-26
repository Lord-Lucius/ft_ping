#ifndef MAIN_H
# define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define DEBUG_FLAG 1
#define DEBUG_TXT RED "DEBUG" RESET

typedef struct ping_s {
	int socket_fd;
	char *target;
	int verbose_flag;
} ping_t;

int parse_ping(int ac, char **av, ping_t *p);

void debug(char *msg);
void fatal(char *msg);
void usage(char *program_name, char *call_position);

#endif
