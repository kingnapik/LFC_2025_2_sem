/*
guilherme knapik - kingnapik
grupo 3 RA1
*/
#include <vector>
#include <string>
#include <cctype>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <stack>
#include <sstream>
#include <cmath>
#include <map>
#include <algorithm>
#include <iomanip>

using namespace std;

struct Value{
    float val;
    bool isInt;
};

//globais para memoria e resultados
static map<string, float> memoria; //suporte a mutiplas variaveis de memoria
static vector<float> resultados;

//pre declaracao dos estados AFD
void estadoInicial(const string& token, vector<string>& tokens);
void estadoNumero(const string& token, vector<string>& tokens);
void estadoOperador(const string& token, vector<string>& tokens);
void estadoParenteses(const string& token, vector<string>& tokens);
void estadoComando(const string& token, vector<string>& tokens);

//func da main
vector<string> parseExpressao(string linha, vector<string>& _tokens_);
float executarExpressao(const vector<string>& _tokens_, vector<float>& resultados);
void gerarAssembly(const vector<vector<string>>& todasExpressoes, string& codigoAssembly);
void lerArquivo(string nomeArquivo, vector<string>& linhas);
void exibirResultados(const vector<float>& resultados);
void exibirMenu();

//16-bit helper para conersao
float to16BitFloat(float value) {
    return round(value * 100.0f) / 100.0f;
}

void estadoInicial(const string& token, vector<string>& tokens) {
    if (token.empty()) return;

    char firstChar = token[0];
    if (isdigit(firstChar) || firstChar == '.') { //pega num tokens
        estadoNumero(token, tokens);
    } else if (firstChar == '+' || firstChar == '-' || firstChar == '*' || firstChar == '/' || firstChar == '%' || firstChar == '^') { //pega op tokens
        estadoOperador(token, tokens);
    } else if (firstChar == '(' || firstChar == ')') { //pega paren tokens
        estadoParenteses(token, tokens);
    } else if (isalpha(firstChar)) { //pega comando tokens
        estadoComando(token, tokens);
    } else {
        throw invalid_argument("Token invalido: '" + token + "'"); //tokens errados desde o inicio
    }
}

void estadoNumero(const string& token, vector<string>& tokens) {
    bool temDec = false;
    
    for (char c : token) {
        if (c == '.') { //tratando ponto duplo, quando tem ponto 'temDec' vira true, se cair outro ponto, ele levanta o erro de ponto duplo
            if (temDec) {
                throw invalid_argument("Multiplos pontos decimais");
            }
            temDec = true;
        } else if (!isdigit(c)) {
            throw invalid_argument("Numero malformado: '" + token + "'"); //valida se não passaram nada como 12@34 ou 1.24a
        }
    }
    
    if (token[0] == '.' || token.back() == '.') {
        throw invalid_argument("Ponto no lugar errado: '" + token + "'"); //verifica se o numero foi passado com ponto no lugar errado, tipo .50 ou 45.
    }
    
    tokens.push_back(token);
}

void estadoOperador(const string& token, vector<string>& tokens) {
    if (token.length() != 1) { //pega erros como ++ ou outro lixo passdo junto como +op
        throw invalid_argument("Operador escrito errado: '" + token + "'");
    }
    
    tokens.push_back(token);
}

void estadoParenteses(const string& token, vector<string>& tokens) {
    if (token.length() != 1) { //pega casos de token errado novamente
        throw invalid_argument("Parenteses invalido: '" + token + "'");
    }
    
    tokens.push_back(token);
}

void estadoComando(const string& token, vector<string>& tokens) {
    for (char c : token) {
        if (!isalpha(c)) {
            throw invalid_argument("Comando invalido: " + token); //pega casos como RE$
        }
        if (islower(c)) {
            throw invalid_argument("Comando deve ser todo maiusculo: " + token); //pega casos como Mem
        }
    }
    tokens.push_back(token);

    /* //artefato de logica velha onde tratava RES e outros inputs separadamente
    if (comando == "RES" || comando.length() >= 1) { 
        tokens.push_back(comando);
    }else {
        throw invalid_argument("Comando invalido: " + comando);
    }*/
}

vector<string> parseExpressao(string linha, vector<string>& _tokens_) {
    _tokens_.clear(); //limpa anterior
    
    try {
        vector<string> rawTokens; //inicio da 'tokenizacao'
        string stringAtual = "";
        
        for (size_t i = 0; i < linha.length(); i++) {
            char c = linha[i];
            
            if (c == ' ' || c == '\t') { //espaco finaliza token
                if (!stringAtual.empty()) {
                    rawTokens.push_back(stringAtual);
                    stringAtual = "";
                }
            } else if (c == '(' || c == ')') { // ou parenteses para finalizar o token tbm
                if (!stringAtual.empty()) {
                    rawTokens.push_back(stringAtual);
                    stringAtual = "";
                }
                rawTokens.push_back(string(1, c));
            } else { // char qualquer entao add na string atual
                stringAtual += c;
            }
        }
        if (!stringAtual.empty()) { //add do token final no vetor
            rawTokens.push_back(stringAtual);
        }
        
        if (rawTokens.empty()) { //pega expresao vazia, no caso, whitespace (so espaços)
            throw invalid_argument("Expressao vazia");
        }
        
        for (const auto& tok : rawTokens) { //passa token para validacao
            estadoInicial(tok, _tokens_);
        }
        
        return _tokens_;
    } catch (const exception& e) {
        throw;
    }
}

float executarExpressao(const vector<string>& _tokens_, vector<float>& resultados) {
    stack<Value> values;
    
    for (const auto& token : _tokens_) {
        if (token == "(" || token == ")") { //parenteses sao marcadores de grupo, entao pula
            continue;
        } else if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1 && isdigit(token[1]))) { //push num para o stack
            bool isInt = (token.find('.')==string::npos);//se nao tem ponto e inteiro
            values.push({stof(token), isInt});
        } else if (token == "RES") { //se recebe RES e values esta vazio, RES nao tem parametro e esta errado
            if (values.empty()) {
                throw invalid_argument("RES precisa de um parametro");
            }
            
            int n = static_cast<int>(values.top().val); //tira valor do index de RES e testa pra ver se ele e valido vvv
            values.pop();
            if (n <= 0 || n > resultados.size()) { //tem que ser maior que 0 e menos que o total de valores em resultado
                throw invalid_argument("Indice RES invalido: " + to_string(n)); //senao trigger de erro
            }
            
            float resVal = resultados[resultados.size()-n];
            bool eraInt = (floor(resVal) == resVal);
            values.push({resVal, eraInt}); //precisa disso para poder retornar da memoria se era int ou nao de maneira correta
        } else if (token != "RES" && isalpha(token[0])) { //qqr outra coisa e memoria
            if (!values.empty()) { //se vazio, e para guardar na memoria
                Value valor = values.top(); //guardando com struct completa
                values.pop();
                memoria[token] = to16BitFloat(valor.val); //guarda o valor com 16-bit precision
                
                string intFlag = token + "_isInt";// guarda se e int ou nao separado
                memoria[intFlag] = valor.isInt ? 1.0f : 0.0f;
                values.push({to16BitFloat(valor.val), valor.isInt});
            } 
            else { //se values estier vazio, e para acessar a memoria e nao guardar
                if (memoria.find(token) != memoria.end()) {
                    string intFlag = token + "_isInt";// devolve o status se e int ou nao
                    bool wasInt = (memoria.find(intFlag) != memoria.end()) && (memoria[intFlag] == 1.0f);
                    values.push({memoria[token], wasInt}); //push do token para memoria
                } 
                else {
                    values.push({0.0f, true});//se nao tiver nada retorna 0.0 como inteiro
                }
            }
        } else {
            if (values.size() < 2) {
                throw invalid_argument("quantidade de numeros insuficiente para operacao");
            }
            //conta sempre segue o mesmo padrao a b op
            Value b = values.top(); values.pop(); 
            Value a = values.top(); values.pop();
            float result;
            
            if (token == "+") result = to16BitFloat(a.val + b.val); //soma e simples
            else if (token == "-") result = to16BitFloat(a.val - b.val); //subtracao tbm
            else if (token == "*") result = to16BitFloat(a.val * b.val); //mult e simples tbm
            else if (token == "/") { //divisao e mais complicada, comecando por filtrar divisao por zero vvv
                if (b.val == 0) {
                    throw invalid_argument("Divisao por zero");
                }
                if (a.isInt && b.isInt) { // se os dois forem int, etao e div real
                    result = to16BitFloat(static_cast<float>(static_cast<int>(a.val) / static_cast<int>(b.val)));
                } else { //senao e numero com casa decimal e tem que ser feita a div real
                    result = to16BitFloat(a.val / b.val);
                }
            }
            else if (token == "%") {
                if (b.val == 0) {
                    throw invalid_argument("Divisao por zero");
                }else if(!a.isInt || !b.isInt){ //verifica se ambos são inteiros ou nao
                    throw invalid_argument("% Tem que ser entre inteiros");
                }
                result = to16BitFloat(static_cast<float>(static_cast<int>(a.val) % static_cast<int>(b.val))); //casting para int por conta do c++ so fazer operacao de % com int e nao float
            }
            else if (token == "^") {
                if (floor(b.val) != b.val) { //verifica se o b e inteiro para o expoente 
                    throw invalid_argument("Expoente precisa ser inteiro");
                }
                result = to16BitFloat(pow(a.val, b.val)); //passa em precisao 16bits
            }
            else throw invalid_argument("Operador desconhecido: " + token);
            
            values.push({result, false}); //push do resultado
        }
    }
    
    if (values.size() != 1) { //ter certesa que temos apenas um valor (sanity)
        throw invalid_argument("Expressao RPN malformada - resultado final invalido");
    }
    
    float resultado = to16BitFloat(values.top().val); //ultima conversao para 16bit precision para ter certesa que foi arredondado mesmo "fail safe"
    resultados.push_back(resultado); //passa para resultado para o RES poder achar
    return resultado;
}

void gerarAssembly(const vector<vector<string>>& todasExpressoes, string& codigoAssembly) {
    stringstream assembly;
    
    assembly << "; Codigo Assembly para Arduino UNO (ATmega328P)\n";
    assembly << "; === CALCULADORA RPN ===\n";
    assembly << "; Fixed-point: multiplicamos por 100 (2 casas decimais)\n\n";
    
    assembly << "#include <avr/io.h>\n\n";
    
    // definições de registradores
    assembly << "; === DEFINICOES DE REGISTRADORES ===\n";
    assembly << "#define temp     r16\n";
    assembly << "#define temp2    r17\n";
    assembly << "#define val_l    r24\n";
    assembly << "#define val_h    r25\n";
    assembly << "#define op_l     r20\n";
    assembly << "#define op_h     r21\n\n";
    
    // área de dados
    assembly << "; === AREA DE DADOS ===\n";
    assembly << ".section .bss\n";
    assembly << "rpn_stack:     .space 64  ; Pilha RPN (32 valores de 16-bit)\n";
    assembly << "memory_var:    .space 2   ; Variavel MEM (16-bit)\n";
    assembly << "stack_ptr:     .space 1   ; Ponteiro da pilha RPN\n\n";
    
    assembly << ".section .text\n";
    assembly << ".global main\n\n";
    
    // função principal
    assembly << "main:\n";
    assembly << "    ; Inicializar sistema\n";
    assembly << "    ldi temp, lo8(RAMEND)\n";
    assembly << "    out _SFR_IO_ADDR(SPL), temp\n";
    assembly << "    ldi temp, hi8(RAMEND)\n";
    assembly << "    out _SFR_IO_ADDR(SPH), temp\n\n";
    
    assembly << "    ; Zerar stack pointer e memoria\n";
    assembly << "    clr temp\n";
    assembly << "    sts stack_ptr, temp\n";
    assembly << "    sts memory_var, temp      ; MEM = 0 inicialmente\n";
    assembly << "    sts memory_var+1, temp\n\n";
    
    assembly << "    ; Inicializar UART (9600 baud @ 16MHz)\n";
    assembly << "    ldi temp, 103             ; UBRR = 103 para 9600 baud\n";
    assembly << "    sts UBRR0L, temp\n";
    assembly << "    clr temp\n";
    assembly << "    sts UBRR0H, temp\n";
    assembly << "    ldi temp, (1<<TXEN0)\n";  
    assembly << "    sts UCSR0B, temp\n";
    assembly << "    ldi temp, (1<<UCSZ01)|(1<<UCSZ00)  ; 8-bit data\n";
    assembly << "    sts UCSR0C, temp\n\n";
    
    // processa expressões RPN
    int expr_num = 1;
    for (const auto& tokens : todasExpressoes) {
        assembly << "    ; === EXPRESSAO " << expr_num << " ===\n";
        
        // OTIMIZAÇÃO: só reseta se não é a primeira expressão
        if (expr_num > 1) {
            assembly << "    clr temp\n";
            assembly << "    sts stack_ptr, temp\n\n";
        }
        
        // processar tokens - MANTENDO SUAS OPERAÇÕES ORIGINAIS
        for (const auto& token : tokens) {
            if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
                // número: converter para fixed-point
                float valor = stof(token);
                int fixed_val = (int)(valor * 100);
                
                assembly << "    ; Push: " << token << " -> " << fixed_val;
                if (fixed_val < 0) {
                    assembly << " (negative)";
                    int16_t signed_val = (int16_t)fixed_val;
                    uint16_t unsigned_rep = (uint16_t)signed_val;
                    assembly << " = 0x" << hex << unsigned_rep << dec << "\n";
                    assembly << "    ldi val_l, " << (unsigned_rep & 0xFF) << "\n";
                    assembly << "    ldi val_h, " << ((unsigned_rep >> 8) & 0xFF) << "\n";
                } else {
                    assembly << " (positive)\n";
                    assembly << "    ldi val_l, " << (fixed_val & 0xFF) << "\n";
                    assembly << "    ldi val_h, " << ((fixed_val >> 8) & 0xFF) << "\n";
                }
                assembly << "    rcall stack_push\n\n";
            }
            else if (token == "+") {
                assembly << "    ; Adicao\n";
                assembly << "    rcall stack_pop    ; op2\n";
                assembly << "    mov op_l, val_l\n";
                assembly << "    mov op_h, val_h\n";
                assembly << "    rcall stack_pop    ; op1\n";
                assembly << "    add val_l, op_l\n";
                assembly << "    adc val_h, op_h\n";
                assembly << "    rcall stack_push\n\n";
            }
            else if (token == "-") {
                assembly << "    ; Subtracao\n";
                assembly << "    rcall stack_pop    ; op2\n";
                assembly << "    mov op_l, val_l\n";
                assembly << "    mov op_h, val_h\n";
                assembly << "    rcall stack_pop    ; op1\n";
                assembly << "    sub val_l, op_l    ; op1 - op2\n";
                assembly << "    sbc val_h, op_h\n";
                assembly << "    rcall stack_push\n\n";
            }
            else if (token == "*") {
                assembly << "    ; Multiplicacao\n";
                assembly << "    rcall stack_pop    ; op2\n";
                assembly << "    mov op_l, val_l\n";
                assembly << "    mov op_h, val_h\n";
                assembly << "    rcall stack_pop    ; op1\n";
                assembly << "    rcall mul16x16\n";
                assembly << "    rcall stack_push\n\n";
            }
            else if (token == "/") {
                assembly << "    ; Divisao\n";
                assembly << "    rcall stack_pop    ; op2\n";
                assembly << "    mov op_l, val_l\n";
                assembly << "    mov op_h, val_h\n";
                assembly << "    rcall stack_pop    ; op1\n";
                assembly << "    rcall div16x16\n";
                assembly << "    rcall stack_push\n\n";
            }
            else if (token == "MEM") {
                assembly << "    ; Comando MEM\n";
                assembly << "    rcall handle_mem\n\n";
            }
        }
        
        // resultado e impressão
        assembly << "    ; Resultado final\n";
        assembly << "    rcall stack_pop\n";
        assembly << "    rcall print_hex_result\n";  // MANTENDO SEU ORIGINAL
        assembly << "    rcall print_newline\n\n";
        
        expr_num++;
    }
    
    assembly << "main_loop:\n";
    assembly << "    rjmp main_loop\n\n";
    
    // ===== SUBROTINAS (SUAS ORIGINAIS) =====
    assembly << "; ========== SUBROTINAS ==========\n\n";
    
    // Push na pilha RPN (SUA VERSÃO ORIGINAL)
    assembly << "stack_push:\n";
    assembly << "    ; val_h:val_l -> pilha\n";
    assembly << "    lds ZL, stack_ptr\n";
    assembly << "    clr ZH\n";
    assembly << "    lsl ZL                    ; * 2 (16-bit values)\n";
    assembly << "    rol ZH\n";
    assembly << "    subi ZL, lo8(-(rpn_stack))\n";
    assembly << "    sbci ZH, hi8(-(rpn_stack))\n";
    assembly << "    st Z+, val_l\n";
    assembly << "    st Z, val_h\n";
    assembly << "    \n";
    assembly << "    ; Incrementar stack pointer\n";
    assembly << "    lds temp, stack_ptr\n";
    assembly << "    inc temp\n";
    assembly << "    sts stack_ptr, temp\n";
    assembly << "    ret\n\n";
    
    // Pop da pilha RPN (SUA VERSÃO ORIGINAL)
    assembly << "stack_pop:\n";
    assembly << "    ; pilha -> val_h:val_l\n";
    assembly << "    lds temp, stack_ptr\n";
    assembly << "    dec temp\n";
    assembly << "    sts stack_ptr, temp\n";
    assembly << "    \n";
    assembly << "    mov ZL, temp\n";
    assembly << "    clr ZH\n";
    assembly << "    lsl ZL                    ; * 2 (16-bit values)\n";
    assembly << "    rol ZH\n";
    assembly << "    subi ZL, lo8(-(rpn_stack))\n";
    assembly << "    sbci ZH, hi8(-(rpn_stack))\n";
    assembly << "    ld val_l, Z+\n";
    assembly << "    ld val_h, Z\n";
    assembly << "    ret\n\n";
    
    // SUA MULTIPLICAÇÃO ORIGINAL (QUE FUNCIONAVA!)
    assembly << "mul16x16:\n";
    assembly << "    ; 16x16 multiplication with fixed-point division by 100\n";
    assembly << "    ; Input: val_h:val_l and op_h:op_l\n";
    assembly << "    ; Output: val_h:val_l = (val * op) / 100\n";
    assembly << "    push r18\n";
    assembly << "    push r22\n";
    assembly << "    push r23\n";
    assembly << "    push r26\n";
    assembly << "    push r27\n";
    assembly << "    \n";
    assembly << "    ; Save signs and convert to absolute values\n";
    assembly << "    clr r18                    ; Sign flag\n";
    assembly << "    sbrs val_h, 7              ; Check if val is negative\n";
    assembly << "    rjmp mul16_check_op\n";
    assembly << "    com val_l                  ; Negate val\n";
    assembly << "    com val_h\n";
    assembly << "    subi val_l, 255            ; Add 1\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    inc r18                    ; Toggle sign\n";
    assembly << "mul16_check_op:\n";
    assembly << "    sbrs op_h, 7               ; Check if op is negative\n";
    assembly << "    rjmp mul16_do_mult\n";
    assembly << "    com op_l                   ; Negate op\n";
    assembly << "    com op_h\n";
    assembly << "    subi op_l, 255             ; Add 1\n";
    assembly << "    sbci op_h, 255\n";
    assembly << "    inc r18                    ; Toggle sign\n";
    assembly << "    \n";
    assembly << "mul16_do_mult:\n";
    assembly << "    ; 16x16 -> 32-bit multiplication\n";
    assembly << "    mul val_l, op_l            ; AL * BL\n";
    assembly << "    movw r26, r0               ; r27:r26 = low result\n";
    assembly << "    clr r23                    ; r23 = high extension\n";
    assembly << "    \n";
    assembly << "    mul val_l, op_h            ; AL * BH\n";
    assembly << "    add r27, r0\n";
    assembly << "    adc r23, r1\n";
    assembly << "    \n";
    assembly << "    mul val_h, op_l            ; AH * BL\n";
    assembly << "    add r27, r0\n";
    assembly << "    adc r23, r1\n";
    assembly << "    \n";
    assembly << "    mul val_h, op_h            ; AH * BH\n";
    assembly << "    add r23, r0\n";
    assembly << "    \n";
    assembly << "    ; Divide by 100 using repeated subtraction\n";
    assembly << "    clr val_l                  ; Initialize quotient\n";
    assembly << "    clr val_h\n";
    assembly << "    clr r1                     ; Make sure r1 is zero\n";
    assembly << "    \n";
    assembly << "mul16_div100:\n";
    assembly << "    ; Check if remainder >= 100\n";
    assembly << "    cpi r26, 100\n";
    assembly << "    cpc r27, r1\n";
    assembly << "    cpc r23, r1\n";
    assembly << "    brlo mul16_div_done        ; If < 100, done\n";
    assembly << "    \n";
    assembly << "    ; Subtract 100\n";
    assembly << "    subi r26, 100\n";
    assembly << "    sbci r27, 0\n";
    assembly << "    sbci r23, 0\n";
    assembly << "    \n";
    assembly << "    ; Increment quotient\n";
    assembly << "    subi val_l, 255            ; Add 1\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    \n";
    assembly << "    rjmp mul16_div100\n";
    assembly << "    \n";
    assembly << "mul16_div_done:\n";
    assembly << "    ; Check for rounding (if remainder >= 50)\n";
    assembly << "    cpi r26, 50\n";
    assembly << "    brlo mul16_no_round\n";
    assembly << "    subi val_l, 255            ; Round up\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    \n";
    assembly << "mul16_no_round:\n";
    assembly << "    ; Restore sign using r18\n";
    assembly << "    sbrc r18, 0                ; Check sign flag\n";
    assembly << "    rjmp mul16_negate\n";
    assembly << "    rjmp mul16_done\n";
    assembly << "    \n";
    assembly << "mul16_negate:\n";
    assembly << "    com val_l\n";
    assembly << "    com val_h\n";
    assembly << "    subi val_l, 255            ; Add 1\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    \n";
    assembly << "mul16_done:\n";
    assembly << "    clr r1                     ; AVR convention\n";
    assembly << "    pop r27\n";
    assembly << "    pop r26\n";
    assembly << "    pop r23\n";
    assembly << "    pop r22\n";
    assembly << "    pop r18\n";
    assembly << "    ret\n\n";

    assembly << "div16x16:\n";
    assembly << "    ; 16x16 division with fixed-point multiplication by 100\n";
    assembly << "    ; Input: val_h:val_l (dividend, scaled x100) and op_h:op_l (divisor, scaled x100)\n";
    assembly << "    ; Output: val_h:val_l = (val * 100) / op\n";
    assembly << "    push r18\n";
    assembly << "    push r19\n";
    assembly << "    push r20\n";   // (op_l) saved, but we won't clobber it anymore
    assembly << "    push r21\n";   // (op_h)
    assembly << "    push r22\n";
    assembly << "    push r23\n";
    assembly << "    push r26\n";
    assembly << "    push r27\n";
    assembly << "\n";
    assembly << "    clr r1                     ; ensure zero reg for adc/cpc\n";
    assembly << "\n";
    assembly << "    ; Save signs and convert to absolute values\n";
    assembly << "    clr r18                    ; Sign parity flag\n";
    assembly << "    sbrs val_h, 7              ; if dividend negative\n";
    assembly << "    rjmp div16_check_divisor\n";
    assembly << "    com val_l                  ; negate dividend\n";
    assembly << "    com val_h\n";
    assembly << "    subi val_l, 255            ; +1\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    inc r18\n";
    assembly << "div16_check_divisor:\n";
    assembly << "    sbrs op_h, 7               ; if divisor negative\n";
    assembly << "    rjmp div16_make_num\n";
    assembly << "    com op_l                   ; negate divisor\n";
    assembly << "    com op_h\n";
    assembly << "    subi op_l, 255             ; +1\n";
    assembly << "    sbci op_h, 255\n";
    assembly << "    inc r18\n";
    assembly << "\n";
    assembly << "div16_make_num:\n";
    assembly << "    ; Build 24-bit numerator N = val * 100 into r23:r27:r26\n";
    assembly << "    clr r26\n";
    assembly << "    clr r27\n";
    assembly << "    clr r23\n";
    assembly << "    ldi r19, 100               ; loop counter\n";
    assembly << "div16_mult_loop:\n";
    assembly << "    add r26, val_l             ; N += val (low)\n";
    assembly << "    adc r27, val_h             ; N += val (high)\n";
    assembly << "    adc r23, r1                ; carry into top byte\n";
    assembly << "    dec r19\n";
    assembly << "    brne div16_mult_loop\n";
    assembly << "\n";
    assembly << "    ; Now divide N (24-bit) by op (16-bit) via repeated subtraction\n";
    assembly << "    clr val_l                  ; quotient low\n";
    assembly << "    clr val_h                  ; quotient high\n";
    assembly << "\n";
    assembly << "div16_loop:\n";
    assembly << "    ; if N < op then done (compare r23:r27:r26 with 0:op_h:op_l)\n";
    assembly << "    cp  r26, op_l\n";
    assembly << "    cpc r27, op_h\n";
    assembly << "    cpc r23, r1\n";
    assembly << "    brlo div16_div_done\n";
    assembly << "    ; N -= op\n";
    assembly << "    sub r26, op_l\n";
    assembly << "    sbc r27, op_h\n";
    assembly << "    sbc r23, r1\n";
    assembly << "    ; quotient++\n";
    assembly << "    subi val_l, 255\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    rjmp div16_loop\n";
    assembly << "\n";
    assembly << "div16_div_done:\n";
    assembly << "    ; Restore sign if odd number of negations\n";
    assembly << "    sbrc r18, 0\n";
    assembly << "    rjmp div16_negate\n";
    assembly << "    rjmp div16_done\n";
    assembly << "\n";
    assembly << "div16_negate:\n";
    assembly << "    com val_l\n";
    assembly << "    com val_h\n";
    assembly << "    subi val_l, 255\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "\n";
    assembly << "div16_done:\n";
    assembly << "    clr r1                     ; ABI\n";
    assembly << "    pop r27\n";
    assembly << "    pop r26\n";
    assembly << "    pop r23\n";
    assembly << "    pop r22\n";
    assembly << "    pop r21\n";
    assembly << "    pop r20\n";
    assembly << "    pop r19\n";
    assembly << "    pop r18\n";
    assembly << "    ret\n";

    // MEM handling (SUA VERSÃO ORIGINAL)
    assembly << "handle_mem:\n";
    assembly << "    ; MEM pode ser: armazenar (se tem valor na pilha) ou carregar (se pilha vazia)\n";
    assembly << "    lds temp, stack_ptr\n";
    assembly << "    cpi temp, 0\n";
    assembly << "    breq load_mem              ; Se pilha vazia, carregar MEM\n";
    assembly << "    \n";
    assembly << "    ; ARMAZENAR: pop valor e salvar em MEM\n";
    assembly << "store_mem:\n";
    assembly << "    rcall stack_pop\n";
    assembly << "    sts memory_var, val_l\n";
    assembly << "    sts memory_var+1, val_h\n";
    assembly << "    rcall stack_push           ; Push valor de volta\n";
    assembly << "    ret\n";
    assembly << "    \n";
    assembly << "load_mem:\n";
    assembly << "    ; CARREGAR: ler valor de MEM e fazer push\n";
    assembly << "    lds val_l, memory_var\n";
    assembly << "    lds val_h, memory_var+1\n";
    assembly << "    rcall stack_push\n";
    assembly << "    ret\n\n";
    
    // SUA FUNÇÃO PRINT_HEX_RESULT ORIGINAL
    assembly << "print_hex_result:\n";
    assembly << "    ; Check if negative (MSB set)\n";
    assembly << "    sbrs val_h, 7\n";
    assembly << "    rjmp print_positive\n";
    assembly << "    \n";
    assembly << "    ; Print minus sign for negative\n";
    assembly << "    ldi temp, '-'\n";
    assembly << "    rcall uart_send\n";
    assembly << "    \n";
    assembly << "    ; Convert to positive (two's complement)\n";
    assembly << "    com val_l\n";
    assembly << "    com val_h\n";
    assembly << "    subi val_l, -1\n";
    assembly << "    sbci val_h, -1\n";
    assembly << "    \n";
    assembly << "print_positive:\n";
    assembly << "    ; Print \"0x\" prefix\n";
    assembly << "    ldi temp, '0'\n";
    assembly << "    rcall uart_send\n";
    assembly << "    ldi temp, 'x'\n";
    assembly << "    rcall uart_send\n";
    assembly << "    \n";
    assembly << "    ; Print high byte in hex\n";
    assembly << "    mov temp, val_h\n";
    assembly << "    rcall print_hex_byte\n";
    assembly << "    \n";
    assembly << "    ; Print low byte in hex\n";
    assembly << "    mov temp, val_l\n";
    assembly << "    rcall print_hex_byte\n";
    assembly << "    ret\n\n";
    
    // SUA FUNÇÃO PRINT_HEX_BYTE ORIGINAL
    assembly << "print_hex_byte:\n";
    assembly << "    ; Print one byte in hex (2 digits)\n";
    assembly << "    push temp\n";
    assembly << "    \n";
    assembly << "    ; High nibble\n";
    assembly << "    mov temp2, temp\n";
    assembly << "    swap temp2\n";
    assembly << "    andi temp2, 0x0F\n";
    assembly << "    cpi temp2, 10\n";
    assembly << "    brlo hex_high_digit\n";
    assembly << "    ; Letter A-F\n";
    assembly << "    subi temp2, -55          ; Convert 10-15 to 'A'-'F'\n";
    assembly << "    rjmp hex_high_send\n";
    assembly << "hex_high_digit:\n";
    assembly << "    ; Digit 0-9\n";
    assembly << "    subi temp2, -48          ; Convert 0-9 to '0'-'9'\n";
    assembly << "hex_high_send:\n";
    assembly << "    mov temp, temp2\n";
    assembly << "    rcall uart_send\n";
    assembly << "    \n";
    assembly << "    ; Low nibble\n";
    assembly << "    pop temp\n";
    assembly << "    andi temp, 0x0F\n";
    assembly << "    cpi temp, 10\n";
    assembly << "    brlo hex_low_digit\n";
    assembly << "    ; Letter A-F\n";
    assembly << "    subi temp, -55           ; Convert 10-15 to 'A'-'F'\n";
    assembly << "    rjmp hex_low_send\n";
    assembly << "hex_low_digit:\n";
    assembly << "    ; Digit 0-9\n";
    assembly << "    subi temp, -48           ; Convert 0-9 to '0'-'9'\n";
    assembly << "hex_low_send:\n";
    assembly << "    rcall uart_send\n";
    assembly << "    ret\n\n";
    
    // SUAS FUNÇÕES ORIGINAIS DE PRINT
    assembly << "print_newline:\n";
    assembly << "    ldi temp, '\\n'\n";
    assembly << "    rcall uart_send\n";
    assembly << "    ret\n\n";
    
    assembly << "uart_send:\n";
    assembly << "uart_wait:\n";
    assembly << "    lds temp2, UCSR0A\n";
    assembly << "    sbrs temp2, UDRE0\n";
    assembly << "    rjmp uart_wait\n";
    assembly << "    sts UDR0, temp\n";
    assembly << "    ret\n\n";
    
    codigoAssembly = assembly.str();
}

void lerArquivo(string nomeArquivo, vector<string>& linhas) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        throw invalid_argument("Erro: arquivo nao encontrado ou invalido '" + nomeArquivo + "'");
    }
    
    string linha;
    while (getline(arquivo, linha)) {
        if (!linha.empty()) {
            linhas.push_back(linha);
        }
    }
    
    arquivo.close();
}

void exibirResultados(const vector<float>& resultados) {
    cout << "\n=== RESULTADOS ===" << endl;
    for (size_t i = 0; i < resultados.size(); i++) {
        cout << "Linha " << (i + 1) << ": " << fixed << setprecision(2) << resultados[i] << endl;
    }
    cout << "===================" << endl;
}

void exibirMenu() {
    cout << "Analisador Lexico RPN com Automatos Finitos Deterministicos" << endl;
    cout << "Uso: ./programa <arquivo.txt>" << endl;
}

bool testaParentesesAntes(const string& linha){
    stack<char> parenStack;
    for (size_t i = 0; i < linha.size(); i++) {
        if (linha[i] == '(') {
            parenStack.push('(');
        } else if (linha[i] == ')') {
            if (parenStack.empty()) {
                cerr << "ERRO: fechou sem abrir " << endl;
                return false;
            }
            parenStack.pop();
        }
    }
    if (!parenStack.empty()) {
        cerr << "ERRO: abriu e nao fechou" << endl;
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        exibirMenu();
        return 1;
    }
    
    string nomeArquivo = argv[1];
    
    try {
        vector<string> linhas;
        lerArquivo(nomeArquivo, linhas);
        
        cout << "Processando arquivo: " << nomeArquivo << endl;
        cout << "Total de linhas: " << linhas.size() << endl << endl;
        
        vector<vector<string>> todosTokensValidos; // Para salvar todos os tokens validos
        
        for (size_t i = 0; i < linhas.size(); i++) {
            cout << "Linha " << (i + 1) << ": " << linhas[i] << endl;
            if (testaParentesesAntes(linhas[i])==false){
                cerr << "parenteses desbalanceados na expressao"<<endl<<endl;
                continue;
            }
            try {
                vector<string> tokens;
                parseExpressao(linhas[i], tokens);
                
                cout << "Tokens: ";
                for (const auto& token : tokens) {
                    cout << token << " ";
                }
                cout << endl;
                
                float resultado = executarExpressao(tokens, resultados);
                todosTokensValidos.push_back(tokens);

                cout << "Resultado: " << fixed << setprecision(2) << resultado << endl;
                cout << endl;
                
            } catch (const exception& e) {
                cerr << "ERRO: " << e.what() << endl << endl;
            }
        }
        
        exibirResultados(resultados);
        
        // Gerar Assembly para todas as operações corretas da ultima execução
        string ultimoAssembly;
        gerarAssembly(todosTokensValidos, ultimoAssembly);

        // Salvar todos os tokens validos da ultima execucao - only tokens, no headers
        ofstream tokensFile("tokens.txt");
        for (size_t i = 0; i < todosTokensValidos.size(); i++) {
            for (const auto& token : todosTokensValidos[i]) {
                tokensFile << token << " ";
            }
            tokensFile << "\n";
        }
        tokensFile.close();
        
        // Salvar Assembly da ultima execucao
        ofstream assemblyFile("codigo.S");
        assemblyFile << ultimoAssembly;
        assemblyFile.close();
        
        cout << "\nArquivos gerados:" << endl;
        cout << "- tokens.txt (todos os tokens validos da execucao)" << endl;
        cout << "- codigo.S (codigo Assembly de todas as operacoes corretas da execucao)" << endl;
        
    } catch (const exception& e) {
        cerr << "ERRO FATAL: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}