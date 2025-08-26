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

// Global variables for memory and previous results
static map<string, float> memoria; // Suporte a múltiplas variáveis de memória
static vector<float> resultados;

// Estados AFD - Updated to handle single tokens
void estadoInicial(const string& token, vector<string>& tokens);
void estadoNumero(const string& token, vector<string>& tokens);
void estadoOperador(const string& token, vector<string>& tokens);
void estadoParenteses(const string& token, vector<string>& tokens);
void estadoComando(const string& token, vector<string>& tokens);

// func da main
vector<string> parseExpressao(string linha, vector<string>& _tokens_);
float executarExpressao(const vector<string>& _tokens_, vector<float>& resultados);
void gerarAssembly(const vector<vector<string>>& todasExpressoes, string& codigoAssembly);
void lerArquivo(string nomeArquivo, vector<string>& linhas);
void exibirResultados(const vector<float>& resultados);
void exibirMenu();

// Helper function for 16-bit precision
float to16BitFloat(float value) {
    return round(value * 100.0f) / 100.0f;
}

void estadoInicial(const string& token, vector<string>& tokens) {
    
    if (token.empty()) return;
    
    char firstChar = token[0];
    
    if (isdigit(firstChar) || firstChar == '.') {
        estadoNumero(token, tokens);
    } else if (firstChar == '+' || firstChar == '-' || firstChar == '*' || 
               firstChar == '/' || firstChar == '%' || firstChar == '^') {
        estadoOperador(token, tokens);
    } else if (firstChar == '(' || firstChar == ')') {
        estadoParenteses(token, tokens);
    } else if (isalpha(firstChar)) {
        estadoComando(token, tokens);
    } else {
        throw invalid_argument("Token invalido: '" + token + "'");
    }
}

void estadoNumero(const string& token, vector<string>& tokens) {
    bool hasDecimal = false;
    
    // Validate entire token is a number
    for (char c : token) {
        if (c == '.') {
            if (hasDecimal) {
                throw invalid_argument("Numero com multiplos pontos decimais");
            }
            hasDecimal = true;
        } else if (!isdigit(c)) {
            throw invalid_argument("Numero malformado: '" + token + "'");
        }
    }
    
    if (token[0] == '.' || token.back() == '.') {
        throw invalid_argument("Numero malformado: '" + token + "'");
    }
    
    tokens.push_back(token);
}

void estadoOperador(const string& token, vector<string>& tokens) {
    if (token.length() != 1) {
        throw invalid_argument("Operador invalido: '" + token + "'");
    }
    
    char op = token[0];
    if (op != '+' && op != '-' && op != '*' && op != '/' && op != '%' && op != '^') {
        throw invalid_argument("Operador invalido: '" + token + "'");
    }
    
    tokens.push_back(token);
}

void estadoParenteses(const string& token, vector<string>& tokens) {
    if (token.length() != 1) {
        throw invalid_argument("Parenteses invalido: '" + token + "'");
    }
    
    char paren = token[0];
    if (paren != '(' && paren != ')') {
        throw invalid_argument("Parenteses invalido: '" + token + "'");
    }
    
    tokens.push_back(token);
}

void estadoComando(const string& token, vector<string>& tokens) {
    for (char c : token) {
        if (!isalpha(c)) {
            throw invalid_argument("Comando invalido: " + token);
        }
        if (islower(c)) {
            throw invalid_argument("Comando invalido: " + token);
        }
    }
    
    string comando = token;
    transform(comando.begin(), comando.end(), comando.begin(), ::toupper);
    
    if (comando == "RES" || comando.length() >= 1) {
        tokens.push_back(comando);
    } else {
        throw invalid_argument("Comando invalido: " + comando);
    }
}

vector<string> parseExpressao(string linha, vector<string>& _tokens_) {
    _tokens_.clear();
    
    try {
        // STEP 1: Custom tokenization that separates parentheses
        vector<string> rawTokens;
        string current = "";
        
        for (size_t i = 0; i < linha.length(); i++) {
            char c = linha[i];
            
            if (c == ' ' || c == '\t') {
                // Space - finish current token
                if (!current.empty()) {
                    rawTokens.push_back(current);
                    current = "";
                }
            } else if (c == '(' || c == ')') {
                // Parenthesis - finish current token and add parenthesis as separate token
                if (!current.empty()) {
                    rawTokens.push_back(current);
                    current = "";
                }
                rawTokens.push_back(string(1, c));
            } else {
                // Regular character - add to current token
                current += c;
            }
        }
        
        // Add final token if exists
        if (!current.empty()) {
            rawTokens.push_back(current);
        }
        
        if (rawTokens.empty()) {
            throw invalid_argument("Expressao vazia");
        }
        
        // STEP 2: Pass each token through AFD for validation
        for (const auto& tok : rawTokens) {
            estadoInicial(tok, _tokens_);
        }
        
        return _tokens_;
    } catch (const exception& e) {
        throw;
    }
}

float executarExpressao(const vector<string>& _tokens_, vector<float>& resultados) {
    stack<float> values;
    
    // Direct RPN evaluation - no infix to postfix conversion
    for (const auto& token : _tokens_) {
        if (token == "(" || token == ")") {
            // Skip parentheses in RPN - they're just grouping markers
            continue;
        } else if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1 && isdigit(token[1]))) {
            // Number - push to stack
            values.push(stof(token));
        } else if (token == "RES") {
            // RES expects the index on the stack
            if (values.empty()) {
                throw invalid_argument("RES precisa de um parametro");
            }
            
            int n = static_cast<int>(values.top());
            values.pop();
            
            if (n <= 0 || n > resultados.size()) {
                throw invalid_argument("Indice RES invalido: " + to_string(n));
            }
            
            values.push(resultados[resultados.size() - n]);
        } else if (token != "RES" && isalpha(token[0])) {
            // Memory operation
            if (!values.empty()) {
                // Store in memory with 16-bit precision
                float valor = to16BitFloat(values.top());
                values.pop();
                memoria[token] = valor;
                values.push(valor);
            } else {
                // Retrieve from memory
                if (memoria.find(token) != memoria.end()) {
                    values.push(memoria[token]);
                } else {
                    values.push(0.0f);
                }
            }
        } else {
            // Operator - pop two operands
            if (values.size() < 2) {
                throw invalid_argument("Operandos insuficientes para operacao");
            }
            
            float b = values.top(); values.pop();
            float a = values.top(); values.pop();
            float result;
            
            if (token == "+") result = to16BitFloat(a + b);
            else if (token == "-") result = to16BitFloat(a - b);
            else if (token == "*") result = to16BitFloat(a * b);
            else if (token == "/") {
                if (b == 0) {
                    throw invalid_argument("Divisao por zero");
                }
                if (floor(a) == a && floor(b) == b) {
                    result = to16BitFloat(static_cast<float>(static_cast<int>(a) / static_cast<int>(b)));
                } else {
                    result = to16BitFloat(a / b);
                }
            }
            else if (token == "%") {
                if (b == 0) {
                    throw invalid_argument("Divisao por zero");
                }
                result = to16BitFloat(static_cast<float>(static_cast<int>(a) % static_cast<int>(b)));
            }
            else if (token == "^") {
                result = to16BitFloat(pow(a, b));
            }
            else throw invalid_argument("Operador desconhecido: " + token);
            
            values.push(result);
        }
    }
    
    if (values.size() != 1) {
        throw invalid_argument("Expressao RPN malformada - resultado final invalido");
    }
    
    float resultado = to16BitFloat(values.top());
    resultados.push_back(resultado);
    return resultado;
}

void gerarAssembly(const vector<vector<string>>& todasExpressoes, string& codigoAssembly) {
    stringstream assembly;
    
    assembly << "; Codigo Assembly para Arduino UNO (ATmega328P)\n";
    assembly << "; === CALCULADORA RPN ===\n";
    assembly << "; Fixed-point: multiplicamos por 100 (2 casas decimais)\n\n";
    
    assembly << "#include <avr/io.h>\n\n";
    
    // Definições de registradores
    assembly << "; === DEFINICOES DE REGISTRADORES ===\n";
    assembly << "#define temp     r16\n";
    assembly << "#define temp2    r17\n";
    assembly << "#define val_l    r24\n";
    assembly << "#define val_h    r25\n";
    assembly << "#define op_l     r20\n";
    assembly << "#define op_h     r21\n\n";
    
    // Área de dados
    assembly << "; === AREA DE DADOS ===\n";
    assembly << ".section .bss\n";
    assembly << "rpn_stack:     .space 64  ; Pilha RPN (32 valores de 16-bit)\n";
    assembly << "memory_var:    .space 2   ; Variavel MEM (16-bit)\n";
    assembly << "stack_ptr:     .space 1   ; Ponteiro da pilha RPN\n\n";
    
    assembly << ".section .text\n";
    assembly << ".global main\n\n";
    
    // Função principal
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
    
    // Processar expressões RPN
    int expr_num = 1;
    for (const auto& tokens : todasExpressoes) {
        assembly << "    ; === EXPRESSAO " << expr_num << " ===\n";
        
        // Reset stack for each expression
        assembly << "    clr temp\n";
        assembly << "    sts stack_ptr, temp\n\n";
        
        // Process RPN tokens directly (no infix conversion needed)
        for (const auto& token : tokens) {
            if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
                // Número: converter para fixed-point
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
        
        // Obter resultado final e exibir
        assembly << "    ; Resultado final\n";
        assembly << "    rcall stack_pop\n";
        assembly << "    rcall print_hex_result\n";
        assembly << "    rcall print_newline\n\n";
        
        expr_num++;
    }
    
    assembly << "main_loop:\n";
    assembly << "    rjmp main_loop\n\n";
    
    // ===== SUBROTINAS =====
    assembly << "; ========== SUBROTINAS ==========\n\n";
    
    // Push na pilha RPN
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
    
    // Multiplicacao 16-bit com correcao de escala (divide por 100)
    assembly << "mul16x16:\n";
    assembly << "    mov r18, val_l\n";
    assembly << "    mov r19, val_h\n";
    assembly << "    mov r20, op_l\n";
    assembly << "    mov r21, op_h\n";
    assembly << "    clr r22\n";
    assembly << "    clr r23\n";
    assembly << "    clr r24\n";
    assembly << "    clr r25\n";
    assembly << "    mul r18, r20\n";
    assembly << "    mov r22, r0\n";
    assembly << "    mov r23, r1\n";
    assembly << "    mul r18, r21\n";
    assembly << "    add r23, r0\n";
    assembly << "    adc r24, r1\n";
    assembly << "    adc r25, r1\n";          // r1 is zero
    assembly << "    mul r19, r20\n";
    assembly << "    add r23, r0\n";
    assembly << "    adc r24, r1\n";
    assembly << "    adc r25, r1\n";
    assembly << "    mul r19, r21\n";
    assembly << "    add r24, r0\n";
    assembly << "    adc r25, r1\n";
    assembly << "    clr r1\n";
    assembly << "    mov r18, r22\n";
    assembly << "    mov r19, r23\n";
    assembly << "    mov r20, r24\n";
    assembly << "    mov r21, r25\n";
    assembly << "    clr r24\n";
    assembly << "    clr r25\n";
    assembly << "mul_div_loop:\n";
    assembly << "    cpi  r18, 100\n";
    assembly << "    cpc  r19, r1\n";         // compare to 0 with carry
    assembly << "    cpc  r20, r1\n";
    assembly << "    cpc  r21, r1\n";
    assembly << "    brlo mul_done\n";
    assembly << "    subi r18, 100\n";
    assembly << "    sbci r19, 0\n";
    assembly << "    sbci r20, 0\n";
    assembly << "    sbci r21, 0\n";
    assembly << "    adiw r24, 1\n";
    assembly << "    rjmp mul_div_loop\n";
    assembly << "mul_done:\n";
    assembly << "    mov val_l, r24\n";
    assembly << "    mov val_h, r25\n";
    assembly << "    ret\n\n";

    // Divisao 16-bit com correcao de escala (multiplica dividendo por 100)
    assembly << "div16x16:\n";
    assembly << "    tst op_l\n";
    assembly << "    brne div_continue\n";
    assembly << "    tst op_h\n";
    assembly << "    breq div_error\n";
    assembly << "div_continue:\n";
    assembly << "    mov r18, val_l\n";
    assembly << "    mov r19, val_h\n";
    assembly << "    mov r20, op_l\n";
    assembly << "    mov r21, op_h\n";
    assembly << "    ldi r22, 100\n";         // = lo8(100)
    assembly << "    clr r23\n";              // = hi8(100)
    assembly << "    mul r18, r22\n";         // (val * 100) low part
    assembly << "    mov r24, r0\n";
    assembly << "    mov r25, r1\n";
    assembly << "    mul r19, r22\n";         // add high*100 into high byte
    assembly << "    add r25, r0\n";
    assembly << "    mov r18, r24\n";         // dividend_scaled low
    assembly << "    mov r19, r25\n";         // dividend_scaled high
    assembly << "    mov r20, op_l\n";        // divisor low
    assembly << "    mov r21, op_h\n";        // divisor high
    assembly << "    clr r24\n";              // quotient low
    assembly << "    clr r25\n";              // quotient high
    assembly << "div_loop:\n";
    assembly << "    cp  r18, r20\n";
    assembly << "    cpc r19, r21\n";
    assembly << "    brlo div_done\n";
    assembly << "    sub r18, r20\n";
    assembly << "    sbc r19, r21\n";
    assembly << "    adiw r24, 1\n";
    assembly << "    rjmp div_loop\n";
    assembly << "div_done:\n";
    assembly << "    mov val_l, r24\n";
    assembly << "    mov val_h, r25\n";
    assembly << "    ret\n";
    assembly << "div_error:\n";
    assembly << "    ldi val_l, 0xF1\n";
    assembly << "    ldi val_h, 0xD8\n";
    assembly << "    ret\n\n";

    // Handle MEM command
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
    assembly << "    rcall stack_push           ; Push valor de volta (MEM retorna o valor armazenado)\n";
    assembly << "    ret\n";
    assembly << "    \n";
    assembly << "load_mem:\n";
    assembly << "    ; CARREGAR: ler valor de MEM e fazer push\n";
    assembly << "    lds val_l, memory_var\n";
    assembly << "    lds val_h, memory_var+1\n";
    assembly << "    rcall stack_push\n";
    assembly << "    ret\n\n";
    
    // Pop da pilha RPN
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
    
    // Print resultado em HEX
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
    
    // Print byte em hexadecimal
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
    
    // Print newline
    assembly << "print_newline:\n";
    assembly << "    ldi temp, '\\n'\n";
    assembly << "    rcall uart_send\n";
    assembly << "    ret\n\n";
    
    // Send via UART
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