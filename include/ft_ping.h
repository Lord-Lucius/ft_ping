#ifndef MAIN_H
# define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
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

#define DEBUG_FLAG 0
#define DEBUG_TXT RED "DEBUG" RESET

#define BUFFER_SIZE 1024

typedef struct response_s {
	char recv_buffer[BUFFER_SIZE];
	struct sockaddr_in src_addr;
	socklen_t src_len;
	ssize_t send_bytes;
	ssize_t recv_bytes;
} response_t;

typedef struct icmp_s {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t identifier;
	uint16_t sequence;
	uint8_t payload[56];
} icmp_t;

typedef struct ping_s {
	int socket_fd;
	char *target;
	char resolved_target[20];
	struct sockaddr_in dest_addr;
	int verbose_flag;
} ping_t;

int parse_ping(int ac, char **av, ping_t *p);
int resolve_addr(ping_t *p);
int create_icmp_datagram(icmp_t *datagram);
int ping_once(ping_t *def, icmp_t *datagram, response_t *response);

void print_bytes(response_t *response);
void debug(char *msg);
void fatal(char *msg);
void usage(char *program_name, char *call_position);

void cleanup(ping_t *p);

#endif
