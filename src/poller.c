#include <errno.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "../include/poller.h"

#define MAX_EVENTS 64

struct poller {
  int epfd;
  int running;
};

struct poller *poller_init(void) {
  struct poller *p = malloc(sizeof(struct poller));
  if (!p)
    return NULL;

  p->epfd = epoll_create1(EPOLL_CLOEXEC);
  if (p->epfd < 0) {
    free(p);
    return NULL;
  }

  p->running = 1;
  return p;
}

int poller_add(struct poller *p, int fd, uint32_t events, void *user_data) {
  struct epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;
  ev.data.ptr = user_data;

  return epoll_ctl(p->epfd, EPOLL_CTL_ADD, fd, &ev);
}

int poller_run(struct poller *p) {
  struct epoll_event events[MAX_EVENTS];

  while (p->running) {
    int nfds = epoll_wait(p->epfd, events, MAX_EVENTS, -1);
    if (nfds < 0) {
      if (errno == EINTR)
        continue;
      return -1;
    }
  }

  return 0;
}

void poller_destroy(struct poller *p) {
  if (p) {
    if (p->epfd >= 0) {
      close(p->epfd);
    }
    free(p);
  }
}
