#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "ft_ping.h"

int run(ping_t *def, icmp_t *datagram, response_t *response) {
	uint16_t sequence = 0;

	create_icmp_datagram(datagram);
	if (ping_send(def, datagram, response) == -1)
		return -1;
	sequence++;

	start_time();
	printf("PING %s (%s): %lu\n", def->target, def->resolved_target, sizeof(datagram->payload));
	while (!g_sig) {
		ssize_t n = recvfrom(def->socket_fd, response->recv_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&response->src_addr, &response->src_len);
		printf("%lu\n", n);
		if (n < 0) {
			if (errno == EINTR) {
				if (g_alarm) {
					create_icmp_datagram(datagram);
					datagram->sequence = htons(sequence);
					ping_send(def, datagram, response);
					sequence++;
					g_alarm = 0;
				}
				continue;
			}
			fatal("recvfrom failed");
			return -1;
		}
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
