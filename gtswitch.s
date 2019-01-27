	.text
        .globl gt_switch
        .type gt_switch, @function

/*
 * Assumes x86-64.  See https://wiki.osdev.org/System_V_ABI
 *
 * void gt_switch(strut gt_ctx *old, struct gt_ctx *new)
 *
 * On X86-64, parameters are first passed via rdi, rsi, rdx, rcx, r8, and r9.
 * Other values are passed on the stack (in reverse order).  Hence rdi is old,
 * and rsi is new.
 */
gt_switch:
        // Save the callee-saved registers to the 'old' thread context
        mov %rsp, 0x00(%rdi)
        mov %r15, 0x08(%rdi)
        mov %r14, 0x10(%rdi)
        mov %r13, 0x18(%rdi)
        mov %r12, 0x20(%rdi)
        mov %rbx, 0x28(%rdi)
        mov %rbp, 0x30(%rdi)

        // Restore the callee-saved registeres from the 'new' thread context
        mov 0x00(%rsi), %rsp
        mov 0x08(%rsi), %r15
        mov 0x10(%rsi), %r14
        mov 0x18(%rsi), %r13
        mov 0x20(%rsi), %r12
        mov 0x28(%rsi), %rbx
        mov 0x30(%rsi), %rbp

        // Calling `ret` once we've restored the new thread context either:
        //   - Returns back to a stack where `gt_yield()` cooperatively
        //     initiated the context switch, or
        //   - Returns back to a stack that's bottomed out 
        ret
