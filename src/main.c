#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ft_ping.h"

int main(int ac, char **av) {
	uint16_t sequence = 0;
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

	if (DEBUG_FLAG) {
		uint8_t *bytes = (uint8_t *)&datagram;
		for (size_t i = 0; i < sizeof(datagram); i++) {
			printf("%02x ", bytes[i]);
			if ((i + 1) % 16 == 0) printf("\n");
		}
		printf("\n");
	}

	printf("PING %s (%s): %lu\n", def.target, def.resolved_target, sizeof(datagram.payload));
	while (g_sig == 0) {
		if (create_icmp_datagram(&datagram) == -1)
			return 1;
		int ret = ping_once(&def, &datagram, &response);
		if (ret == -1)
			return 1;
		if (ret == 1)
			break;
		print_bytes(&response);
		sequence++;
		datagram.sequence = htons(sequence);
		sleep(1);
		if (g_sig)
			break;
	}
	debug("exit clean after ctrl+c");
	cleanup(&def);
	return 0;
}
