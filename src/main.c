#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ft_ping.h"

static void update_stats(rtt_stat_t *stats, double rtt) {
	if (rtt < stats->min) stats->min = rtt;
	if (rtt > stats->max) stats->max = rtt;
	stats->sum += rtt;
	stats->sum_squared += rtt * rtt;
}

static int handle_send(ping_t *def, icmp_t *datagram, response_t *response,
					   uint16_t sequence) {
	if (create_icmp_datagram(datagram) == -1) return -1;
	datagram->sequence = htons(sequence);
	datagram->checksum = 0;
	datagram->checksum = calculate_checksum(datagram, sizeof(*datagram));

	if (ping_send(def, datagram, response) == -1) return -1;

	def->packet_sent++;
	g_signals &= ~SIG_ALRM_BIT;
	return 0;
}

static int handle_recv(ping_t *def, response_t *response, ssize_t n) {
	response->recv_bytes = n;
	int icmp_type = check_icmp_type(response);

	if (icmp_type != ICMP_FOR_US) {
		if (def->verbose_flag &&
			(icmp_type == ICMP_ANOMALY || icmp_type == ICMP_BAD_CHECKSUM))
			print_verbose_error(response, icmp_type);
		return 0;
	}

	def->packet_received++;
	double rtt = print_recv_packet(response);

	if (rtt < 0) return -1;

	update_stats(&def->stats, rtt);
	return 0;
}

int run(ping_t *def, icmp_t *datagram, response_t *response) {
	if (DEBUG_FLAG) debug("=== entered RUN function ===");

	uint16_t sequence = 0;

	printf("PING %s (%s): %lu data bytes\n", def->target, def->resolved_target,
		   sizeof(datagram->payload));

	if (TTL_TEST_FLAG == 1) {
		int ttl = 1;
		setsockopt(def->socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
	}
	if (start_timer() == -1) {
		fatal("timer couldn't start");
		return -1;
	}

	while (!(g_signals & SIG_INT_BIT)) {
		response->src_len = sizeof(response->src_addr);

		ssize_t n = recvfrom(def->socket_fd, response->recv_buffer, BUFFER_SIZE,
							 0, (struct sockaddr *)&response->src_addr,
							 &response->src_len);

		if (n < 0) {
			if (errno != EINTR) {
				fatal("recvfrom failed");
				return -1;
			}
			if ((g_signals & SIG_ALRM_BIT) &&
				handle_send(def, datagram, response, sequence++) == -1)
					return -1;
			continue;
		}

		if (handle_recv(def, response, n) == -1) return -1;
	}

	if (DEBUG_FLAG) debug("=== exited RUN function ===");
	return 0;
}

int main(int ac, char **av) {
	if (DEBUG_FLAG) {
		debug("=== entered main function ===");
	}

	ping_t def = {0};
	icmp_t datagram = {0};
	response_t response = {0};

	if (ac <= 1) {
		usage(av[0], "MAIN :: ac <= 1");
		exit(2);
	}
	set_sigaction();
	if (parse_ping(ac, av, &def) == -1) {
		return (1);
	}
	if (resolve_addr(&def) == -1) {
		return (1);
	}

	if (run(&def, &datagram, &response) == -1) {
		return (1);
	}

	print_exiting_stats(&def);
	cleanup(&def);

	if (DEBUG_FLAG) {
		debug("=== exited main function ===");
	}
	return ((def.packet_received == 0) ? 1 : 0);
}
