#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
		snprintf(tmp, sizeof(tmp),
				 "Failed to resolved destination address (gai_strerror: %s)\n",
				 gai_strerror(ret));
		fatal(tmp);
		return -1;
	}
	memcpy(&p->dest_addr, res->ai_addr, sizeof(struct sockaddr_in));
	if (inet_ntop(AF_INET, &(p->dest_addr.sin_addr), p->resolved_target,
				  INET_ADDRSTRLEN) == NULL) {
		char tmp[200];
		snprintf(
			tmp, sizeof(tmp),
			"Failed to transform network addr to printable addr (errno: %s)\n",
			strerror(errno));
		fatal(tmp);
		return -1;
	}
	freeaddrinfo(res);
	return 0;
}

static void check_options(int ac, char **av, ping_t *p) {
	int opt;

	while ((opt = getopt(ac, av, "v?")) != -1) {
		if (DEBUG_FLAG) {
			char debug_txt[200];
			snprintf(debug_txt, sizeof(debug_txt),
					 "MAIN: getopt option_character: %c(%d)", opt, opt);
			debug(debug_txt);
		}
		switch (opt) {
		case 'v':
			p->verbose_flag = 1;
			break;
		case '?':
			usage(av[0], "main :: case '?'");
			exit(0);
		default:
			break;
		}
	}
}

static int initialize_default_ping(ping_t *p) {
	p->socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (p->socket_fd == -1) {
		char tmp[200];
		snprintf(tmp, sizeof(tmp), "Failed to initialize socket (errno: %s)\n",
				 strerror(errno));
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
	check_options(ac, av, p);

	if (optind >= ac) {
		fatal("no tagert given");
		return -1;
	}
	p->target = av[optind];
	return (0);
}
