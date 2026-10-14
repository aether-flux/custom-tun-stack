#define _GNU_SOURCE

#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "../include/poller.h"
#include "../include/tun.h"

#define BUF_SIZE 2048

static struct poller *g_poller = NULL;

// Handle SIGINT signal
static void handle_sigint(int signal) {
  (void)signal;
  printf("\n[+] Caught SIGINT...\n");
  if (g_poller) {
    poller_stop(g_poller);
  }
}

// Prints hex data from buffer
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

// Callback to run when event is triggered
void cb(int fd, uint32_t events, void *user_data) {
  unsigned char buffer[BUF_SIZE];
  ssize_t nread = tun_read(fd, buffer, sizeof(buffer));
  if (nread < 0) {
    // if (errno == EINTR)
    //   continue;
    perror("Error reading from TUN device");
    return;
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

int main(void) {
  char if_name[16] = {0};

  // Register signal handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_sigint;
  sigaction(SIGINT, &sa, NULL);

  printf("Allocating TUN device\n");
  int tun_fd = tun_alloc("tun0", if_name);
  if (tun_fd < 0) {
    perror("Failed to allocate TUN device");
    fprintf(stderr,
            "    Note: Creating TUN devices requires root/sudo privileges.\n");
    return EXIT_FAILURE;
  }

  // Setting tun_fd as non-blocking to adhere with epoll
  int flags = fcntl(tun_fd, F_GETFL, 0);
  fcntl(tun_fd, F_SETFL, flags | O_NONBLOCK);

  printf("[+] Successfully allocated interface: %s (fd: %d)\n", if_name,
         tun_fd);
  printf("[+] Bring up the interface in another terminal:\n");
  printf("    sudo ip link set dev %s up\n", if_name);
  printf("    sudo ip addr add 10.0.0.1/24 dev %s\n\n", if_name);
  printf("[+] Listening for incoming packets on %s...\n\n", if_name);

  // Initialize poller and use epoll
  struct poller *p = poller_init();
  g_poller = p;

  if (poller_add(p, tun_fd, EPOLLIN, cb, NULL) < 0) {
    poller_destroy(p);
    close(tun_fd);
    return EXIT_FAILURE;
  }

  poller_run(p);

  printf("[+] Cleaning up resources...\n");
  close(tun_fd);
  poller_destroy(p);
  printf("[+] Exit.\n");
  return EXIT_SUCCESS;
}
