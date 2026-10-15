#ifndef PACKET_H
#define PACKET_H

#include <stddef.h>
#include <stdint.h>

/**
 * Calculate checksum of given header
 *
 * @param buf Buffer of header bytes
 * @param len Size of buffer
 * @return Checksum computed from buffer
 * match
 */
uint16_t get_checksum(const void *buf, size_t len);

/**
 * Handle different parts of the received buffer
 *
 * @param buf Buffer with received message
 * @param len Number of bytes in buffer
 * @return Status, -1 if error, 0 otherwise
 */
int handle_buffer(uint8_t *buf, size_t len);

/**
 * Handle internet header bytes
 *
 * @param buf Buffer with internet header
 * @param ip_header_len Size of internet header in bytes
 * @return Status, -1 if error, 0 otherwise
 */
int handle_iph(uint8_t *buf, size_t ip_header_len);

/**
 * Handle ICMP header bytes
 *
 * @param buf Buffer with ICMP header
 * @param icmp_len Size of ICMP header in bytes
 * @return Status, -1 if error, 0 otherwise
 */
int handle_icmp(uint8_t *buf, size_t icmp_len);

#endif // !PACKET_H
