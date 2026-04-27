#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "ft_ping.h"

int resolve_addr(ping_t *p) {
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;

	int ret = getaddrinfo(p->target, 0, &hints, &res);
	if (ret != 0) {
		char tmp[200];
		snprintf(tmp, sizeof(tmp), "Failed to resolved destination address (gai_strerror: %s)\n", gai_strerror(ret));
		fatal(tmp);
		return -1;
	}
	memcpy(&p->dest_addr, res->ai_addr, sizeof(struct sockaddr_in));
	if (inet_ntop(AF_INET, &(p->dest_addr.sin_addr), p->resolved_target, INET_ADDRSTRLEN) == NULL) {
		char tmp[200];
		snprintf(tmp, sizeof(tmp), "Failed to transform network addr to printable addr (errno: %s)\n", strerror(errno));
		fatal(tmp);
		return -1;
	}
	freeaddrinfo(res);
	return 0;
}

static int check_options(int ac, char **av, ping_t *p) {
	int opt;
	int ret_flag = 0;

	while ((opt = getopt(ac, av, "v?")) != -1) {
		if (DEBUG_FLAG) {
			char debug_txt[200];
			snprintf(debug_txt, sizeof(debug_txt), "MAIN: getopt option_character: %c(%d)", opt, opt);
			debug(debug_txt);
		}
		switch (opt) {
			case 'v':
				p->verbose_flag = 1;
				ret_flag = 1;
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
	return ret_flag;
}

static int initialize_default_ping(ping_t *p) {
	p->socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (p->socket_fd == -1) {
		char tmp[200];
		snprintf(tmp, sizeof(tmp), "Failed to initialize socket (errno: %s)\n", strerror(errno));
		fatal(tmp);
		return (-1);
	}
	p->target = "";
	p->verbose_flag = 0;
	return (0);
}

int parse_ping(int ac, char **av, ping_t *p) {
	if (initialize_default_ping(p) == -1)
		return (-1);
	if (check_options(ac, av, p) != 0)
		p->target = av[2];
	else
		p->target = av[1];
	return (0);
}
