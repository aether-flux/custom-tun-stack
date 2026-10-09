#ifndef TUN_H
#define TUN_H

#include <stddef.h>
#include <sys/types.h>

/**
 * Allocates and binds a linux TUN interface.
 *
 * @param dev_name Requested interface name (eg, "tun0")
 *                 If dev_name[0] = '\0', the kernel auto-assigns a name.
 * @param actual_name Buffer of atleast 16 bytes to receive the assigned
 * interface name May be NULL if not needed.
 * @return File descriptor (>=0) on success, -1 on failure (errno is set)
 */
int tun_alloc(const char *dev_name, char *actual_name);

/**
 * Reads a raw IP packet from the TUN device.
 *
 * @param tun_fd File descriptor returned by tun_alloc
 * @param buf Buffer to store the incoming packet
 * @param len Capacity of the buffer
 * @return Number of bytes read, 0 on EOF, -1 on error
 */
ssize_t tun_read(int tun_fd, void *buf, size_t len);

/**
 * Writes a raw IP packet to the TUN device.
 *
 * @param tun_fd File descriptor returned by tun_alloc
 * @param buf Buffer containing the outgoing packet
 * @param len Length of the packet to write
 * @return Number of bytes written, or -1 on error
 */
ssize_t tun_write(int tun_fd, const void *buf, size_t len);

#endif // !TUN_H
