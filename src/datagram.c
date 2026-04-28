#include <_time.h>
#include <unistd.h>

#include "ft_ping.h"

uint16_t calculate_checksum(void *data, size_t len) {
	uint32_t sum = 0;
	uint16_t *to_work = (uint16_t *)data;

	for (size_t i = 0; i < (len / 2); i++) {
		sum += to_work[i];
	}
	if (len % 2 != 0) {
		sum += ((uint8_t *)data)[len - 1];
	}
	while (sum > 0xFFFF) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	return (uint16_t)~sum;
}

int create_icmp_datagram(icmp_t *datagram) {
	datagram->type = 0x08;
	datagram->code = 0x00;
	datagram->checksum = 0;
	datagram->identifier = htons(getpid() & 0xFFFF);
	datagram->sequence = htons(0);
	struct timespec *ts = (struct timespec *)datagram->payload;
	clock_gettime(CLOCK_MONOTONIC, ts);
	for (int i = 16; i < 56; i++)
		datagram->payload[i] = 0x10 + (i - 16);
	datagram->checksum = calculate_checksum(datagram, sizeof(*datagram));
	return 0;
}
