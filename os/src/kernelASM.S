        // file: kernelASM.S

        .syntax unified
        .thumb
        .file       "kernelASM.S"

        .text

        .align      2
        .global     SVC_Handler
        .thumb_func
        .type       SVC_Handler, %function
SVC_Handler:
        push    {lr}            // lr contains EXC_RETURN value

        // store context on thread stack
        // copy r0 from PSP as args to pendsv_handler
        mrs     r12, PSP
        stmdb   r12!, {r4-r11}
        msr     PSP, r12

        bl      SVC_Handler_C
        // get new context from thread stack
        // pendsv_handler needs to set new PSP
        mrs     r12, PSP
        ldmia   r12!, {r4-r11}
        msr     PSP, r12

        // exit
        pop     {lr}            
        bx      lr              // actual return address is taken from PSP stack

        .align      2
        .global     PendSV_Handler
        .thumb_func
        .type       PendSV_Handler, %function
PendSV_Handler:
        push    {lr}            // lr contains EXC_RETURN value

        // store context on thread stack
        mrs     r12, PSP
        stmdb   r12!, {r4-r11}
        msr     PSP, r12

        bl      PendSV_Handler_C
        // get new context from thread stack
        // pendsv_handler needs to set new PSP
        mrs     r12, PSP
        ldmia   r12!, {r4-r11}
        msr     PSP, r12

        // exit
        pop     {lr}            
        bx      lr              // actual return address is taken from PSP stack

        .end
