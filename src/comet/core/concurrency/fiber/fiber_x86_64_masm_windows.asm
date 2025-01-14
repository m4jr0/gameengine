; Copyright 2025 m4jr0. All Rights Reserved.
; Use of this source code is governed by the MIT
; license that can be found in the LICENSE file.

; Architecture: x86_64
; Platform: Windows (MASM)
; https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention

.code

; struct ExecutionContext
EXEC_CON_RBX   equ 000h
EXEC_CON_RBP   equ 008h
EXEC_CON_RDI   equ 010h
EXEC_CON_RSI   equ 018h
EXEC_CON_R12   equ 020h
EXEC_CON_R13   equ 028h
EXEC_CON_R14   equ 030h
EXEC_CON_R15   equ 038h
EXEC_CON_RSP   equ 040h
EXEC_CON_RIP   equ 048h
EXEC_CON_XMM6  equ 050h
EXEC_CON_XMM7  equ 060h
EXEC_CON_XMM8  equ 070h
EXEC_CON_XMM9  equ 080h
EXEC_CON_XMM10 equ 090h
EXEC_CON_XMM11 equ 0a0h
EXEC_CON_XMM12 equ 0b0h
EXEC_CON_XMM13 equ 0c0h
EXEC_CON_XMM14 equ 0d0h
EXEC_CON_XMM15 equ 0e0h
EXEC_CON_RCX   equ 0f0h

; Switch execution contexts.
; void SwitchExecutionContext(ExecutionContext* src, 
;                             const ExecutionContext* dst)
SwitchExecutionContext proc
    ; Step 1: store current context in src. ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Save callee registers state.
    mov    qword ptr [rcx + EXEC_CON_RBX],     rbx
    mov    qword ptr [rcx + EXEC_CON_RBP],     rbp
    mov    qword ptr [rcx + EXEC_CON_RDI],     rdi
    mov    qword ptr [rcx + EXEC_CON_RSI],     rsi
    mov    qword ptr [rcx + EXEC_CON_R12],     r12
    mov    qword ptr [rcx + EXEC_CON_R13],     r13
    mov    qword ptr [rcx + EXEC_CON_R14],     r14
    mov    qword ptr [rcx + EXEC_CON_R15],     r15
    movdqa xmmword ptr [rcx + EXEC_CON_XMM6],  xmm6
    movdqa xmmword ptr [rcx + EXEC_CON_XMM7],  xmm7
    movdqa xmmword ptr [rcx + EXEC_CON_XMM8],  xmm8
    movdqa xmmword ptr [rcx + EXEC_CON_XMM9],  xmm9
    movdqa xmmword ptr [rcx + EXEC_CON_XMM10], xmm10
    movdqa xmmword ptr [rcx + EXEC_CON_XMM11], xmm11
    movdqa xmmword ptr [rcx + EXEC_CON_XMM12], xmm12
    movdqa xmmword ptr [rcx + EXEC_CON_XMM13], xmm13
    movdqa xmmword ptr [rcx + EXEC_CON_XMM14], xmm14
    movdqa xmmword ptr [rcx + EXEC_CON_XMM15], xmm15

    ; Save return address state.
    mov rax,                            qword ptr [rsp]
    mov qword ptr [rcx + EXEC_CON_RIP], rax

    ; Save top of the stack.
    lea rax,                            [rsp + 08h]
    mov qword ptr [rcx + EXEC_CON_RSP], rax

    ; Step 2: restore context from dst. ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov r8, rdx

    mov    rbx,   qword ptr [r8 + EXEC_CON_RBX]
    mov    rbp,   qword ptr [r8 + EXEC_CON_RBP]
    mov    rdi,   qword ptr [r8 + EXEC_CON_RDI]
    mov    rsi,   qword ptr [r8 + EXEC_CON_RSI]
    mov    r12,   qword ptr [r8 + EXEC_CON_R12]
    mov    r13,   qword ptr [r8 + EXEC_CON_R13]
    mov    r14,   qword ptr [r8 + EXEC_CON_R14]
    mov    r15,   qword ptr [r8 + EXEC_CON_R15]
    movdqa xmm6,  xmmword ptr [r8 + EXEC_CON_XMM6]
    movdqa xmm7,  xmmword ptr [r8 + EXEC_CON_XMM7]
    movdqa xmm8,  xmmword ptr [r8 + EXEC_CON_XMM8]
    movdqa xmm9,  xmmword ptr [r8 + EXEC_CON_XMM9]
    movdqa xmm10, xmmword ptr [r8 + EXEC_CON_XMM10]
    movdqa xmm11, xmmword ptr [r8 + EXEC_CON_XMM11]
    movdqa xmm12, xmmword ptr [r8 + EXEC_CON_XMM12]
    movdqa xmm13, xmmword ptr [r8 + EXEC_CON_XMM13]
    movdqa xmm14, xmmword ptr [r8 + EXEC_CON_XMM14]
    movdqa xmm15, xmmword ptr [r8 + EXEC_CON_XMM15]

    ; data argument.
    mov rcx, qword ptr [r8 + EXEC_CON_RCX]

    mov rsp, qword ptr [r8 + EXEC_CON_RSP]

    mov rax, qword ptr [r8 + EXEC_CON_RIP]

    jmp rax
SwitchExecutionContext endp

end