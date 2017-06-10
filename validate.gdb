# validate.gdb

delete
b context_handler

commands
        echo START of contect handler\n
        p *current_task_ptr
        set $ctp = current_task_ptr
        set $stack_size = $ctp->stack_start - $ctp->stack_end
        set $sp_off = $ctp->stack_start - $psp
        set $tpc = ((struct stacked_regs*)$psp)->pc
        info symbol $tpc
        printf "Stack Size: %d, SP Offset: %d\n", $stack_size, $sp_off

         c
end

# after task switch
b core.S:30
commands
        echo AFTER context switch\n
        p *current_task_ptr

        set $ctp = current_task_ptr
        set $stack_size = $ctp->stack_start - $ctp->stack_end
        set $sp_off = $ctp->stack_start - $ctp->stack

        set $tpc = ((struct regs*)$ctp->stack)->stacked->pc
        info symbol $tpc
        printf "Stack Size: %d, SP Offset: %d\n", $stack_size, $sp_off
end

