#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
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

void print_recv_packet(response_t *response) {
	(void)response;
	printf("<packet_size> bytes from <ip_address>: icmp_seq=<packet_number> ttl=<size_i_guess> time=<time_to_receive> ms\n");
}

void print_bytes(response_t *response) {
	printf("Received %zd bytes:\n", response->recv_bytes);
	for (ssize_t i = 0; i < response->recv_bytes; i++) {
		printf("%02x ", ((uint8_t *)response->recv_buffer)[i]);
		if ((i + 1) % 16 == 0)
			printf("\n");
	}
	printf("\n");
}

void debug(char *msg) {
	fprintf(stderr, "[%s] ", DEBUG_TXT);
	fprintf(stderr, "%s\n", msg);
}

void fatal(char *msg) { fprintf(stderr, "[%sERROR%s] %s", RED, RESET, msg); }

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
	if (p->socket_fd >= 0)
		close(p->socket_fd);
}
