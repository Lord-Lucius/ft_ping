#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ft_ping.h"

int main(int ac, char **av) {
	ping_t *def = NULL;
	if (ac <= 1) {
		usage(av[0], "MAIN :: ac <= 1");
		exit(0);
	}
	if (parse_ping(ac, av, def) == -1) {
		fatal("Coudn't parse arguments\n");
		exit(1);
	}
	return 0;
}
