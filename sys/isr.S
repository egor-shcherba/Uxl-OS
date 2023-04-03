.code32

.extern exception_handler

.macro SAVE_REGS
  push %eax
  push %ebx
  push %ecx
  push %edx
  push %edi
  push %esi
  push %ebp

  push %ds
  push %es
  push %gs
  push %fs

  mov $0x10, %eax
  mov %eax, %ds
  mov %eax, %es
  mov %eax, %gs
  mov %eax, %fs

  mov %cr3, %eax
  push %eax
.endm

.macro RESTORE_REGS
  pop %fs
  pop %gs
  pop %es
  pop %ds

  pop %ebp
  pop %esi
  pop %edi
  pop %edx
  pop %ecx
  pop %ebx
  pop %eax
.endm

.macro exc_handler_with_error handler, number
.globl \handler
\handler:
  push $\number
  jmp except_enter
.endm

.macro exc_handler_without_error handler, number
.globl \handler
\handler:
  push $0
  push $\number
  jmp except_enter
.endm

.macro define_irq number
.globl irq_\number
irq_\number:
  push $0
  push $\number
  jmp irq_common
.endm

.globl reschedule
reschedule:
  push %eax
  pushf
  push %cs
  mov $done, %eax
  push %eax

  push $0x0
  push $0x0

  SAVE_REGS
  push %esp
  call task_save_context
  call schedule
  jmp intr_leave

done:
  pop %eax

  ret

except_enter:
  SAVE_REGS
  push %esp
  call task_save_context
  call exception_handler
  jmp intr_leave

irq_common:
  SAVE_REGS
  push %esp
  call task_save_context
  call pic_i8259_main_handler
  jmp intr_leave

.globl syscall_enter
syscall_enter:
  push $0
  push %eax
  SAVE_REGS
  push %esp
  call task_save_context
  call syscall_main
  jmp intr_leave

.globl intr_leave
intr_leave:
  call task_restore_context
  mov %eax, %esp

  pop %ebx
  mov %cr3, %eax
  cmp %eax, %ebx
  jz dont_switch_cr3
  mov %eax, %cr3

dont_switch_cr3:
  RESTORE_REGS
  addl $0x8, %esp
  iret

exc_handler_without_error exc_divide_error,   0x0
exc_handler_without_error exc_debug,          0x1
exc_handler_without_error exc_nmi,            0x2
exc_handler_without_error exc_breakpoint,     0x3
exc_handler_without_error exc_overflow,       0x4
exc_handler_without_error exc_boundrange,     0x5
exc_handler_without_error exc_inv_opcode,     0x6
exc_handler_without_error exc_no_dev,         0x7
exc_handler_with_error exc_double_fault,      0x8

exc_handler_with_error exc_inv_tss,           0xA
exc_handler_with_error exc_seg_not_present,   0xB
exc_handler_with_error exc_ss,                0xC
exc_handler_with_error exc_general_prot,      0xD
exc_handler_with_error exc_page_fault,        0xE

exc_handler_without_error exc_x87,            0x10
exc_handler_with_error exc_aligned_check,     0x11
exc_handler_without_error exc_machine_check,  0x12
exc_handler_without_error exc_simd,           0x13

define_irq 0
define_irq 1
define_irq 2
define_irq 3
define_irq 4
define_irq 5
define_irq 6
define_irq 7
define_irq 8
define_irq 9
define_irq 10
define_irq 11
define_irq 12
define_irq 13
define_irq 14
define_irq 15
