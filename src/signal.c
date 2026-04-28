#include <signal.h>
#include <stdio.h>
#include <strings.h>

#include "ft_ping.h"

volatile sig_atomic_t g_sig = 0;

void handler(int signal) {
	if (signal == SIGINT)
		g_sig = 1;
}

void set_sigaction(void) {
	struct sigaction act;
	bzero(&act, sizeof(act));
	act.sa_handler = &handler;
	sigaction(SIGINT, &act, NULL);
}
