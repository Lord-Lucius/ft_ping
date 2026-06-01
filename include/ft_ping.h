#ifndef FT_PING_H
#define FT_PING_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include <netinet/in.h>

#define RESET "\033[0m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

#define DEBUG_FLAG 0
#define DEBUG_TXT RED "DEBUG" RESET

#define BUFFER_SIZE 1024
#define PAYLOAD_SIZE 56

extern volatile sig_atomic_t g_sig;
extern volatile sig_atomic_t g_alarm;

typedef struct rtt_stat_s {
	double min;
	double max;
	double sum;
	double sum_squared;
} rtt_stat_t;

typedef enum icmp_filter_e {
	ICMP_FOR_US = 0,
	ICMP_ANOMALY = 1,
	ICMP_OUR_REQUEST = 2,
	ICMP_BAD_CHECKSUM = 3
} icmp_filter_t;

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
	uint8_t payload[PAYLOAD_SIZE];
} icmp_t;

typedef struct ping_s {
	int socket_fd;
	char *target;
	char resolved_target[INET_ADDRSTRLEN];
	struct sockaddr_in dest_addr;
	int verbose_flag;
	int packet_sent;
	int packet_received;
	rtt_stat_t stats;
} ping_t;

void set_sigaction(void);
int start_timer(void);

int parse_ping(int ac, char **av, ping_t *p);
int resolve_addr(ping_t *p);
int create_icmp_datagram(icmp_t *datagram);
int ping_send(ping_t *def, icmp_t *datagram, response_t *response);
uint16_t calculate_checksum(void *data, size_t len);
icmp_t *get_reply_icmp(response_t *response);
size_t get_ip_header_size(response_t *response);
int check_icmp_type(response_t *response);

void print_verbose_error(response_t *response);
double print_recv_packet(response_t *response);
void print_exiting_stats(ping_t *def);
void print_bytes(response_t *response);
void debug(char *msg);
void fatal(char *msg);
void usage(char *program_name, char *call_position);

void cleanup(ping_t *p);

#endif
