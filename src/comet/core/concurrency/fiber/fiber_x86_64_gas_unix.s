#  Copyright 2024 m4jr0. All Rights Reserved.
#  Use of this source code is governed by the MIT
#  license that can be found in the LICENSE file.

#  Architecture: x86_64
#  Platform: Unix (GAS)
#  https://wiki.osdev.org/System_V_ABI

.text
.align   4

#  struct ExecutionContext
.section .data
.equ     EXEC_CON_RBX, 0x00
.equ     EXEC_CON_RBP, 0x08
.equ     EXEC_CON_R12, 0x10
.equ     EXEC_CON_R13, 0x18
.equ     EXEC_CON_R14, 0x20
.equ     EXEC_CON_R15, 0x28
.equ     EXEC_CON_RSP, 0x30
.equ     EXEC_CON_RIP, 0x38
.equ     EXEC_CON_RDI, 0x40

.section .text

#  Switch execution contexts.
# void SwitchExecutionContext(ExecutionContext* src, 
#  const ExecutionContext* dst)
.global  SwitchExecutionContext
SwitchExecutionContext:
    # Step 1: store current context in src. ####################################
    # Save callee registers state.
    movq %rbx, EXEC_CON_RBX(%rdi)
    movq %rbp, EXEC_CON_RBP(%rdi)
    movq %r12, EXEC_CON_R12(%rdi)
    movq %r13, EXEC_CON_R13(%rdi)
    movq %r14, EXEC_CON_R14(%rdi)
    movq %r15, EXEC_CON_R15(%rdi)

    # Save return address state.
    movq (%rsp), %rcx
    movq %rcx,   EXEC_CON_RIP(%rdi)

    # Save top of the stack.
    lea  0x08(%rsp), %rcx
    movq %rcx,       EXEC_CON_RSP(%rdi)

    # Step 2: restore context from dst. ########################################
    movq %rsi, %r8

    movq EXEC_CON_RBX(%r8), %rbx
    movq EXEC_CON_RBP(%r8), %rbp
    movq EXEC_CON_R12(%r8), %r12
    movq EXEC_CON_R13(%r8), %r13
    movq EXEC_CON_R14(%r8), %r14
    movq EXEC_CON_R15(%r8), %r15

    # data argument.
    movq EXEC_CON_RDI(%r8), %rdi

    movq EXEC_CON_RSP(%r8), %rsp

    movq EXEC_CON_RIP(%r8), %rcx
    jmp  *%rcx
