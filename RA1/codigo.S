; Codigo Assembly para Arduino UNO (ATmega328P)
; === CALCULADORA RPN ===
; Fixed-point: multiplicamos por 100 (2 casas decimais)

#include <avr/io.h>

; === DEFINICOES DE REGISTRADORES ===
#define temp     r16
#define temp2    r17
#define val_l    r24
#define val_h    r25
#define op_l     r20
#define op_h     r21

; === AREA DE DADOS ===
.section .bss
rpn_stack:     .space 64  ; Pilha RPN (32 valores de 16-bit)
memory_var:    .space 2   ; Variavel MEM (16-bit)
stack_ptr:     .space 1   ; Ponteiro da pilha RPN

.section .text
.global main

main:
    ; Inicializar sistema
    ldi temp, lo8(RAMEND)
    out _SFR_IO_ADDR(SPL), temp
    ldi temp, hi8(RAMEND)
    out _SFR_IO_ADDR(SPH), temp

    ; Zerar stack pointer e memoria
    clr temp
    sts stack_ptr, temp
    sts memory_var, temp      ; MEM = 0 inicialmente
    sts memory_var+1, temp

    ; Inicializar UART (9600 baud @ 16MHz)
    ldi temp, 103             ; UBRR = 103 para 9600 baud
    sts UBRR0L, temp
    clr temp
    sts UBRR0H, temp
    ldi temp, (1<<TXEN0)
    sts UCSR0B, temp
    ldi temp, (1<<UCSZ01)|(1<<UCSZ00)  ; 8-bit data
    sts UCSR0C, temp

    ; === EXPRESSAO 1 ===
    ; Push: 2 -> 200 (positive)
    ldi val_l, 200
    ldi val_h, 0
    rcall stack_push

    ; Push: 4 -> 400 (positive)
    ldi val_l, 144
    ldi val_h, 1
    rcall stack_push

    ; Subtracao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    sub val_l, op_l    ; op1 - op2
    sbc val_h, op_h
    rcall stack_push

    ; Push: 4 -> 400 (positive)
    ldi val_l, 144
    ldi val_h, 1
    rcall stack_push

    ; Multiplicacao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall mul16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 2 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 2 -> 200 (positive)
    ldi val_l, 200
    ldi val_h, 0
    rcall stack_push

    ; Push: 3 -> 300 (positive)
    ldi val_l, 44
    ldi val_h, 1
    rcall stack_push

    ; Subtracao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    sub val_l, op_l    ; op1 - op2
    sbc val_h, op_h
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 3 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 3 -> 300 (positive)
    ldi val_l, 44
    ldi val_h, 1
    rcall stack_push

    ; Push: 2 -> 200 (positive)
    ldi val_l, 200
    ldi val_h, 0
    rcall stack_push

    ; Subtracao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    sub val_l, op_l    ; op1 - op2
    sbc val_h, op_h
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 4 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 4 -> 400 (positive)
    ldi val_l, 144
    ldi val_h, 1
    rcall stack_push

    ; Push: 4 -> 400 (positive)
    ldi val_l, 144
    ldi val_h, 1
    rcall stack_push

    ; Adicao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    add val_l, op_l
    adc val_h, op_h
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 5 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 8 -> 800 (positive)
    ldi val_l, 32
    ldi val_h, 3
    rcall stack_push

    ; Push: 9 -> 900 (positive)
    ldi val_l, 132
    ldi val_h, 3
    rcall stack_push

    ; Subtracao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    sub val_l, op_l    ; op1 - op2
    sbc val_h, op_h
    rcall stack_push

    ; Push: 8 -> 800 (positive)
    ldi val_l, 32
    ldi val_h, 3
    rcall stack_push

    ; Adicao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    add val_l, op_l
    adc val_h, op_h
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 6 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 1 -> 100 (positive)
    ldi val_l, 100
    ldi val_h, 0
    rcall stack_push

    ; Push: 1 -> 100 (positive)
    ldi val_l, 100
    ldi val_h, 0
    rcall stack_push

    ; Multiplicacao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall mul16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 7 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 100 -> 10000 (positive)
    ldi val_l, 16
    ldi val_h, 39
    rcall stack_push

    ; Push: 0 -> 0 (positive)
    ldi val_l, 0
    ldi val_h, 0
    rcall stack_push

    ; Multiplicacao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall mul16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 8 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 3 -> 300 (positive)
    ldi val_l, 44
    ldi val_h, 1
    rcall stack_push

    ; Push: 0.20 -> 20 (positive)
    ldi val_l, 20
    ldi val_h, 0
    rcall stack_push

    ; Multiplicacao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall mul16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 9 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 25 -> 2500 (positive)
    ldi val_l, 196
    ldi val_h, 9
    rcall stack_push

    ; Push: 0.5 -> 50 (positive)
    ldi val_l, 50
    ldi val_h, 0
    rcall stack_push

    ; Multiplicacao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall mul16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 10 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 2 -> 200 (positive)
    ldi val_l, 200
    ldi val_h, 0
    rcall stack_push

    ; Push: 1 -> 100 (positive)
    ldi val_l, 100
    ldi val_h, 0
    rcall stack_push

    ; Multiplicacao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall mul16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 11 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 0.5 -> 50 (positive)
    ldi val_l, 50
    ldi val_h, 0
    rcall stack_push

    ; Push: 5.72 -> 571 (positive)
    ldi val_l, 59
    ldi val_h, 2
    rcall stack_push

    ; Multiplicacao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall mul16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 12 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 1 -> 100 (positive)
    ldi val_l, 100
    ldi val_h, 0
    rcall stack_push

    ; Push: 1 -> 100 (positive)
    ldi val_l, 100
    ldi val_h, 0
    rcall stack_push

    ; Divisao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall div16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 13 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 5 -> 500 (positive)
    ldi val_l, 244
    ldi val_h, 1
    rcall stack_push

    ; Push: 10 -> 1000 (positive)
    ldi val_l, 232
    ldi val_h, 3
    rcall stack_push

    ; Divisao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall div16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 14 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 2.34 -> 233 (positive)
    ldi val_l, 233
    ldi val_h, 0
    rcall stack_push

    ; Push: 3.88 -> 388 (positive)
    ldi val_l, 132
    ldi val_h, 1
    rcall stack_push

    ; Divisao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall div16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 15 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 40 -> 4000 (positive)
    ldi val_l, 160
    ldi val_h, 15
    rcall stack_push

    ; Push: 3.2 -> 320 (positive)
    ldi val_l, 64
    ldi val_h, 1
    rcall stack_push

    ; Divisao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall div16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 16 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 20 -> 2000 (positive)
    ldi val_l, 208
    ldi val_h, 7
    rcall stack_push

    ; Push: 10 -> 1000 (positive)
    ldi val_l, 232
    ldi val_h, 3
    rcall stack_push

    ; Divisao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall div16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 17 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 20.0 -> 2000 (positive)
    ldi val_l, 208
    ldi val_h, 7
    rcall stack_push

    ; Push: 7.0 -> 700 (positive)
    ldi val_l, 188
    ldi val_h, 2
    rcall stack_push

    ; Divisao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    rcall div16x16
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 18 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 10 -> 1000 (positive)
    ldi val_l, 232
    ldi val_h, 3
    rcall stack_push

    ; Comando MEM
    rcall handle_mem

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 19 ===
    clr temp
    sts stack_ptr, temp

    ; Comando MEM
    rcall handle_mem

    ; Push: 10 -> 1000 (positive)
    ldi val_l, 232
    ldi val_h, 3
    rcall stack_push

    ; Adicao
    rcall stack_pop    ; op2
    mov op_l, val_l
    mov op_h, val_h
    rcall stack_pop    ; op1
    add val_l, op_l
    adc val_h, op_h
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

main_loop:
    rjmp main_loop

; ========== SUBROTINAS ==========

stack_push:
    ; val_h:val_l -> pilha
    lds ZL, stack_ptr
    clr ZH
    lsl ZL                    ; * 2 (16-bit values)
    rol ZH
    subi ZL, lo8(-(rpn_stack))
    sbci ZH, hi8(-(rpn_stack))
    st Z+, val_l
    st Z, val_h
    
    ; Incrementar stack pointer
    lds temp, stack_ptr
    inc temp
    sts stack_ptr, temp
    ret

stack_pop:
    ; pilha -> val_h:val_l
    lds temp, stack_ptr
    dec temp
    sts stack_ptr, temp
    
    mov ZL, temp
    clr ZH
    lsl ZL                    ; * 2 (16-bit values)
    rol ZH
    subi ZL, lo8(-(rpn_stack))
    sbci ZH, hi8(-(rpn_stack))
    ld val_l, Z+
    ld val_h, Z
    ret

mul16x16:
    ; 16x16 multiplication with fixed-point division by 100
    ; Input: val_h:val_l and op_h:op_l
    ; Output: val_h:val_l = (val * op) / 100
    push r18
    push r22
    push r23
    push r26
    push r27
    
    ; Save signs and convert to absolute values
    clr r18                    ; Sign flag
    sbrs val_h, 7              ; Check if val is negative
    rjmp mul16_check_op
    com val_l                  ; Negate val
    com val_h
    subi val_l, 255            ; Add 1
    sbci val_h, 255
    inc r18                    ; Toggle sign
mul16_check_op:
    sbrs op_h, 7               ; Check if op is negative
    rjmp mul16_do_mult
    com op_l                   ; Negate op
    com op_h
    subi op_l, 255             ; Add 1
    sbci op_h, 255
    inc r18                    ; Toggle sign
    
mul16_do_mult:
    ; 16x16 -> 32-bit multiplication
    mul val_l, op_l            ; AL * BL
    movw r26, r0               ; r27:r26 = low result
    clr r23                    ; r23 = high extension
    
    mul val_l, op_h            ; AL * BH
    add r27, r0
    adc r23, r1
    
    mul val_h, op_l            ; AH * BL
    add r27, r0
    adc r23, r1
    
    mul val_h, op_h            ; AH * BH
    add r23, r0
    
    ; Divide by 100 using repeated subtraction
    clr val_l                  ; Initialize quotient
    clr val_h
    clr r1                     ; Make sure r1 is zero
    
mul16_div100:
    ; Check if remainder >= 100
    cpi r26, 100
    cpc r27, r1
    cpc r23, r1
    brlo mul16_div_done        ; If < 100, done
    
    ; Subtract 100
    subi r26, 100
    sbci r27, 0
    sbci r23, 0
    
    ; Increment quotient
    subi val_l, 255            ; Add 1
    sbci val_h, 255
    
    rjmp mul16_div100
    
mul16_div_done:
    ; Check for rounding (if remainder >= 50)
    cpi r26, 50
    brlo mul16_no_round
    subi val_l, 255            ; Round up
    sbci val_h, 255
    
mul16_no_round:
    ; Restore sign using r18
    sbrc r18, 0                ; Check sign flag
    rjmp mul16_negate
    rjmp mul16_done
    
mul16_negate:
    com val_l
    com val_h
    subi val_l, 255            ; Add 1
    sbci val_h, 255
    
mul16_done:
    clr r1                     ; AVR convention
    pop r27
    pop r26
    pop r23
    pop r22
    pop r18
    ret

div16x16:
    ; 16x16 division with fixed-point multiplication by 100
    ; Input: val_h:val_l (dividend, scaled x100) and op_h:op_l (divisor, scaled x100)
    ; Output: val_h:val_l = (val * 100) / op
    push r18
    push r19
    push r20
    push r21
    push r22
    push r23
    push r26
    push r27

    clr r1                     ; ensure zero reg for adc/cpc

    ; Save signs and convert to absolute values
    clr r18                    ; Sign parity flag
    sbrs val_h, 7              ; if dividend negative
    rjmp div16_check_divisor
    com val_l                  ; negate dividend
    com val_h
    subi val_l, 255            ; +1
    sbci val_h, 255
    inc r18
div16_check_divisor:
    sbrs op_h, 7               ; if divisor negative
    rjmp div16_make_num
    com op_l                   ; negate divisor
    com op_h
    subi op_l, 255             ; +1
    sbci op_h, 255
    inc r18

div16_make_num:
    ; Build 24-bit numerator N = val * 100 into r23:r27:r26
    clr r26
    clr r27
    clr r23
    ldi r19, 100               ; loop counter
div16_mult_loop:
    add r26, val_l             ; N += val (low)
    adc r27, val_h             ; N += val (high)
    adc r23, r1                ; carry into top byte
    dec r19
    brne div16_mult_loop

    ; Now divide N (24-bit) by op (16-bit) via repeated subtraction
    clr val_l                  ; quotient low
    clr val_h                  ; quotient high

div16_loop:
    ; if N < op then done (compare r23:r27:r26 with 0:op_h:op_l)
    cp  r26, op_l
    cpc r27, op_h
    cpc r23, r1
    brlo div16_div_done
    ; N -= op
    sub r26, op_l
    sbc r27, op_h
    sbc r23, r1
    ; quotient++
    subi val_l, 255
    sbci val_h, 255
    rjmp div16_loop

div16_div_done:
    ; Restore sign if odd number of negations
    sbrc r18, 0
    rjmp div16_negate
    rjmp div16_done

div16_negate:
    com val_l
    com val_h
    subi val_l, 255
    sbci val_h, 255

div16_done:
    clr r1                     ; ABI
    pop r27
    pop r26
    pop r23
    pop r22
    pop r21
    pop r20
    pop r19
    pop r18
    ret
handle_mem:
    ; MEM pode ser: armazenar (se tem valor na pilha) ou carregar (se pilha vazia)
    lds temp, stack_ptr
    cpi temp, 0
    breq load_mem              ; Se pilha vazia, carregar MEM
    
    ; ARMAZENAR: pop valor e salvar em MEM
store_mem:
    rcall stack_pop
    sts memory_var, val_l
    sts memory_var+1, val_h
    rcall stack_push           ; Push valor de volta
    ret
    
load_mem:
    ; CARREGAR: ler valor de MEM e fazer push
    lds val_l, memory_var
    lds val_h, memory_var+1
    rcall stack_push
    ret

print_hex_result:
    ; Check if negative (MSB set)
    sbrs val_h, 7
    rjmp print_positive
    
    ; Print minus sign for negative
    ldi temp, '-'
    rcall uart_send
    
    ; Convert to positive (two's complement)
    com val_l
    com val_h
    subi val_l, -1
    sbci val_h, -1
    
print_positive:
    ; Print "0x" prefix
    ldi temp, '0'
    rcall uart_send
    ldi temp, 'x'
    rcall uart_send
    
    ; Print high byte in hex
    mov temp, val_h
    rcall print_hex_byte
    
    ; Print low byte in hex
    mov temp, val_l
    rcall print_hex_byte
    ret

print_hex_byte:
    ; Print one byte in hex (2 digits)
    push temp
    
    ; High nibble
    mov temp2, temp
    swap temp2
    andi temp2, 0x0F
    cpi temp2, 10
    brlo hex_high_digit
    ; Letter A-F
    subi temp2, -55          ; Convert 10-15 to 'A'-'F'
    rjmp hex_high_send
hex_high_digit:
    ; Digit 0-9
    subi temp2, -48          ; Convert 0-9 to '0'-'9'
hex_high_send:
    mov temp, temp2
    rcall uart_send
    
    ; Low nibble
    pop temp
    andi temp, 0x0F
    cpi temp, 10
    brlo hex_low_digit
    ; Letter A-F
    subi temp, -55           ; Convert 10-15 to 'A'-'F'
    rjmp hex_low_send
hex_low_digit:
    ; Digit 0-9
    subi temp, -48           ; Convert 0-9 to '0'-'9'
hex_low_send:
    rcall uart_send
    ret

print_newline:
    ldi temp, '\n'
    rcall uart_send
    ret

uart_send:
uart_wait:
    lds temp2, UCSR0A
    sbrs temp2, UDRE0
    rjmp uart_wait
    sts UDR0, temp
    ret

