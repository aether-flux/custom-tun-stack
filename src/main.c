#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/tun.h"

#define BUF_SIZE 2048

static void hex_dump(const unsigned char *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02x ", buf[i]);
    if ((i + 1) % 16 == 0) {
      printf("\n");
    }
  }

  if (len % 16 != 0) {
    printf("\n");
  }
}

int main(void) {
  char if_name[16] = {0};
  unsigned char buffer[BUF_SIZE];

  printf("Allocating TUN device\n");
  int tun_fd = tun_alloc("tun0", if_name);
  if (tun_fd < 0) {
    perror("Failed to allocate TUN device");
    fprintf(stderr,
            "    Note: Creating TUN devices requires root/sudo privileges.\n");
    return EXIT_FAILURE;
  }

  printf("[+] Successfully allocated interface: %s (fd: %d)\n", if_name,
         tun_fd);
  printf("[+] Bring up the interface in another terminal:\n");
  printf("    sudo ip link set dev %s up\n", if_name);
  printf("    sudo ip addr add 10.0.0.1/24 dev %s\n\n", if_name);
  printf("[+] Listening for incoming packets on %s...\n\n", if_name);

  while (1) {
    ssize_t nread = tun_read(tun_fd, buffer, sizeof(buffer));
    if (nread < 0) {
      if (errno == EINTR)
        continue;
      perror("Error reading from TUN device");
      break;
    }

    printf("=== Received packet (%zd bytes) ===\n", nread);
    hex_dump(buffer, (size_t)nread);

    // Basic ipv4 check
    if (nread > 0) {
      uint8_t version = (buffer[0] >> 4) & 0x0F;
      uint8_t ihl = buffer[0] & 0x0F;
      printf("IP Version: %u, Header Length: %u bytes\n\n", version, ihl * 4);
    }
  }

  close(tun_fd);
  return EXIT_SUCCESS;
}
