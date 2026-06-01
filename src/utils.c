#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include "ft_ping.h"

#if DEBUG_FLAG
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
#endif

void print_verbose_error(response_t *response) {
	if (!response || response->recv_bytes < 28)
		return;

	char ip_address_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &response->src_addr.sin_addr, ip_address_str,
			  INET_ADDRSTRLEN);

	uint8_t *ip_header = (uint8_t *)response->recv_buffer;
	uint8_t received_ttl = ip_header[8];

	icmp_t *icmp = get_reply_icmp(response);

	printf("%zd bytes from %s: icmp_seq=%d ttl=%d", response->recv_bytes,
		   ip_address_str, (icmp ? icmp->sequence : 0), received_ttl);

	if (icmp) {
		switch (icmp->type) {
		case 0:
			printf(" (Echo Reply)");
			break;
		case 11:
			printf(" (Time to live exceeded)");
			break;
		default:
			printf(" (type=%d code=%d)", icmp->type, icmp->code);
			break;
		}
	}

	printf("\n");
}

double print_recv_packet(response_t *response) {

	uint8_t *ip_header = (uint8_t *)response->recv_buffer;
	size_t ip_header_size = get_ip_header_size(response);

	icmp_t *icmp = get_reply_icmp(response);
	uint8_t ttl = ip_header[8];

	size_t packet_size = response->recv_bytes - ip_header_size;
	char ip_address_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &response->src_addr.sin_addr, ip_address_str,
			  INET_ADDRSTRLEN);

	struct timespec current_ts;
	if (clock_gettime(CLOCK_MONOTONIC, &current_ts) == -1) {
		fatal("clock_gettime failed");
		return -1;
	}
	struct timespec *response_ts = (struct timespec *)icmp->payload;

	long sec_diff = current_ts.tv_sec - response_ts->tv_sec;
	long nsec_diff = current_ts.tv_nsec - response_ts->tv_nsec;

	if (nsec_diff < 0) {
		sec_diff -= 1;
		nsec_diff += 1000000000;
	}

	double rtt_ms = (double)sec_diff * 1000.0 + (double)nsec_diff / 1000000.0;

	printf("%zd bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n", packet_size,
		   ip_address_str, ntohs(icmp->sequence), ttl, rtt_ms);
	return rtt_ms;
}

void print_exiting_stats(ping_t *def) {
	printf("\n--- %s ping statistics ---\n", def->target);
	if (def->packet_sent) {
		printf(
			"%d packets transmitted, %d packets received, %.1f%% packet loss\n",
			def->packet_sent, def->packet_received,
			((float)(def->packet_sent - def->packet_received) * 100.0F) /
				(float)def->packet_sent);
	}
	if (def->packet_received) {
		double avg = def->stats.sum / (def->packet_received);
		double variance =
			(def->stats.sum_squared / (def->packet_received)) - (avg * avg);
		if (variance < 0) variance = 0;
		double stddev = sqrt(variance);
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			   def->stats.min, avg, def->stats.max, stddev);
	}
}

void debug(char *msg) {
	fprintf(stderr, "[%s] ", DEBUG_TXT);
	fprintf(stderr, "%s\n", msg);
}

void fatal(char *msg) {
	fprintf(stderr, "[%sERROR%s] %s\n", RED, RESET, msg);
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
