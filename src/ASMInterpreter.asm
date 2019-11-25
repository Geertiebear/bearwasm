%macro next_instr 0
	add r14, 16
	mov rbx, [r13 + r14]
	mov rcx, qword opcodes
	jmp [rcx + (rbx * 8)]
%endmacro

vm_exit:
	pop rax ; store return value
	mov rsp, r15 ; restore stack
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rbp
	ret

mem_error:
	push -1
	jmp vm_exit

global_get:
	mov rax, [r13 + r14 + 8]
	push qword [r11 + (rax * 8)]
	next_instr

local_get:
	mov rax, [r13 + r14 + 8]
	push qword [r12 + (rax * 8)]
	next_instr

i32_load:
	mov eax, [r13 + r14 + 12] ; offset
	pop rbx
	add rax, rbx
	mov rcx, rax
	add rcx, 4
	mov rdx, [r9] ; memory size
	cmp rcx, rdx
	jg mem_error
	mov edx, [r10 + rax]
	push rdx
	next_instr

i32_load_8_s:
i32_load_8_u:
	mov eax, [r13 + r14 + 12] ; offset
	pop rbx
	add rax, rbx
	mov rcx, rax
	add rcx, 1
	mov rdx, [r9] ; memory size
	cmp rcx, rdx
	jg mem_error
	mov bl, [r10 + rax]
	push rbx
	next_instr

i32_add:
	pop rax
	pop rbx
	add rax, rbx
	push rax
	next_instr

i32_const:
	mov eax, [r13 + r14 + 8]
	push rax
	next_instr

instr_end:
	; TODO: this instruction is a bit more involved than this
	jmp vm_exit

global _vm_enter
_vm_enter:
	push rbp
	push rbx
	push r12
	push r13
	push r14
	push r15 ; save registers
	mov r15, rsp ; save stack

	mov rsp, [rdi] ; stack variable in state
	mov r14d, [rdi + 8] ; initial pc
	mov ebx, [rdi + 12] ; current_function
	mov rcx, [rdi + 16] ; expression pointer
	mov r13, [rcx + (rbx * 8)] ; current_expression
	mov rdx, [r13 + r14] ; instruction
	mov rcx, [rdi + 24] ; locals
	mov r12, [rcx + (rbx * 8)] ; current_locals
	mov r11, [rdi + 48] ; globals
	mov rbx, [rdi +	32] ; memories
	mov r10, [rbx] ; memories[0]
	mov r9, [rdi + 40] ; memory_sizes
	
	mov rbx, qword opcodes
	jmp [rbx + (rdx * 8)]

instr_unreachable:
	push qword [r13 + r14]
	jmp vm_exit

align 16
opcodes:
        dq instr_unreachable ; 0x0
        dq instr_unreachable ; 0x1
        dq instr_unreachable ; 0x2
        dq instr_unreachable ; 0x3
        dq instr_unreachable ; 0x4
        dq instr_unreachable ; 0x5
        dq instr_unreachable ; 0x6
        dq instr_unreachable ; 0x7
        dq instr_unreachable ; 0x8
        dq instr_unreachable ; 0x9
        dq instr_unreachable ; 0xa
        dq instr_end ; 0xb
        dq instr_unreachable ; 0xc
        dq instr_unreachable ; 0xd
        dq instr_unreachable ; 0xe
        dq instr_unreachable ; 0xf
        dq instr_unreachable ; 0x10
        dq instr_unreachable ; 0x11
        dq instr_unreachable ; 0x12
        dq instr_unreachable ; 0x13
        dq instr_unreachable ; 0x14
        dq instr_unreachable ; 0x15
        dq instr_unreachable ; 0x16
        dq instr_unreachable ; 0x17
        dq instr_unreachable ; 0x18
        dq instr_unreachable ; 0x19
        dq instr_unreachable ; 0x1a
        dq instr_unreachable ; 0x1b
        dq instr_unreachable ; 0x1c
        dq instr_unreachable ; 0x1d
        dq instr_unreachable ; 0x1e
        dq instr_unreachable ; 0x1f
        dq local_get ; 0x20
        dq instr_unreachable ; 0x21
        dq instr_unreachable ; 0x22
        dq global_get ; 0x23
        dq instr_unreachable ; 0x24
        dq instr_unreachable ; 0x25
        dq instr_unreachable ; 0x26
        dq instr_unreachable ; 0x27
        dq i32_load ; 0x28
        dq instr_unreachable ; 0x29
        dq instr_unreachable ; 0x2a
        dq instr_unreachable ; 0x2b
        dq i32_load_8_s ; 0x2c
        dq i32_load_8_u ; 0x2d
        dq instr_unreachable ; 0x2e
        dq instr_unreachable ; 0x2f
        dq instr_unreachable ; 0x30
        dq instr_unreachable ; 0x31
        dq instr_unreachable ; 0x32
        dq instr_unreachable ; 0x33
        dq instr_unreachable ; 0x34
        dq instr_unreachable ; 0x35
        dq instr_unreachable ; 0x36
        dq instr_unreachable ; 0x37
        dq instr_unreachable ; 0x38
        dq instr_unreachable ; 0x39
        dq instr_unreachable ; 0x3a
        dq instr_unreachable ; 0x3b
        dq instr_unreachable ; 0x3c
        dq instr_unreachable ; 0x3d
        dq instr_unreachable ; 0x3e
        dq instr_unreachable ; 0x3f
        dq instr_unreachable ; 0x40
        dq i32_const ; 0x41
        dq instr_unreachable ; 0x42
        dq instr_unreachable ; 0x43
        dq instr_unreachable ; 0x44
        dq instr_unreachable ; 0x45
        dq instr_unreachable ; 0x46
        dq instr_unreachable ; 0x47
        dq instr_unreachable ; 0x48
        dq instr_unreachable ; 0x49
        dq instr_unreachable ; 0x4a
        dq instr_unreachable ; 0x4b
        dq instr_unreachable ; 0x4c
        dq instr_unreachable ; 0x4d
        dq instr_unreachable ; 0x4e
        dq instr_unreachable ; 0x4f
        dq instr_unreachable ; 0x50
        dq instr_unreachable ; 0x51
        dq instr_unreachable ; 0x52
        dq instr_unreachable ; 0x53
        dq instr_unreachable ; 0x54
        dq instr_unreachable ; 0x55
        dq instr_unreachable ; 0x56
        dq instr_unreachable ; 0x57
        dq instr_unreachable ; 0x58
        dq instr_unreachable ; 0x59
        dq instr_unreachable ; 0x5a
        dq instr_unreachable ; 0x5b
        dq instr_unreachable ; 0x5c
        dq instr_unreachable ; 0x5d
        dq instr_unreachable ; 0x5e
        dq instr_unreachable ; 0x5f
        dq instr_unreachable ; 0x60
        dq instr_unreachable ; 0x61
        dq instr_unreachable ; 0x62
        dq instr_unreachable ; 0x63
        dq instr_unreachable ; 0x64
        dq instr_unreachable ; 0x65
        dq instr_unreachable ; 0x66
        dq instr_unreachable ; 0x67
        dq instr_unreachable ; 0x68
        dq instr_unreachable ; 0x69
        dq i32_add ; 0x6a
        dq instr_unreachable ; 0x6b
        dq instr_unreachable ; 0x6c
        dq instr_unreachable ; 0x6d
        dq instr_unreachable ; 0x6e
        dq instr_unreachable ; 0x6f
        dq instr_unreachable ; 0x70
        dq instr_unreachable ; 0x71
        dq instr_unreachable ; 0x72
        dq instr_unreachable ; 0x73
        dq instr_unreachable ; 0x74
        dq instr_unreachable ; 0x75
        dq instr_unreachable ; 0x76
        dq instr_unreachable ; 0x77
        dq instr_unreachable ; 0x78
        dq instr_unreachable ; 0x79
        dq instr_unreachable ; 0x7a
        dq instr_unreachable ; 0x7b
        dq instr_unreachable ; 0x7c
        dq instr_unreachable ; 0x7d
        dq instr_unreachable ; 0x7e
        dq instr_unreachable ; 0x7f
        dq instr_unreachable ; 0x80
        dq instr_unreachable ; 0x81
        dq instr_unreachable ; 0x82
        dq instr_unreachable ; 0x83
        dq instr_unreachable ; 0x84
        dq instr_unreachable ; 0x85
        dq instr_unreachable ; 0x86
        dq instr_unreachable ; 0x87
        dq instr_unreachable ; 0x88
        dq instr_unreachable ; 0x89
        dq instr_unreachable ; 0x8a
        dq instr_unreachable ; 0x8b
        dq instr_unreachable ; 0x8c
        dq instr_unreachable ; 0x8d
        dq instr_unreachable ; 0x8e
        dq instr_unreachable ; 0x8f
        dq instr_unreachable ; 0x90
        dq instr_unreachable ; 0x91
        dq instr_unreachable ; 0x92
        dq instr_unreachable ; 0x93
        dq instr_unreachable ; 0x94
        dq instr_unreachable ; 0x95
        dq instr_unreachable ; 0x96
        dq instr_unreachable ; 0x97
        dq instr_unreachable ; 0x98
        dq instr_unreachable ; 0x99
        dq instr_unreachable ; 0x9a
        dq instr_unreachable ; 0x9b
        dq instr_unreachable ; 0x9c
        dq instr_unreachable ; 0x9d
        dq instr_unreachable ; 0x9e
        dq instr_unreachable ; 0x9f
        dq instr_unreachable ; 0xa0
        dq instr_unreachable ; 0xa1
        dq instr_unreachable ; 0xa2
        dq instr_unreachable ; 0xa3
        dq instr_unreachable ; 0xa4
        dq instr_unreachable ; 0xa5
        dq instr_unreachable ; 0xa6
        dq instr_unreachable ; 0xa7
        dq instr_unreachable ; 0xa8
        dq instr_unreachable ; 0xa9
        dq instr_unreachable ; 0xaa
        dq instr_unreachable ; 0xab
        dq instr_unreachable ; 0xac
        dq instr_unreachable ; 0xad
        dq instr_unreachable ; 0xae
        dq instr_unreachable ; 0xaf
        dq instr_unreachable ; 0xb0
        dq instr_unreachable ; 0xb1
        dq instr_unreachable ; 0xb2
        dq instr_unreachable ; 0xb3
        dq instr_unreachable ; 0xb4
        dq instr_unreachable ; 0xb5
        dq instr_unreachable ; 0xb6
        dq instr_unreachable ; 0xb7
        dq instr_unreachable ; 0xb8
        dq instr_unreachable ; 0xb9
        dq instr_unreachable ; 0xba
        dq instr_unreachable ; 0xbb
        dq instr_unreachable ; 0xbc
        dq instr_unreachable ; 0xbd
        dq instr_unreachable ; 0xbe
        dq instr_unreachable ; 0xbf
        dq instr_unreachable ; 0xc0
        dq instr_unreachable ; 0xc1
        dq instr_unreachable ; 0xc2
        dq instr_unreachable ; 0xc3
        dq instr_unreachable ; 0xc4
        dq instr_unreachable ; 0xc5
        dq instr_unreachable ; 0xc6
        dq instr_unreachable ; 0xc7
        dq instr_unreachable ; 0xc8
        dq instr_unreachable ; 0xc9
        dq instr_unreachable ; 0xca
        dq instr_unreachable ; 0xcb
        dq instr_unreachable ; 0xcc
        dq instr_unreachable ; 0xcd
        dq instr_unreachable ; 0xce
        dq instr_unreachable ; 0xcf
        dq instr_unreachable ; 0xd0
        dq instr_unreachable ; 0xd1
        dq instr_unreachable ; 0xd2
        dq instr_unreachable ; 0xd3
        dq instr_unreachable ; 0xd4
        dq instr_unreachable ; 0xd5
        dq instr_unreachable ; 0xd6
        dq instr_unreachable ; 0xd7
        dq instr_unreachable ; 0xd8
        dq instr_unreachable ; 0xd9
        dq instr_unreachable ; 0xda
        dq instr_unreachable ; 0xdb
        dq instr_unreachable ; 0xdc
        dq instr_unreachable ; 0xdd
        dq instr_unreachable ; 0xde
        dq instr_unreachable ; 0xdf
        dq instr_unreachable ; 0xe0
        dq instr_unreachable ; 0xe1
        dq instr_unreachable ; 0xe2
        dq instr_unreachable ; 0xe3
        dq instr_unreachable ; 0xe4
        dq instr_unreachable ; 0xe5
        dq instr_unreachable ; 0xe6
        dq instr_unreachable ; 0xe7
        dq instr_unreachable ; 0xe8
        dq instr_unreachable ; 0xe9
        dq instr_unreachable ; 0xea
        dq instr_unreachable ; 0xeb
        dq instr_unreachable ; 0xec
        dq instr_unreachable ; 0xed
        dq instr_unreachable ; 0xee
        dq instr_unreachable ; 0xef
        dq instr_unreachable ; 0xf0
        dq instr_unreachable ; 0xf1
        dq instr_unreachable ; 0xf2
        dq instr_unreachable ; 0xf3
        dq instr_unreachable ; 0xf4
        dq instr_unreachable ; 0xf5
        dq instr_unreachable ; 0xf6
        dq instr_unreachable ; 0xf7
        dq instr_unreachable ; 0xf8
        dq instr_unreachable ; 0xf9
        dq instr_unreachable ; 0xfa
        dq instr_unreachable ; 0xfb
        dq instr_unreachable ; 0xfc
        dq instr_unreachable ; 0xfd
        dq instr_unreachable ; 0xfe
        dq instr_unreachable ; 0xff
