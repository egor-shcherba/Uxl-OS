#ifndef _SCHED_H
#define _SCHED_H

#include <kernel/task.h>

void sched_init(void);

void sched_enqueue(struct task *task);
struct task* sched_dequeue(void);
struct task* sched_get_current(void);

void sched_enable(void);
void schedule(void);

#endif /* NOT _SCHED_H */
