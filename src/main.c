#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ft_ping.h"

int main(int ac, char **av) {
	ping_t def;
	icmp_t datagram;

	if (ac <= 1) {
		usage(av[0], "MAIN :: ac <= 1");
		exit(0);
	}
	if (parse_ping(ac, av, &def) == -1)
		return 1;
	if (resolve_addr(&def) == -1)
		return 1;
	bzero(&datagram, sizeof(datagram));
	if (create_icmp_datagram(&datagram))
		return 1;
	uint8_t *bytes = (uint8_t *)&datagram;
	for (size_t i = 0; i < sizeof(datagram); i++) {
		printf("%02x ", bytes[i]);
		if ((i + 1) % 16 == 0) printf("\n");
	}
	printf("\n");
	show_ping(&def, &datagram);
	cleanup(&def);
	return 0;
}
