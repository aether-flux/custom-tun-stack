#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/tun.h"

int tun_alloc(const char *dev_name, char *actual_name) {
  int fd, rc;
  size_t iface_name_len;
  struct ifreq ifr;

  if (dev_name != NULL) {
    iface_name_len = strlen(dev_name);
    if (iface_name_len >= IFNAMSIZ) {
      errno = EINVAL;
      return -1;
    }
  }

  fd = open("/dev/net/tun", O_RDWR | O_CLOEXEC);
  if (fd < 0) {
    return -1;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (dev_name != NULL && dev_name[0] != '\0') {
    strncpy(ifr.ifr_name, dev_name, IFNAMSIZ - 1);
  }

  rc = ioctl(fd, TUNSETIFF, &ifr);
  if (rc < 0) {
    int ioctl_errno = errno;
    close(fd);
    errno = ioctl_errno;
    return -1;
  }

  if (actual_name != NULL) {
    strncpy(actual_name, ifr.ifr_name, IFNAMSIZ);
    actual_name[IFNAMSIZ - 1] = '\0';
  }

  return fd;
}

ssize_t tun_read(int tun_fd, void *buf, size_t len) {
  return read(tun_fd, buf, len);
}

ssize_t tun_write(int tun_fd, const void *buf, size_t len) {
  return write(tun_fd, buf, len);
}
