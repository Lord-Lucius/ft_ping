#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "ft_ping.h"

/*
	PING 127.0.0.1 (127.0.0.1): 56 data bytes
	64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.080 ms
	64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.124 ms
	64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=0.121 ms
	64 bytes from 127.0.0.1: icmp_seq=3 ttl=64 time=0.128 ms
	64 bytes from 127.0.0.1: icmp_seq=4 ttl=64 time=0.130 ms
	64 bytes from 127.0.0.1: icmp_seq=5 ttl=64 time=0.154 ms
	64 bytes from 127.0.0.1: icmp_seq=6 ttl=64 time=0.129 ms
	^C
	--- 127.0.0.1 ping statistics ---
	7 packets transmitted, 7 packets received, 0.0% packet loss
	round-trip min/avg/max/stddev = 0.080/0.124/0.154/0.020 ms
*/

void print_verbose_error(icmp_t *reply, ssize_t bytes_recv) {
	(void)reply, (void)bytes_recv;
	printf("test\n");
}

void print_recv_packet(response_t *response) {

	uint8_t *buf = (uint8_t *)response->recv_buffer;
	size_t ip_header_size = (buf[0] & 0x0F) * 4;

	icmp_t *icmp = (icmp_t *)(response->recv_buffer + ip_header_size);
	uint8_t ttl = buf[8];

	size_t packet_size = response->recv_bytes - ip_header_size;
	char ip_address_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &response->src_addr.sin_addr, ip_address_str,
			  INET_ADDRSTRLEN);

	struct timespec current_ts;
	clock_gettime(CLOCK_MONOTONIC, &current_ts);
	struct timespec *response_ts = (struct timespec *)icmp->payload;

	long sec_diff = current_ts.tv_sec - response_ts->tv_sec;
	long nsec_diff = current_ts.tv_nsec - response_ts->tv_nsec;

	if (nsec_diff < 0) {
		sec_diff -= 1;
		nsec_diff += 1000000000;
	}

	double rtt_ms = sec_diff * 1000.0 + nsec_diff / 1000000.0;

	printf("%zd bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n", packet_size,
		   ip_address_str, ntohs(icmp->sequence), ttl, rtt_ms);
}

void print_exiting_stats(ping_t *def) {
	printf("\n--- %s ping statistics ---\n", def->target);
	printf("%d packets transmitted, %d packets received, %.1f%% packet loss\n",
		   def->packet_sended, def->packet_received,
		   ((float)(def->packet_sended - def->packet_received) * 100.0F) /
			   (float)def->packet_sended);
}

void print_bytes(response_t *response) {
	printf("Received %zd bytes:\n", response->recv_bytes);
	for (ssize_t i = 0; i < response->recv_bytes; i++) {
		printf("%02x ", ((uint8_t *)response->recv_buffer)[i]);
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}
	printf("\n");
}

void debug(char *msg) {
	fprintf(stderr, "[%s] ", DEBUG_TXT);
	fprintf(stderr, "%s\n", msg);
}

void fatal(char *msg) {
	fprintf(stderr, "[%sERROR%s] %s", RED, RESET, msg);
}

void usage(char *program_name, char *call_position) {
	if (DEBUG_FLAG) {
		char debug_txt[200];
		snprintf(debug_txt, sizeof(debug_txt),
				 "USAGE: function call position: %s", call_position);
		debug(debug_txt);
	}
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "\t%s [options] <destination>\n\n", program_name);
	fprintf(stderr, "Options: \n\t-v verbose\n\t-? detailed help\n\n");
	fprintf(stderr, "Examples: \n");
	fprintf(stderr, "\t%s -?\n", program_name);
	fprintf(stderr, "\t%s 127.0.0.1\n", program_name);
	fprintf(stderr, "\t%s -v 127.0.0.1\n\n", program_name);
}

void cleanup(ping_t *p) {
	if (p->socket_fd >= 0) {
		close(p->socket_fd);
	}
}
