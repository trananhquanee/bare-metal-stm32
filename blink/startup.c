#include <stdint.h>

extern uint32_t _stack_top;
void Reset_Handler(void);
//khai báo main
int main(void);
//Vector_table  bắt buộc phải nằm ở đầu flash
__attribute__ ((section(".isr_vector")))
uint32_t vector_table[] ={
    (uint32_t)&_stack_top,
    (uint32_t)&Reset_Handler,
};
void Reset_Handler(void){
    main();
    while(1);
}
