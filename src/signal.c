#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/time.h>

#include "ft_ping.h"

volatile sig_atomic_t g_sig = 0;
volatile sig_atomic_t g_alarm = 0;

void start_time(void) {
	struct itimerval timer;

	timer.it_value.tv_sec = 1;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 1;
	timer.it_interval.tv_usec = 0;

	setitimer(ITIMER_REAL, &timer, NULL);
}

void handler(int signal) {
	if (signal == SIGINT) g_sig = 1;
	if (signal == SIGALRM) g_alarm = 1;
}

void set_sigaction(void) {
	if (DEBUG_FLAG) {
		debug("=== entered SET_SIGACTION function ===");
	}

	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = &handler;
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGALRM, &act, NULL);

	if (DEBUG_FLAG) {
		debug("=== exited SET_SIGACTION function ===");
	}
}
