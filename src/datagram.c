#include <_time.h>
#include <unistd.h>

#include "ft_ping.h"

int ping_once(ping_t *def, icmp_t *datagram, response_t *response) {
	response->src_len = sizeof(response->src_addr);
	if ((response->send_bytes = sendto(def->socket_fd, datagram, sizeof(*datagram), 0, (const struct sockaddr *)&def->dest_addr, sizeof(def->dest_addr))) == -1) {
		fatal("sendto failed");
		return -1;
	}

	if ((response->recv_bytes = recvfrom(def->socket_fd, response->recv_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&response->src_addr, &response->src_len)) == -1) {
		fatal("recvfrom failed");
		return -1;
	}
	return 0;
}

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
