#ifndef _TASK_H
#define _TASK_H

#define TASK_NAME_LENGTH    64

#include <klib/list.h>
#include <sys/x86.h>

enum TASK_STATE {
  UNUSED,
  READY,
  RUNNING,
  SLEEP_INTR,
  SLEEP,
  ZOMBIE,
  EMBRYO
};

struct task_mm {
  uint32_t code_start;
  uint32_t code_end;
  uint32_t data_start;
  uint32_t data_end;
  uint32_t bss_start;
  uint32_t bss_end;
  uint32_t heap_end;
};

struct task {
  int pid;
  int ppid;
  int gigid;
  int uid;

  enum TASK_STATE state;
  char name[TASK_NAME_LENGTH];

  uint32_t kstack;
  uint32_t kesp;

  uint32_t ustack;
  uint32_t uesp;

  struct list node;
  struct task_mm mm;
  struct regs *regs;
};

void task_init(void);

void task_create_kworker(const char*, void (*)(void));
void task_create_init_proc(const char *, void (*)(void));

struct task* task_alloc();
void task_free(struct task *task);

void yield(void);
void sleep(void);
void wakeup(struct task *task);


#endif /* NOT _TASK_H */
