#pragma once

#define Function(x) .global x; \
	.type x, @function;\
	x

#define Push_Caller_Saved \
    push %rbx; \
    push %rbp; \
    push %rdi; \
    push %rsi; \
    push %r12; \
    push %r13; \
    push %r14; \
    push %r15; \

#define Pop_Caller_Saved \
    pop %r15; \
    pop %r14; \
    pop %r13; \
    pop %r12; \
    pop %rsi; \
    pop %rdi; \
    pop %rbp; \
    pop %rbx;
