#ifndef _PIC_I8259_H
#define _PIC_I8259_H

void pic_i8259_init(void);

void pic_i8259_set_handler(int irq_num, void (*handler)(void));

void pic_i8259_irq_enable(int irq_num);
void pic_i8259_irq_disable(int irq_num);

#endif /* NOT _PIC_I8259_H */