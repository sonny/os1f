        // file: kernel.S

        .syntax unified
        .thumb
        .file       "kernel.S"

        .set SCB_ICSR,0xE000ED04 
        .set PENDSVSET, (0x1<<28)
        
        .text

/*
        .align      2
        .global     SVC_Handler
        .thumb_func
        .type       SVC_Handler, %function
SVC_Handler:
        // enable PendSV
        // load SCB.ICSR
        ldr     r12, =SCB_ICSR
        ldr     r0, [r12]
        // toggle SCB_ICSR)PENDSVSET bit
        orr     r0, r0, #PENDSVSET
        // save SCB.ICSR
        str     r0, [r12]

        // exit
        bx      lr
        .end
*/

        
        .align      2
        .global     PendSV_Handler
        .thumb_func
        .type       PendSV_Handler, %function
PendSV_Handler:
        push    {lr}            // lr contains EXC_RETURN value

        // copy r0 from PSP as args to pendsv_handler
        mrs     r12, PSP
        ldr     r0, [r12]

        // store context on thread stack
        stmdb   r12!, {r4-r11}
        msr     PSP, r12

        bl      pendsv_handler
        // get new context from thread stack
        // pendsv_handler needs to set new PSP
        mrs     r12, PSP
        ldmia   r12!, {r4-r11}
        msr     PSP, r12

        // exit
        pop     {lr}            
        bx      lr              // actual return address is taken from PSP stack
        .end
