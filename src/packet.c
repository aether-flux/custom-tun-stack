#include <stddef.h>
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>

#include "../include/packet.h"

int handle_buffer(uint8_t *buf, size_t len) {
  if (len < 20)
    return -1; // minimum length

  // Extract version and IHL
  uint8_t version = (buf[0] >> 4) & 0x0F;
  uint8_t ihl = buf[0] & 0x0F;
  size_t ip_header_len = ihl * 4;

  if (version != 4)
    return -1; // not ipv4
  if (len < ip_header_len)
    return -1; // truncated header

  uint8_t protocol = buf[9];

  // Handle IPv4 layer
  if (handle_iph(buf, ip_header_len) < 0)
    return -1; // bad checksum or invalid header

  // Handle protocol
  if (protocol == 1) { // ICMP
    uint8_t *icmp_header = buf + ip_header_len;
    size_t icmp_len = len - ip_header_len;

    if (handle_icmp(icmp_header, icmp_len) < 0)
      return -1; // invalid ICMP packet
  } else {
    return -1; // unsupported protocol
  }

  return 0; // success
}

int handle_iph(uint8_t *buf, size_t ip_header_len) { return 0; }

int handle_icmp(uint8_t *buf, size_t icmp_len) { return 0; }
