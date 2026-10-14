#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "../include/poller.h"

#define MAX_EVENTS 64

struct poller_ctx {
  poller_callback_t cb;
  void *user_data;
  int fd;
  struct poller_ctx *next;
};

struct poller {
  int epfd;
  int running;
  struct poller_ctx *ctx_list;
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
  p->ctx_list = NULL;
  return p;
}

int poller_add(struct poller *p, int fd, uint32_t events, poller_callback_t cb,
               void *user_data) {
  struct poller_ctx *ctx = malloc(sizeof(struct poller_ctx));

  ctx->cb = cb;
  ctx->user_data = user_data;
  ctx->fd = fd;

  ctx->next = p->ctx_list;
  p->ctx_list = ctx;

  struct epoll_event ev;
  ev.events = events;
  ev.data.ptr = ctx;

  if (epoll_ctl(p->epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
    p->ctx_list = ctx->next;
    free(ctx);
    return -1;
  }

  return 0;
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

    for (int i = 0; i < nfds; i++) {
      struct poller_ctx *ctx = events[i].data.ptr;
      if (ctx && ctx->cb) {
        ctx->cb(ctx->fd, events[i].events, ctx->user_data);
      }
    }
  }

  return 0;
}

void poller_stop(struct poller *p) {
  if (p)
    p->running = false;
}

void poller_destroy(struct poller *p) {
  if (!p)
    return;

  struct poller_ctx *cur = p->ctx_list;
  while (cur != NULL) {
    struct poller_ctx *next = cur->next;
    free(cur);
    cur = next;
  }

  if (p->epfd >= 0)
    close(p->epfd);

  free(p);
}
