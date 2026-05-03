#include <sys/errno.h>
#include <time.h>
#include <unistd.h>

#include "ft_ping.h"

int check_icmp_type(response_t *response) {
	icmp_t *reply = get_reply_icmp(response);

	if (reply->type != 0) {
		if (reply->type == 8)
			return ICMP_OUR_REQUEST;
		return ICMP_ANOMALY;
	}
	if (reply->identifier != htons(getpid() & 0xFFFF))
		return ICMP_ANOMALY;
	return ICMP_FOR_US;
}

icmp_t *get_reply_icmp(response_t *response) {
	uint8_t *buf = (uint8_t *)response->recv_buffer;
	size_t ip_hdr_len = (size_t)(buf[0] & 0x0F) * 4;
	return ((icmp_t *)(buf + ip_hdr_len));
}

int ping_send(ping_t *def, icmp_t *datagram, response_t *response) {
	response->src_len = sizeof(response->src_addr);
	if ((response->send_bytes =
			 sendto(def->socket_fd, datagram, sizeof(*datagram), 0,
					(const struct sockaddr *)&def->dest_addr,
					sizeof(def->dest_addr))) == -1) {
		fatal("sendto failed");
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

void create_icmp_datagram(icmp_t *datagram) {
	datagram->type = 0x08;
	datagram->code = 0x00;
	datagram->checksum = 0;
	datagram->identifier = htons(getpid() & 0xFFFF);
	datagram->sequence = 0;
	struct timespec *ts = (struct timespec *)datagram->payload;
	clock_gettime(CLOCK_MONOTONIC, ts);
	for (int i = 16; i < PAYLOAD_SIZE; i++)
		datagram->payload[i] = 0x10 + (i - 16);
	datagram->checksum = calculate_checksum(datagram, sizeof(*datagram));
}
