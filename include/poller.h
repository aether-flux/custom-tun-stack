#ifndef POLLER_H
#define POLLER_H

#include <stdint.h>

// Callback function signature for when an event triggers
typedef void (*poller_callback_t)(int fd, uint32_t events, void *user_data);

// Incomplete poller struct
struct poller;

/**
 * Initializes an epoll instance
 *
 * @return Pointer to initialized poller struct
 */
struct poller *poller_init(void);

/**
 * Adds a fd to poller instance
 *
 * @param p Pointer to poller struct
 * @param fd File descriptor to add to epoll list
 * @param events Events to watch the fd on
 * @param cb Callback and context for the fd
 * @param user_data Data to store at the pointer
 * @return Integer, 0 if success, -1 if error (errno is set)
 */
int poller_add(struct poller *p, int fd, uint32_t events, poller_callback_t cb,
               void *user_data);

/**
 * Runs the main event loop
 *
 * @param p Pointer to poller struct
 * @return Integer, 0 if success, -1 if error (errno is set)
 */
int poller_run(struct poller *p);

/**
 * Stops the poll loop, if running
 *
 * @param p Pointer to poller struct
 */
void poller_stop(struct poller *p);

/**
 * Removes and cleans up poller instance
 *
 * @param p Pointer to poller struct
 */
void poller_destroy(struct poller *p);

#endif // !POLLER_H
