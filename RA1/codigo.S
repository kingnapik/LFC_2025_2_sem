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
    clr temp
    sts stack_ptr, temp

    ; Push: 3.14 -> 314 (positive)
    ldi val_l, 58
    ldi val_h, 1
    rcall stack_push

    ; Push: 2.0 -> 200 (positive)
    ldi val_l, 200
    ldi val_h, 0
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

    ; === EXPRESSAO 2 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 10.5 -> 1050 (positive)
    ldi val_l, 26
    ldi val_h, 4
    rcall stack_push

    ; Push: 3.2 -> 320 (positive)
    ldi val_l, 64
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

    ; Push: 4.0 -> 400 (positive)
    ldi val_l, 144
    ldi val_h, 1
    rcall stack_push

    ; Push: 5.0 -> 500 (positive)
    ldi val_l, 244
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

    ; === EXPRESSAO 4 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 15 -> 1500 (positive)
    ldi val_l, 220
    ldi val_h, 5
    rcall stack_push

    ; Push: 3 -> 300 (positive)
    ldi val_l, 44
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

    ; === EXPRESSAO 5 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 13.1 -> 1310 (positive)
    ldi val_l, 30
    ldi val_h, 5
    rcall stack_push

    ; Push: 15 -> 1500 (positive)
    ldi val_l, 220
    ldi val_h, 5
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

    ; === EXPRESSAO 6 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 17 -> 1700 (positive)
    ldi val_l, 164
    ldi val_h, 6
    rcall stack_push

    ; Push: 5 -> 500 (positive)
    ldi val_l, 244
    ldi val_h, 1
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 7 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 2.1 -> 209 (positive)
    ldi val_l, 209
    ldi val_h, 0
    rcall stack_push

    ; Push: 3 -> 300 (positive)
    ldi val_l, 44
    ldi val_h, 1
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 8 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 100.5 -> 10050 (positive)
    ldi val_l, 66
    ldi val_h, 39
    rcall stack_push

    ; Comando MEM
    rcall handle_mem

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 9 ===
    clr temp
    sts stack_ptr, temp

    ; Comando MEM
    rcall handle_mem

    ; Push: 50.0 -> 5000 (positive)
    ldi val_l, 136
    ldi val_h, 19
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

    ; === EXPRESSAO 10 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 2 -> 200 (positive)
    ldi val_l, 200
    ldi val_h, 0
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 11 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 3.0 -> 300 (positive)
    ldi val_l, 44
    ldi val_h, 1
    rcall stack_push

    ; Push: 4.0 -> 400 (positive)
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

    ; Push: 2.0 -> 200 (positive)
    ldi val_l, 200
    ldi val_h, 0
    rcall stack_push

    ; Push: 3.0 -> 300 (positive)
    ldi val_l, 44
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

    ; === EXPRESSAO 12 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 3.0 -> 300 (positive)
    ldi val_l, 44
    ldi val_h, 1
    rcall stack_push

    ; Push: 4.0 -> 400 (positive)
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

    ; Push: 2.0 -> 200 (positive)
    ldi val_l, 200
    ldi val_h, 0
    rcall stack_push

    ; Push: 3.0 -> 300 (positive)
    ldi val_l, 44
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

    ; Push: 2.135 -> 213 (positive)
    ldi val_l, 213
    ldi val_h, 0
    rcall stack_push

    ; Push: 2 -> 200 (positive)
    ldi val_l, 200
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

    ; === EXPRESSAO 14 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 10.5 -> 1050 (positive)
    ldi val_l, 26
    ldi val_h, 4
    rcall stack_push

    ; Push: 5 -> 500 (positive)
    ldi val_l, 244
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

    ; Push: 5 -> 500 (positive)
    ldi val_l, 244
    ldi val_h, 1
    rcall stack_push

    ; Push: 10 -> 1000 (positive)
    ldi val_l, 232
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

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 16 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 0 -> 0 (positive)
    ldi val_l, 0
    ldi val_h, 0
    rcall stack_push

    ; Push: 10 -> 1000 (positive)
    ldi val_l, 232
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

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 17 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 0.0 -> 0 (positive)
    ldi val_l, 0
    ldi val_h, 0
    rcall stack_push

    ; Push: 10.0 -> 1000 (positive)
    ldi val_l, 232
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

    ; Push: 20.0 -> 2000 (positive)
    ldi val_l, 208
    ldi val_h, 7
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 18 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 10.0 -> 1000 (positive)
    ldi val_l, 232
    ldi val_h, 3
    rcall stack_push

    ; Push: 20.0 -> 2000 (positive)
    ldi val_l, 208
    ldi val_h, 7
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

    ; === EXPRESSAO 19 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 10 -> 1000 (positive)
    ldi val_l, 232
    ldi val_h, 3
    rcall stack_push

    ; Push: 20 -> 2000 (positive)
    ldi val_l, 208
    ldi val_h, 7
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

    ; === EXPRESSAO 20 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 10 -> 1000 (positive)
    ldi val_l, 232
    ldi val_h, 3
    rcall stack_push

    ; Push: 20 -> 2000 (positive)
    ldi val_l, 208
    ldi val_h, 7
    rcall stack_push

    ; Resultado final
    rcall stack_pop
    rcall print_hex_result
    rcall print_newline

    ; === EXPRESSAO 21 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 0 -> 0 (positive)
    ldi val_l, 0
    ldi val_h, 0
    rcall stack_push

    ; Push: 5 -> 500 (positive)
    ldi val_l, 244
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

    ; === EXPRESSAO 22 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 4.5 -> 450 (positive)
    ldi val_l, 194
    ldi val_h, 1
    rcall stack_push

    ; Push: 5 -> 500 (positive)
    ldi val_l, 244
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

    ; === EXPRESSAO 23 ===
    clr temp
    sts stack_ptr, temp

    ; Push: 10.00 -> 1000 (positive)
    ldi val_l, 232
    ldi val_h, 3
    rcall stack_push

    ; Push: 10.00 -> 1000 (positive)
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

mul16x16:
    mov r18, val_l
    mov r19, val_h
    mov r20, op_l
    mov r21, op_h
    clr r22
    clr r23
    clr r24
    clr r25
    mul r18, r20
    mov r22, r0
    mov r23, r1
    mul r18, r21
    add r23, r0
    adc r24, r1
    adc r25, r1
    mul r19, r20
    add r23, r0
    adc r24, r1
    adc r25, r1
    mul r19, r21
    add r24, r0
    adc r25, r1
    clr r1
    mov r18, r22
    mov r19, r23
    mov r20, r24
    mov r21, r25
    clr r24
    clr r25
mul_div_loop:
    cpi  r18, 100
    cpc  r19, r1
    cpc  r20, r1
    cpc  r21, r1
    brlo mul_done
    subi r18, 100
    sbci r19, 0
    sbci r20, 0
    sbci r21, 0
    adiw r24, 1
    rjmp mul_div_loop
mul_done:
    mov val_l, r24
    mov val_h, r25
    ret

div16x16:
    tst op_l
    brne div_continue
    tst op_h
    breq div_error
div_continue:
    mov r18, val_l
    mov r19, val_h
    mov r20, op_l
    mov r21, op_h
    ldi r22, 100
    clr r23
    mul r18, r22
    mov r24, r0
    mov r25, r1
    mul r19, r22
    add r25, r0
    mov r18, r24
    mov r19, r25
    mov r20, op_l
    mov r21, op_h
    clr r24
    clr r25
div_loop:
    cp  r18, r20
    cpc r19, r21
    brlo div_done
    sub r18, r20
    sbc r19, r21
    adiw r24, 1
    rjmp div_loop
div_done:
    mov val_l, r24
    mov val_h, r25
    ret
div_error:
    ldi val_l, 0xF1
    ldi val_h, 0xD8
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
    rcall stack_push           ; Push valor de volta (MEM retorna o valor armazenado)
    ret
    
load_mem:
    ; CARREGAR: ler valor de MEM e fazer push
    lds val_l, memory_var
    lds val_h, memory_var+1
    rcall stack_push
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

