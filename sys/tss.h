#ifndef _TSS_H
#define _TSS_H

struct tss {
  uint16_t prev_task;
  uint16_t __prev_tast;
  uint32_t esp0;
  uint16_t ss0;
  uint16_t __ss0;
  uint32_t esp1;
  uint16_t ss1;
  uint16_t __ss1;
  uint32_t esp2;
  uint16_t ss2;
  uint16_t __ss2;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint16_t es;
  uint16_t __es;
  uint16_t cs;
  uint16_t __cs;
  uint16_t ss;
  uint16_t __ss;
  uint16_t ds;
  uint16_t __ds;
  uint16_t fs;
  uint16_t __fs;
  uint16_t gs;
  uint16_t __gs;
  uint16_t sel_seg;
  uint16_t _sel_seg;
  uint16_t _iomap;
  uint16_t iomap;
};

void tss_set_stack(void *kstack);  /* definition in sys/gdt.c */
void* tss_get_stack(void);         /* definition in sys/gdt.c */

#endif /* _TSS_H */