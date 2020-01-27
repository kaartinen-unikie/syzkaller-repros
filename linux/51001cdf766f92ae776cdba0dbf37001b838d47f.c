// WARNING in cbq_destroy
// https://syzkaller.appspot.com/bug?id=51001cdf766f92ae776cdba0dbf37001b838d47f
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
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
  int i;
  for (i = 0; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
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
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
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

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
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

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 21; call++) {
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

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[11] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                  0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    *(uint16_t*)0x20000080 = 0x11;
    memcpy((void*)0x20000082,
           "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a"
           "\x49\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01"
           "\x2e\x0b\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3"
           "\xff\x5f\x16\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00"
           "\x00\x01\x01\x01\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66"
           "\xfd\x79\x2b\xbf\x0e\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00"
           "\x01\x00\x00\x00\x00\x00\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00"
           "\x00\x06\xad\x8e\x5e\xcc\x32\x6d\x3a\x09\xff\xc2\xc6\x54",
           126);
    syscall(__NR_bind, r[1], 0x20000080ul, 0x80ul);
    break;
  case 3:
    *(uint32_t*)0x20000140 = 0x14;
    res = syscall(__NR_getsockname, r[1], 0x20000040ul, 0x20000140ul);
    if (res != -1)
      r[2] = *(uint32_t*)0x20000044;
    break;
  case 4:
    *(uint64_t*)0x200001c0 = 0;
    *(uint32_t*)0x200001c8 = 0;
    *(uint64_t*)0x200001d0 = 0x20000180;
    *(uint64_t*)0x20000180 = 0x20000200;
    *(uint32_t*)0x20000200 = 0x3c;
    *(uint16_t*)0x20000204 = 0x2c;
    *(uint16_t*)0x20000206 = 0xd27;
    *(uint32_t*)0x20000208 = 0;
    *(uint32_t*)0x2000020c = 0;
    *(uint8_t*)0x20000210 = 0;
    *(uint8_t*)0x20000211 = 0;
    *(uint16_t*)0x20000212 = 0;
    *(uint32_t*)0x20000214 = r[2];
    *(uint16_t*)0x20000218 = 0;
    *(uint16_t*)0x2000021a = 0;
    *(uint16_t*)0x2000021c = 0;
    *(uint16_t*)0x2000021e = -1;
    *(uint16_t*)0x20000220 = 2;
    *(uint16_t*)0x20000222 = 0;
    *(uint16_t*)0x20000224 = 0xa;
    *(uint16_t*)0x20000226 = 1;
    memcpy((void*)0x20000228, "basic\000", 6);
    *(uint16_t*)0x20000230 = 0xc;
    *(uint16_t*)0x20000232 = 2;
    *(uint16_t*)0x20000234 = 8;
    *(uint16_t*)0x20000236 = 1;
    *(uint16_t*)0x20000238 = 0;
    *(uint16_t*)0x2000023a = -1;
    *(uint64_t*)0x20000188 = 0x3c;
    *(uint64_t*)0x200001d8 = 1;
    *(uint64_t*)0x200001e0 = 0;
    *(uint64_t*)0x200001e8 = 0;
    *(uint32_t*)0x200001f0 = 0;
    syscall(__NR_sendmsg, r[0], 0x200001c0ul, 0ul);
    break;
  case 5:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
    if (res != -1)
      r[3] = res;
    break;
  case 6:
    res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
    if (res != -1)
      r[4] = res;
    break;
  case 7:
    *(uint16_t*)0x20000180 = 0x11;
    memcpy((void*)0x20000182,
           "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x2e"
           "\x2a\xba\x00\x00\x00\x01\x2e\x0b\x38\x36\x00\x54\x04\xb0\xe0\x30"
           "\x1a\x08\x00\x00\x00\xe3\xff\x5f\x16\x3e\xe3\x40\xb7\x67\x95\x00"
           "\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01\x3c\x58\x11\x03\x9e\x15"
           "\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e\x5b\xf5\xff\x1b\x08"
           "\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00\x00\x00\x49\x74"
           "\x00\x00\x00\x00\x00\x00\x00\x06\x5e\xcc\x32\x6d\x3a\x09\xff\xc2"
           "\xc6\x54\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           126);
    syscall(__NR_bind, r[4], 0x20000180ul, 0x80ul);
    break;
  case 8:
    *(uint32_t*)0x20000140 = 0x14;
    res = syscall(__NR_getsockname, r[4], 0x20000000ul, 0x20000140ul);
    if (res != -1)
      r[5] = *(uint32_t*)0x20000004;
    break;
  case 9:
    *(uint64_t*)0x20000200 = 0;
    *(uint32_t*)0x20000208 = 0;
    *(uint64_t*)0x20000210 = 0x20000080;
    *(uint64_t*)0x20000080 = 0x20000040;
    *(uint32_t*)0x20000040 = 0x24;
    *(uint16_t*)0x20000044 = 0x29;
    *(uint16_t*)0x20000046 = 1;
    *(uint32_t*)0x20000048 = 0;
    *(uint32_t*)0x2000004c = 0;
    *(uint8_t*)0x20000050 = 0;
    *(uint8_t*)0x20000051 = 0;
    *(uint16_t*)0x20000052 = 0;
    *(uint32_t*)0x20000054 = r[5];
    *(uint16_t*)0x20000058 = 0;
    *(uint16_t*)0x2000005a = -1;
    *(uint16_t*)0x2000005c = 0;
    *(uint16_t*)0x2000005e = -1;
    *(uint16_t*)0x20000060 = 0;
    *(uint16_t*)0x20000062 = 0;
    *(uint64_t*)0x20000088 = 0x24;
    *(uint64_t*)0x20000218 = 1;
    *(uint64_t*)0x20000220 = 0;
    *(uint64_t*)0x20000228 = 0;
    *(uint32_t*)0x20000230 = 0;
    syscall(__NR_sendmsg, r[3], 0x20000200ul, 0ul);
    break;
  case 10:
    res = syscall(__NR_pipe, 0x20000200ul);
    if (res != -1) {
      r[6] = *(uint32_t*)0x20000200;
      r[7] = *(uint32_t*)0x20000204;
    }
    break;
  case 11:
    res = syscall(__NR_socket, 2ul, 2ul, 0ul);
    if (res != -1)
      r[8] = res;
    break;
  case 12:
    *(uint8_t*)0x20000000 = -1;
    *(uint8_t*)0x20000001 = 2;
    *(uint8_t*)0x20000002 = 0;
    *(uint8_t*)0x20000003 = 0;
    *(uint8_t*)0x20000004 = 0;
    *(uint8_t*)0x20000005 = 0;
    *(uint8_t*)0x20000006 = 0;
    *(uint8_t*)0x20000007 = 0;
    *(uint8_t*)0x20000008 = 0;
    *(uint8_t*)0x20000009 = 0;
    *(uint8_t*)0x2000000a = 0;
    *(uint8_t*)0x2000000b = 0;
    *(uint8_t*)0x2000000c = 0;
    *(uint8_t*)0x2000000d = 0;
    *(uint8_t*)0x2000000e = 0;
    *(uint8_t*)0x2000000f = 1;
    *(uint32_t*)0x20000010 = 0;
    syscall(__NR_setsockopt, -1, 0x29ul, 0ul, 0x20000000ul, 0x14ul);
    break;
  case 13:
    syscall(__NR_close, r[8]);
    break;
  case 14:
    syscall(__NR_socket, 0x10ul, 2ul, 0);
    break;
  case 15:
    res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
    if (res != -1)
      r[9] = res;
    break;
  case 16:
    *(uint16_t*)0x20000080 = 0x11;
    memcpy((void*)0x20000082,
           "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x25\x94\x4e\xeb\xa7\x1a"
           "\x49\x76\xe2\x52\x92\x2c\x00\x00\x00\x00\x2a\xba\x00\x00\x00\x01"
           "\x2e\x0b\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe0"
           "\xff\x5f\x16\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00"
           "\x00\x01\x01\x01\x3c\x58\x91\x03\x9e\x15\x77\x50\x27\xec\xce\x66"
           "\xfd\x79\x2b\xbf\x0e\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00"
           "\x01\x00\x00\x00\x00\x00\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00"
           "\x00\x06\xad\x8e\x5e\xcc\x32\x6d\x3a\x09\xff\xc2\xc6\x54",
           126);
    syscall(__NR_bind, r[9], 0x20000080ul, 0x80ul);
    break;
  case 17:
    *(uint32_t*)0x20000140 = 0x14;
    res = syscall(__NR_getsockname, r[9], 0x20000100ul, 0x20000140ul);
    if (res != -1)
      r[10] = *(uint32_t*)0x20000104;
    break;
  case 18:
    *(uint64_t*)0x20000280 = 0;
    *(uint32_t*)0x20000288 = 0;
    *(uint64_t*)0x20000290 = 0x200002c0;
    *(uint64_t*)0x200002c0 = 0x20000300;
    memcpy((void*)0x20000300, "\xd0\x08\x00\x00\x24\x00\xff\xff\xff\x7f\x00\x00"
                              "\x00\x00\xff\xff\xa6\xff\xff\xf7",
           20);
    *(uint32_t*)0x20000314 = r[10];
    memcpy(
        (void*)0x20000318,
        "\x00\x00\x00\x0b\xf1\xff\xff\xff\x00\x00\x00\x00\x08\x00\x01\x00\x63"
        "\x62\x71\x00\xa4\x08\x02\x00\x04\x04\x06\x00\x03\x00\x00\x00\x05\x00"
        "\x00\x00\x00\x70\x03\x00\xfd\xff\xff\xff\xff\xff\x00\x00\x1f\x00\x00"
        "\x00\x05\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x10\x00\x83\x00\x00"
        "\x00\xfd\xff\xff\xff\x07\x00\x00\x00\x08\x49\x00\x00\x01\x00\x00\x00"
        "\x02\x00\x00\x00\xcf\x00\x00\x00\xfe\xff\xff\xff\x08\x00\x00\x33\x26"
        "\xf6\x00\x01\x80\x00\x00\x03\x00\x00\x00\x09\x00\x00\x00\x02\x00\x00"
        "\x00\x00\x27\x4a\x49\xef\x69\x49\xa7\xbb\x00\x00\x00\x05\x00\x00\x00"
        "\x01\x00\x00\x00\x04\x00\x00\x00\xff\x7f\x00\x00\x00\x00\x00\x00\x00"
        "\x04\x00\x00\x00\x02\x00\x00\x04\x00\x00\x00\xff\x0f\x00\x00\xf4\x4f"
        "\x35\x00\x01\x00\x00\x00\x01\x04\x00\x00\x5e\xb7\x68\x35\x01\x00\x00"
        "\x00\x08\x00\x00\x00\xff\xff\xff\xff\xc3\x8d\x00\x00\x1f\x00\x00\x00"
        "\x04\x00\x08\x83\x06\x00\x00\x00\x00\x00\x01\x00\xf7\xff\xff\xff\x03"
        "\x00\x00\x00\x00\x80\xff\xff\x05\x00\x00\x00\x06\x00\x00\x00\x06\x00"
        "\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\xc0\x00\x00\x00\xc0\xff\xff"
        "\xff\x08\x00\x00\x00\x01\x80\x00\x00\x01\x01\x00\x00\x06\x00\x00\x00"
        "\xb3\x00\x00\x00\x00\x00\x00\x20\x05\x00\x00\x00\x01\x04\x00\x00\x04"
        "\x00\x00\x00\x07\x00\x00\x00\x08\x00\x00\x00\x01\x01\x00\x00\xc2\x2e"
        "\x00\x00\xc0\xff\xff\xff\x05\x00\x00\x00\x04\x00\x00\x00\x06\x00\x00"
        "\x00\x09\x00\x00\x00\x08\x00\x00\x00\xf7\xff\xff\xff\x08\x00\x00\x00"
        "\x03\x00\x00\x00\x08\x00\x00\x00\x04\x00\x00\x00\x01\x03\x00\x80\x04"
        "\x00\x00\x00\x04\x00\x00\x00\x00\x10\x00\x00\x04\x00\x00\x00\x07\x00"
        "\x00\x00\x09\x00\x00\x00\xfc\xff\xff\xff\x01\x00\x00\x00\x01\x00\x00"
        "\x00\x09\x00\x00\x00\x00\x02\x00\x00\x06\x0e\x00\x00\x08\x00\x00\x00"
        "\x02\x00\x00\x00\x02\x00\x00\x00\x09\x00\x00\x00\x08\x00\x00\x00\x07"
        "\x00\x00\x00\xab\x00\x00\x00\xfb\x00\x00\x00\x07\x00\x00\x00\xff\x01"
        "\x00\x00\x08\x00\x00\x00\x01\x04\x00\x00\x07\x00\x00\x00\x03\x00\x00"
        "\x00\x09\x00\x00\x00\xff\x07\x00\x00\x08\x00\x00\x00\xff\xff\x00\x00"
        "\xa9\x01\x00\x00\x09\x00\x00\x00\x05\x00\x00\x00\x20\x00\x00\x00\x05"
        "\x00\x00\x00\x01\x00\x00\x00\x07\x00\x00\x00\x8e\xb7\x00\x00\x00\x00"
        "\x00\x80\x01\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\xff\xff\xff"
        "\xff\xfc\xff\xff\xff\x01\xf0\xff\xff\x00\x00\x00\x00\x3f\x00\x00\x00"
        "\x03\x00\x00\x00\x00\x01\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x03"
        "\x00\x00\x00\x05\x00\x00\x00\x05\x00\x00\x00\xff\xff\xff\xff\x07\x00"
        "\x00\x00\x15\xba\x00\x00\x04\x00\x00\x00\x07\x00\x00\x00\x03\x00\x00"
        "\x00\x03\x00\x00\x00\xff\xff\xff\xff\x04\x00\x00\x00\x55\x4f\x00\x00"
        "\x6f\xf8\x00\x00\x07\x00\x00\x00\x03\x00\x00\x00\x00\x02\x00\x00\x06"
        "\x00\x00\x00\x08\x00\x00\x00\xb3\x00\x00\x00\x06\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x07\x00\x00\x01\x01\x00\x00\x03\x00\x00\x00\x00\x80\x00"
        "\x00\x00\x04\x00\x00\x73\x0a\x00\x00\x01\x00\x00\x80\x01\x00\x00\x00"
        "\x9e\x0c\x00\x00\x00\x08\x00\x00\x01\x00\x00\x80\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x40\x00\x00\x00\x00\xfe\xff\xff\xf6\x7d\x00\x00\x06\x00"
        "\x00\x00\xff\x01\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00"
        "\x00\x40\x00\x00\x00\xed\x00\x00\x00\x01\x00\x01\x00\x0e\x0a\x05\x00"
        "\x00\x00\x00\x00\x00\x02\x00\x00\x04\x00\x00\x00\x06\x00\x00\x00\x02"
        "\x00\x00\x00\xff\x7f\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06\x00"
        "\x00\x00\x7f\x00\x00\x00\x02\x00\x00\x00\x00\x04\x00\x00\x04\x00\x00"
        "\x00\x09\x00\x00\x00\x90\x00\x00\x00\x08\x00\x00\x00\xb6\x7c\x00\x00"
        "\x04\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00\xf5\x0a\x00\x00\xff"
        "\xff\x00\x00\x06\x00\x00\x00\x8f\xf9\xff\xff\x07\x00\x00\x00\x1e\xaa"
        "\xff\xff\x3f\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\xff\x0c\x00"
        "\x00\xff\x7f\x00\x00\x03\x00\x00\x00\x7f\x00\x00\x00\x2d\x00\x00\x00"
        "\x08\x00\x00\x00\x00\x01\x00\x00\xff\x07\x00\x00\xff\xff\xff\xff\x2b"
        "\xab\x00\x00\x02\x00\x00\x00\xac\xe8\x00\x00\x05\x00\x00\x00\x00\x00"
        "\x00\x00\xe1\xbf\x93\x4d\x07\x00\x00\x00\x1f\x00\x0d\x00\x03\x00\x00"
        "\x00\x04\x00\x00\x00\x07\x00\x00\x00\x09\x00\x00\x00\x05\x00\x00\x00"
        "\x03\x00\x00\x00\x02\x00\x00\x00\x87\xd4\x00\x00\x02\x00\x00\x00\x08"
        "\x00\x00\x00\x00\x01\x00\x00\x7f\x00\x00\x00\x0e\x00\x00\x00\x08\x00"
        "\x00\x00\x02\x00\x00\x00\x07\x00\x00\x00\xff\xff\xff\x7f\x06\x00\x00"
        "\x00\x00\x00\x00\x00\x05\x00\x00\x00\x01\x00\x00\x00\x82\x00\x00\x00"
        "\x1f\x00\x00\x00\x00\x04\x00\x00\x01\x00\x00\x00\xfa\xff\xff\xff\x05"
        "\x00\x00\x00\x06\x00\x00\x00\x07\x00\x00\x00\x03\x00\x00\x00\x10\x00"
        "\x05\x00\x81\x03\x06\x00\x9f\xff\x04\x00\x08\x00\x00\x00\x04\x04\x06"
        "\x00\xd9\x0b\x00\x00\xff\x00\x00\x00\x05\x00\x00\x00\x04\x00\x00\x00"
        "\x1b\xa2\xcc\x3b\x00\x00\x00\x80\x01\x00\x00\x00\x3f\x00\x00\x00\xff"
        "\x0f\x00\x20\x11\x52\x49\x73\x03\x00\x80\x00\xbe\x06\x00\x00\xff\xff"
        "\xff\xff\xff\xfe\xff\xff\x7f\x00\x00\x00\xff\x00\x00\x00\x81\x00\x00"
        "\x00\x00\xdb\x04\x00\x09\x00\x00\x00\x07\x00\x00\x00\x03\x00\x00\x00"
        "\x00\x00\x00\x00\x06\x00\x00\x00\x8f\x5e\x00\x00\x01\x00\x00\x00\x00"
        "\x02\x00\x00\x06\x00\x00\x00\x01\x01\x00\x00\x02\x00\x00\x00\x07\x00"
        "\x00\x00\x00\x00\x00\xe0\x31\x89\x00\x00\x01\x04\x00\x00\x00\x08\x00"
        "\x00\x06\x00\x00\x00\x04\x00\x00\x00\x08\x00\x00\x00\x04\x00\x00\x00"
        "\x02\x00\x00\x00\xc0\x00\x00\x00\x08\x00\x00\x00\x08\x00\x00\x00\xf7"
        "\xff\xff\xff\x05\x00\x00\x00\x81\x00\x00\x00\x4d\x00\x00\x00\x01\x00"
        "\x00\x80\xeb\x07\x00\x00\x1f\x00\x00\x00\x09\x00\x00\x00\x05\x97\x00"
        "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00"
        "\xff\xff\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\xe0\xa0"
        "\x00\x00\x00\x40\x00\x00\x00\xff\x00\x00\x00\xf7\xff\xff\xff\x05\x00"
        "\x00\x00\x09\x00\x00\x00\x07\x00\x00\x00\x06\x00\x00\x00\x28\x08\x00"
        "\x00\x03\x00\x00\x00\x06\x00\x00\x00\x04\x00\x00\x00\xbe\x00\x00\x00"
        "\x03\x00\x00\x00\x07\x00\x00\x00\x05\x00\x00\x00\x38\x08\x00\x00\xfa"
        "\xff\xff\xff\xff\xff\xff\xff\x47\xb5\x55\x4d\x00\x10\x00\x00\x81\x00"
        "\x00\x00\x07\x00\x00\x00\xff\xff\xff\xff\x04\x00\x00\x00\x01\x00\x00"
        "\x00\x02\x00\x00\x00\x56\x6b\x00\x00\x20\x00\x00\x00\x04\x00\x00\x00"
        "\x02\x00\x00\x00\x8b\x01\x00\x00\x01\x00\x00\x00\xff\xff\x00\x00\x04"
        "\x00\x00\x00\xff\x0f\x00\x00\x05\x00\x00\x00\x20\x00\x00\x00\x03\x00"
        "\x00\x00\xe4\x0d\x00\x00\x07\x00\x00\x00\xf8\xff\xff\xff\x20\x00\x00"
        "\x00\x3f\x00\x00\x00\x38\x7f\x00\x00\x07\x00\x00\x00\x07\x00\x00\x00"
        "\xc1\x0a\x00\x00\x09\x00\x00\x00\x04\x00\x00\x00\x04\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x80\x00\x00\x01\x00\x00\x80\x09\x00\x00\x00\x37\x0b"
        "\x00\x00\x00\x00\x00\x80\xff\x07\x00\x00\x06\x00\x00\x00\xbb\x79\x00"
        "\x00\x04\x00\x00\x00\x02\x00\x00\x00\xff\xff\xff\x7f\x03\x00\x00\x00"
        "\x00\x00\x00\x00\x09\x00\x00\x00\x07\x00\x00\x00\x05\x00\x00\x00\x00"
        "\x00\x00\x80\x7f\x00\x00\x00\x09\x00\x00\x00\x08\x00\x00\x00\xfa\xd4"
        "\xff\xff\xff\x01\x00\x00\x01\x01\x00\x00\x01\x00\x00\x00\x3f\x00\x00"
        "\x00\x04\x00\x00\x00\x20\xf2\xff\xff\x01\x00\x01\x00\xff\xff\x00\x00"
        "\x01\x00\x00\x00\x02\x00\x00\x00\x08\x00\x00\x00\xfb\xff\xff\xff\xf9"
        "\xff\xff\xff\x04\x00\x00\x00\x4d\x00\x00\x00\x06\x00\x00\x00\x02\x00"
        "\x00\x00\x02\x00\x00\x00\x07\x00\x00\x00\x4e\x0d\x00\x00\x00\x02\x00"
        "\x00\x08\x00\x00\x00\x35\x00\x00\x00\x30\x0e\x00\x00\x07\x00\x00\x00"
        "\x00\x04\x00\x00\x02\x00\x00\x00\x01\x80\x00\x00\x20\x00\x00\x00\xf2"
        "\xff\xff\xff\x07\x00\x00\x00\x05\x00\x00\x00\x00\x01\x00\x00\xff\xff"
        "\xfe\xff\x09\x00\x00\x00\x00\x08\x00\x00\x07\x00\x00\x00\x08\x00\x00"
        "\x00\x09\x00\x00\x00\xff\x7f\x00\x00\x09\x00\x00\x00\x04\x00\x00\x00"
        "\x04\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x05\x00\x00\x00\x01"
        "\x00\x00\x00\x09\x00\x00\x00\x08\x00\x00\x00\x7e\x07\x00\x00\x03\x00"
        "\x00\x00\x00\x80\x00\x00\xbf\x16\x00\x00\x02\x00\x00\x06\x00\x00\x00"
        "\xff\xee\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7f\x00\x00\x00\x10"
        "\x08\x00\x00\x00\x00\x00\x00\x01\x00\x00\x80\x02\x00\x00\x00\x08\x00"
        "\x00\x00\x01\x01\x00\x00\x03\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff"
        "\xff\xf7\xff\xff\xff\x06\x00\x00\x00\x03\x00\x00\x00\xa6\x06\x00\x00"
        "\xff\x03\x00\x00\x07\x00\x00\x00\x01\x80\x00\x00\x09\x00\x00\x00\x02"
        "\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x80\xff\xff\x00\x02"
        "\x00\x00\x07\x00\x00\x00\x05\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00"
        "\x80\x00\x00\xaf\x06\x06\x00\x00\x00\x2b\x0a\x00\x00\x05\x00\x00\x00"
        "\x01\x00\x00\x00\x05\x00\x00\x00\x09\x00\x00\x00\x04\x00\x00\x00\x05"
        "\x00\x00\x00\x02\x00\x00\x00\xff\xff\xff\x7f\xf3\x00\x00\x00\x08\x00"
        "\x00\x00\x02\x00\x00\x00\xff\xff\x00\x00\xff\x07\x00\x00\x81\x00\x00"
        "\x00\x40\x00\x00\x00\x75\xfc\x00\x00\x09\x00\x00\x00\xfb\xff\xff\xff"
        "\x05\x00\x00\x00\x00\x00\x00\x00\x81\x00\x00\x00\x01\x00\x00\x00\x80"
        "\x00\x00\x00\xd0\x5e\x00\x00\x02\x00\x00\x00\x1f\x00\x00\x00\x00\x00"
        "\x00\x00\xff\x01\x00\x00\x45\x1e\x00\x00\x2d\x00\x00\x00\x00\x00\x00"
        "\x00\xff\x00\x00\x00\x10\x00\x03\x00\x08\x00\x10\x00\xff\x07\x00\x00"
        "\x01\x00\x00\x00\x18\x00\x01\x00\x39",
        2117);
    *(uint64_t*)0x200002c8 = 3;
    *(uint64_t*)0x20000298 = 1;
    *(uint64_t*)0x200002a0 = 0;
    *(uint64_t*)0x200002a8 = 0;
    *(uint32_t*)0x200002b0 = 0;
    syscall(__NR_sendmsg, -1, 0x20000280ul, 0ul);
    break;
  case 19:
    syscall(__NR_write, r[7], 0x20000000ul, 0xfffffeccul);
    break;
  case 20:
    syscall(__NR_splice, r[6], 0ul, r[8], 0ul, 0x4ffe0ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  loop();
  return 0;
}
