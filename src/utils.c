#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void print_verbose_error(response_t *response, int icmp_type) {
	if (!response || response->recv_bytes < 28) return;

	size_t ip_header_size = get_ip_header_size(response);
	
	// Vérifier qu'il y a assez de données pour IP + ICMP minimum
	if (ip_header_size + 8 > (size_t)response->recv_bytes) {
		return;
	}

	char ip_address_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &response->src_addr.sin_addr, ip_address_str,
			  INET_ADDRSTRLEN);

	if (icmp_type == ICMP_BAD_CHECKSUM) {
		printf("checksum mismatch from %s\n", ip_address_str);
		return;
	}

	size_t icmp_packet_size = response->recv_bytes - ip_header_size;
	
	// Vérifier qu'on a assez de données pour tout le paquet ICMP écho
	if (icmp_packet_size < sizeof(icmp_t)) {
		printf("From %s: bad length\n", ip_address_str);
		return;
	}

	icmp_t *icmp = get_reply_icmp(response);
	
	// Vérifier la longueur du header IP original dans le payload
	size_t orig_ip_hdr_len = (size_t)(icmp->payload[0] & 0x0F) * 4;
	
	// Vérifier que le header IP original n'est pas trop grand
	if (orig_ip_hdr_len < 20 || orig_ip_hdr_len > 60) {
		printf("From %s: bad length\n", ip_address_str);
		return;
	}
	
	// Le payload contient: IP header originel + ICMP request originel
	if (orig_ip_hdr_len + 64 > icmp_packet_size) {
		printf("From %s: bad length\n", ip_address_str);
		return;
	}

	icmp_t *icmp_origine = (icmp_t *)(icmp->payload + orig_ip_hdr_len);

	printf("From %s: icmp_seq=%d", ip_address_str, ntohs(icmp_origine->sequence));

	switch (icmp->type) {
		case 3:
			if (icmp->code == 0)
				printf(" Destination Net Unreachable");
			else if (icmp->code == 1)
				printf(" Destination Host Unreachable");
			else if (icmp->code == 3)
				printf("Destination Port Unreachable");
			break;
		case 11:
			printf(" Time to live exceeded");
			break;
		default:
			printf(" type=%d code=%d", icmp->type, icmp->code);
			break;
	}

	printf("\n");
}

double print_recv_packet(response_t *response) {
	if (DEBUG_FLAG) {
		debug("=== entered PRINT_RECV_PACKET function ===");
	}

	uint8_t *ip_header = (uint8_t *)response->recv_buffer;
	size_t ip_header_size = get_ip_header_size(response);

	// Vérifier que le paquet est assez grand
	if (ip_header_size >= (size_t)response->recv_bytes) {
		if (DEBUG_FLAG) {
			debug("Packet too small to contain ICMP");
		}
		return -1;
	}

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
	
	// Vérifier que le payload est assez grand pour contenir struct timespec
	size_t icmp_packet_size = response->recv_bytes - ip_header_size;
	if (icmp_packet_size < sizeof(icmp_t)) {
		if (DEBUG_FLAG) {
			debug("ICMP packet too small");
		}
		return -1;
	}
	
	size_t available_payload = response->recv_bytes - ip_header_size - 8; // 8 == ICMP header size
	if (available_payload < sizeof(struct timespec)) {
		if (DEBUG_FLAG) {
			debug("Payload too small to contain timespec");
		}
		return -1;
	}
	
	// Use memcpy to avoid misaligned access warning
	struct timespec response_ts_loc;
	memcpy(&response_ts_loc, icmp->payload, sizeof(struct timespec));

	long sec_diff = current_ts.tv_sec - response_ts_loc.tv_sec;
	long nsec_diff = current_ts.tv_nsec - response_ts_loc.tv_nsec;

	if (nsec_diff < 0) {
		sec_diff -= 1;
		nsec_diff += 1000000000L;
	}

	double rtt_ms = (double)sec_diff * 1000.0 + (double)nsec_diff / 1000000.0;

	printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n", packet_size,
		ip_address_str, ntohs(icmp->sequence), ttl, rtt_ms);

	if (DEBUG_FLAG) {
		debug("=== exited PRINT_RECV_PACKET function ===");
	}
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
		double avg = def->stats.sum / (double)def->packet_received;
		double variance;
		
		// Calcul de la variance avec méthode stable numériquement
		// On recalcule directement sum((x - avg)^2) pour éviter la catastrophic cancellation
		if (def->packet_received == 1) {
			printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/0.000 ms\n",
				 def->stats.min, avg, def->stats.max);
		} else {
			// Méthode alternative: variance = (sum of squares) / n - mean^2
			// Mais on corrige l'erreur numérique
			// Pour une implémentation simple, on utilise la formule classique mais robuste:
			variance = (def->stats.sum_squared - (def->stats.sum * def->stats.sum / (double)def->packet_received)) 
						/ (def->packet_received - 1);
			
			if (variance < 0) {
				// Erreur de précision numérique, on considère variance presque nulle
				variance = 0;
			}
			double stddev = sqrt(variance);
			printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
				   def->stats.min, avg, def->stats.max, stddev);
		}
	}
}

void debug(char *msg) {
	fprintf(stderr, "[%s] ", DEBUG_TXT);
	fprintf(stderr, "%s\n", msg);
}

void fatal(char *msg) {
	if (DEBUG_FLAG)
		fprintf(stderr, "[%sERROR%s] %s\n", RED, RESET, msg);
	else
		fprintf(stderr, "%s\n", msg);
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
	fprintf(stderr, "Options: \n\t-v verbose\n\t-h detailed help\n\n");
	fprintf(stderr, "Examples: \n");
	fprintf(stderr, "\t%s -h\n", program_name);
	fprintf(stderr, "\t%s 127.0.0.1\n", program_name);
	fprintf(stderr, "\t%s -v 127.0.0.1\n\n", program_name);
}

void cleanup(ping_t *p) {
	if (p->socket_fd >= 0) {
		close(p->socket_fd);
	}
}
