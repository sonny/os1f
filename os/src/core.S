// file: core.S
// author: Greg

    .syntax unified
    .thumb
    .file       "core.S"

        // offset of stack pointer in task struct
        .set SP_OFFSET, 0

        .text


    .align      2
    .global     context_handler
    .thumb_func
    .type       context_handler, %function
context_handler:
        cpsid   I
        mrs     r1, PSP
        stmdb   r1!, {r4-r11,lr}
#ifdef ENABLE_FP
        vstmdb  r1!, {s16-s31}
#endif
        ldr     r0, =current_task_ptr
        ldr     r0, [r0]
        str     r1, [r0, #SP_OFFSET]
        bl      task_next
        ldr     r0, [r0, #SP_OFFSET]
#ifdef ENABLE_FP
        vldmia  r0!, {s16-s31}
#endif
        ldmia   r0!, {r4-r11, lr}
        msr     PSP, r0
        cpsie   I
        bx      lr

        .end
