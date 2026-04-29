#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ft_ping.h"

int run(ping_t *def, icmp_t *datagram, response_t *response) {
	uint16_t sequence = 0;

	printf("PING %s (%s): %lu bytes\n", def->target, def->resolved_target,
		   sizeof(datagram->payload));

	start_time();

	while (!g_sig) {
		response->src_len = sizeof(response->src_addr);

		ssize_t n = recvfrom(def->socket_fd, response->recv_buffer, BUFFER_SIZE,
							 0, (struct sockaddr *)&response->src_addr,
							 &response->src_len);

		if (n < 0) {
			if (errno == EINTR) {
				if (g_alarm) {
					create_icmp_datagram(datagram);
					datagram->sequence = htons(sequence);
					datagram->checksum = 0;
					datagram->checksum =
						calculate_checksum(datagram, sizeof(*datagram));

					if (ping_send(def, datagram, response) == -1)
						return -1;

					sequence++;
					g_alarm = 0;
				}
				continue;
			}
			fatal("recvfrom failed");
			return -1;
		}

		response->recv_bytes = n;
		print_bytes(response);
	}

	return 0;
}

int main(int ac, char **av) {
	ping_t def;
	icmp_t datagram;
	response_t response;

	if (ac <= 1) {
		usage(av[0], "MAIN :: ac <= 1");
		exit(0);
	}
	set_sigaction();
	if (parse_ping(ac, av, &def) == -1)
		return 1;
	if (resolve_addr(&def) == -1)
		return 1;
	bzero(&datagram, sizeof(datagram));

	if (run(&def, &datagram, &response) == -1)
		return 1;

	debug("exit clean after ctrl+c");
	cleanup(&def);
	return 0;
}
