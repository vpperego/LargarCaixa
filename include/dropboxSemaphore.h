#ifndef __dropboxSemaphore__
#define __dropboxSemaphore__


#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif
#include <errno.h>

#include <stdint.h>

typedef struct dbsem {
#ifdef __APPLE__
  dispatch_semaphore_t sem;
#else
  sem_t sem;
#endif
} dbsem_t;

static inline void dbsem_init(struct dbsem *s ,uint32_t value) {
#ifdef __APPLE__
  dispatch_semaphore_t *sem = &s->sem;

  *sem = dispatch_semaphore_create(value);
#else
  sem_init(&s->sem,0, value);
#endif
}

static inline void dbsem_wait(struct dbsem *s) {

#ifdef __APPLE__
  dispatch_semaphore_wait(s->sem, DISPATCH_TIME_FOREVER);
#else
  int r;

  do {
    r = sem_wait(&s->sem);
  } while (r == -1 && errno == EINTR);
#endif
}

static inline void dbsem_post(struct dbsem *s) {

#ifdef __APPLE__
  dispatch_semaphore_signal(s->sem);
#else
  sem_post(&s->sem);
#endif
}

#endif
