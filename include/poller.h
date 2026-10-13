#ifndef POLLER_H
#define POLLER_H

#include <stdint.h>

/* Incomplete struct */
struct poller;

/* Callback function signature for when an event triggers */
typedef void (*poller_callback_t)(int fd, uint32_t events, void *user_data);

/**
 * Initializes an epoll instance
 * Returns a pointer to poller struct, or NULL
 */
struct poller *poller_init(void);

/**
 * Adds a fd to poller instance
 */
int poller_add(struct poller *p, int fd, uint32_t events, void *user_data);

/**
 * Runs the main event loop
 */
int poller_run(struct poller *p);

/**
 * Removes and cleans up poller instance
 */
void poller_destroy(struct poller *p);

#endif // !POLLER_H
