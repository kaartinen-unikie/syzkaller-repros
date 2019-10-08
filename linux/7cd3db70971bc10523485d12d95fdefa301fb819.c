// WARNING: syz-executor still has locks held!
// https://syzkaller.appspot.com/bug?id=7cd3db70971bc10523485d12d95fdefa301fb819
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

static long syz_open_procfs(long a0, long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void loop(void)
{
  int i, call, thread;
  for (call = 0; call < 2; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

#ifndef __NR_execveat
#define __NR_execveat 322
#endif

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000280,
           "\x2f\x65\x78\x65\x00\x00\x00\x00\x00\x04\x09\x00\x4b\xdd\xd9\xde"
           "\x91\xbe\x10\xee\xbf\x00\x0e\x0e\x28\x1a\xb4\x2f\xb8\x97\xc0\xd5"
           "\x54\xfa\x07\x42\x4a\xde\xe9\x01\xd2\xda\x75\xaf\x1f\x02\x00\xf5"
           "\xab\x26\xd7\xa0\x71\xfb\x35\x33\x1c\xe3\x9c\x5a\x98\x42\x37\xd0"
           "\x92\xef\x1c\x00\xb5\x17\x02\x6f\x8b\xd7\xf2\xb0\x43\x6a\x4c\x40"
           "\x96\x0a\xb3\xf6\xbc\x48\x28\x09\xf6\xbd\x82\xca\xa3\x47\x99\x19"
           "\x07\x00\x00\x00\x00\x00\x00\x00\x5e\x44\x57\x3e\xef\x5f\xd0\xf4"
           "\x23\xa5\xcf\xb3\x86\xe4\xcc\x99\x7c\x6e\xff\xde\x7e\x60\x3f\xde"
           "\xab\x44\x86\x71\xb6\x3b\xec\x6e\x93\x95\xaa\xba\xb4\xd0\x45\xf1"
           "\xad\x98\x2a\x2a\x89\x7f\xaf\xa7\x10\xbe\x9e\x68\x2e\x3c\x6a\x45"
           "\xdb\x03\xd9\xe6\xcb\x58\xfb\xec\x3d\x83\x97\x00\x5f\x17\xd6\xf7"
           "\xaf\xa1\x02\xde\xd1\x83\x7b\xcb\x80\x56\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\xe7\x0d\xa2\xfb\xe1\x59\x5f\x24\xf3\xf2\x02\xeb"
           "\xcb\x77\x74\xbb\x5d\xc9\xa4\xbd\xcc\x24\xef\x5f\xe2\xd2\x0b\x00"
           "\x12\x35\x2c\x85\x36\xe5\xf0\x7e\x7c\xcc\x9b\x97\x35\x70\x12\x3f"
           "\x8a\xbe\xf5\x11\x8b\x24\x88\xa8\x04\xb6\xfe\x27\xf8\x39\xd1\x7f"
           "\xa6\x37\x63\x6c\x6b\xa3\xb2\xb4\xfb\xfd\xd3\x48\xf2\x76\xc7\x15"
           "\x76\xfb\xf5\x14\xd5\x2f\x72\xb2\x9e\x83\xf0\x7d\xa1\x5e\xc5\xda"
           "\x20\xd8\x4d\x47\x90\x7c\xa1\x1c\x3c\x49\x44\x20\x7d\xdc\xd9\x0b"
           "\xcd\x23\x8a\xb5\x39\x1e\xbc\x7b\xee\x55\x62\x86\x47\x94\xfa\x50"
           "\x4c\xe1\xcc\x4e\xb2\x6f\xf2\x04\x0e\xf9\x9d\xd2\xd4\xd2\x9f\xd9"
           "\x6c\xe9\x82\x2c\x32\x3d\xb0\xcc\x40\x78\xf7\xd4\xde\x83\xe5\xbf"
           "\xc6\xb2\xf2\x54\x34\x07\xf1\xbc\x45\x31\xac\xf7",
           364);
    res = syz_open_procfs(0, 0x20000280);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy((void*)0x20000000, "", 1);
    syscall(__NR_execveat, r[0], 0x20000000, 0, 0, 0x1000);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}