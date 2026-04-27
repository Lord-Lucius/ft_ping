#include <_time.h>
#include <time.h>
#include <unistd.h>

#include "ft_ping.h"

void calculate_checksum(icmp_t *datagram) {
	(void)datagram;
}

int create_icmp_datagram(icmp_t *datagram) {
	datagram->type = 8;
	datagram->type = 0;
	datagram->checksum = 0;
	datagram->identifier = getpid() & 0xFFFF;
	datagram->sequence = 0;
	struct timespec *ts = (struct timespec *)datagram->payload;
	clock_gettime(CLOCK_MONOTONIC, ts);
	for (int i = 16; i < 56; i++)
		datagram->payload[i] = 0x10 + (i - 16);
	calculate_checksum(datagram);
	return 0;
}
