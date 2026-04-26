#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "ft_ping.h"

static void check_options(int ac, char **av, ping_t *p) {
	int opt;

	while ((opt = getopt(ac, av, "v?")) != -1) {
		if (DEBUG_FLAG) {
			char debug_txt[200];
			sprintf(debug_txt, "MAIN: getopt option_character: %c(%d)", opt, opt);
			debug(debug_txt);
		}
		switch (opt) {
			case 'v':
				p->verbose_flag = 1;
			case '?':
				if (optopt == '?') {
					usage(av[0], "main :: case '?'");
					exit(1);
				}
				continue;
			default:
				break;
		}
	}
}

static int initialize_default_ping(ping_t *p) {
	p->socket_fd = socket(AF_INET, SOCK_RAW, 0);
	if (p->socket_fd == -1)
		return (-1);
	p->target = "";
	p->verbose_flag = 0;
	return (0);
}

int parse_ping(int ac, char **av, ping_t *p) {
	if (initialize_default_ping(p) == -1)
		return (-1);
	check_options(ac, av, p);
	p->target = av[2];
	return (0);
}
