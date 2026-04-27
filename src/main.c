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
	if (create_icmp_datagram(&datagram))
		return 1;
	show_ping(&def);
	cleanup(&def);
	return 0;
}
