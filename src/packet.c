#include <stddef.h>
#include <stdio.h>
#include <string.h>
// #define _POSIX_C_SOURCE 200809L

#include <stdint.h>

#include "../include/packet.h"

uint16_t get_checksum(const void *buf, size_t len) {
  const uint16_t *ptr = buf;
  uint32_t sum = 0;

  // 16-bit sums
  while (len > 1) {
    sum += *ptr++;
    len -= 2;
  }

  // Handle odd length (if left)
  if (len > 0)
    sum += *(const uint8_t *)ptr;

  // 32-bit sum -> 16-bit (add carry overs)
  while (sum >> 16)
    sum = (sum & 0xFFFF) + (sum >> 16);

  // Perform 1's complement
  return (uint16_t)(~sum);
}

// int handle_buffer(uint8_t *buf, size_t len) {
//   if (len < 20)
//     return -1; // minimum length
//
//   // Extract version and IHL
//   uint8_t version = (buf[0] >> 4) & 0x0F;
//   uint8_t ihl = buf[0] & 0x0F;
//   size_t ip_header_len = ihl * 4;
//
//   if (version != 4)
//     return -1; // not ipv4
//   if (len < ip_header_len)
//     return -1; // truncated header
//
//   uint8_t protocol = buf[9];
//
//   // Handle IPv4 layer
//   if (handle_iph(buf, ip_header_len) < 0)
//     return -1; // bad checksum or invalid header
//
//   // Handle protocol
//   if (protocol == 1) { // ICMP
//     uint8_t *icmp_header = buf + ip_header_len;
//     size_t icmp_len = len - ip_header_len;
//
//     if (handle_icmp(icmp_header, icmp_len) < 0)
//       return -1; // invalid ICMP packet
//   } else {
//     return -1; // unsupported protocol
//   }
//
//   return 0; // success
// }

int handle_buffer(uint8_t *buf, size_t len) {
  if (len < 20) {
    printf("[-] Drop: packet too short (%zu bytes)\n", len);
    return -1;
  }

  uint8_t version = (buf[0] >> 4) & 0x0F;
  uint8_t ihl = buf[0] & 0x0F;
  size_t ip_header_len = ihl * 4;

  if (version != 4) {
    printf("[-] Drop: not IPv4 (version=%u)\n", version);
    return -1;
  }
  if (len < ip_header_len) {
    printf("[-] Drop: truncated IP header\n");
    return -1;
  }

  uint8_t protocol = buf[9];

  // Handle IPv4 layer
  if (handle_iph(buf, ip_header_len) < 0) {
    printf("[-] Drop: IP header checksum failed!\n");
    return -1;
  }

  // Handle protocol
  if (protocol == 1) { // ICMP
    uint8_t *icmp_header = buf + ip_header_len;
    size_t icmp_len = len - ip_header_len;

    if (handle_icmp(icmp_header, icmp_len) < 0) {
      printf("[-] Drop: ICMP handler failed (checksum or type != 8)!\n");
      return -1;
    }
  } else {
    printf("[-] Drop: unsupported protocol (%u)\n", protocol);
    return -1;
  }

  return 0; // success
}

int handle_iph(uint8_t *buf, size_t ip_header_len) {
  // Checksum
  uint16_t checksum_rcvd = *(uint16_t *)(buf + 10);

  buf[10] = 0;
  buf[11] = 0;

  uint16_t checksum_calc = get_checksum(buf, ip_header_len);

  if (checksum_rcvd != checksum_calc)
    return -1; // invalid checksum

  // Swap source <-> destination addresses
  // 12-15 16-19
  uint32_t temp_ip;
  memcpy(&temp_ip, buf + 12, 4);
  memcpy(buf + 12, buf + 16, 4);
  memcpy(buf + 16, &temp_ip, 4);

  // Recompute checksum
  buf[10] = 0;
  buf[11] = 0;

  uint16_t new_checksum = get_checksum(buf, ip_header_len);
  *(uint16_t *)(buf + 10) = new_checksum;

  return 0; // success
}

int handle_icmp(uint8_t *buf, size_t icmp_len) {
  if (icmp_len < 8)
    return -1; // ICMP header too small

  // Verify type
  uint8_t type = buf[0];
  if (type != 8)
    return -1; // not an echo request

  // Checksum
  uint16_t checksum_rcvd = *(uint16_t *)(buf + 2);

  buf[2] = 0;
  buf[3] = 0;

  uint16_t checksum_calc = get_checksum(buf, icmp_len);

  if (checksum_rcvd != checksum_calc)
    return -1; // invalid checksum

  // Set type to 0 (echo reply)
  buf[0] = 0;

  // Recompute checksum
  buf[2] = 0;
  buf[3] = 0;

  uint16_t new_checksum = get_checksum(buf, icmp_len);
  *(uint16_t *)(buf + 2) = new_checksum;

  return 0; // success
}
