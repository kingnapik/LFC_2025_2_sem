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

// Nomes dos integrantes do grupo (exemplo):
// [Nome1 Sobrenome1] - [usuario1]
// [Nome2 Sobrenome2] - [usuario2]
// [Nome3 Sobrenome3] - [usuario3]
// [Nome4 Sobrenome4] - [usuario4]
// Nome do grupo no Canvas: [NomeDoGrupo]

// Global variables for memory and previous results
static map<string, float> memoria; // Suporte a múltiplas variáveis de memória
static vector<float> resultados;

// Function prototypes for AFD states
void estadoInicial(const string& linha, size_t& index, vector<string>& tokens);
void estadoNumero(const string& linha, size_t& index, vector<string>& tokens);
void estadoOperador(const string& linha, size_t& index, vector<string>& tokens);
void estadoParenteses(const string& linha, size_t& index, vector<string>& tokens);
void estadoComando(const string& linha, size_t& index, vector<string>& tokens);
bool isValidNumber(const string& numStr);
bool checkParenthesesBalance(const vector<string>& tokens);
bool validateExpressionStructure(const vector<string>& tokens);

// Main functions as specified
vector<string> parseExpressao(string linha, vector<string>& _tokens_);
float executarExpressao(const vector<string>& _tokens_, vector<float>& resultados, float& memoriaLocal);
void gerarAssembly(const vector<vector<string>>& todasExpressoes, string& codigoAssembly);
void lerArquivo(string nomeArquivo, vector<string>& linhas);
void exibirResultados(const vector<float>& resultados);
void exibirMenu();



// AFD implementation using functions
void estadoInicial(const string& linha, size_t& index, vector<string>& tokens) {
    while (index < linha.length()) {
        char c = linha[index];

        if (isspace(c)) {
            index++;
            continue;
        }

        if (isdigit(c) || c == '.') {
            estadoNumero(linha, index, tokens);
        } else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^') {
            estadoOperador(linha, index, tokens);
        } else if (c == '(' || c == ')') {
            estadoParenteses(linha, index, tokens);
        } else if (isalpha(c)) {
            estadoComando(linha, index, tokens);
        } else {
            throw invalid_argument("Caractere invalido: '" + string(1, c) + "'");
        }
    }
}

void estadoNumero(const string& linha, size_t& index, vector<string>& tokens) {
    size_t start = index;
    bool hasDecimal = false;
    
    while (index < linha.length() && (isdigit(linha[index]) || linha[index] == '.')) {
        if (linha[index] == '.') {
            if (hasDecimal) {
                throw invalid_argument("Numero com multiplos pontos decimais");
            }
            hasDecimal = true;
        }
        index++;
    }
    
    string numStr = linha.substr(start, index - start);
    
    if (!isValidNumber(numStr)) {
        throw invalid_argument("Numero malformado: '" + numStr + "'");
    }
    
    tokens.push_back(numStr);
}

void estadoOperador(const string& linha, size_t& index, vector<string>& tokens) {
    tokens.push_back(string(1, linha[index]));
    index++;
}

void estadoParenteses(const string& linha, size_t& index, vector<string>& tokens) {
    tokens.push_back(string(1, linha[index]));
    index++;
}

void estadoComando(const string& linha, size_t& index, vector<string>& tokens) {
    size_t start = index;
    
    while (index < linha.length() && isalpha(linha[index])) {
        index++;
    }
    
    string comando = linha.substr(start, index - start);
    
    // Convert to uppercase for consistency
    transform(comando.begin(), comando.end(), comando.begin(), ::toupper);
    
    if (comando == "RES" || comando.length() >= 1) { // Any uppercase letter combination for memory
        tokens.push_back(comando);
    } else {
        throw invalid_argument("Comando invalido: " + comando);
    }
}

bool isValidNumber(const string& numStr) {
    if (numStr.empty()) return false;
    if (numStr[0] == '.') return false;
    if (numStr.back() == '.') return false;
    
    int dotCount = 0;
    for (char c : numStr) {
        if (c == '.') {
            dotCount++;
            if (dotCount > 1) return false;
        } else if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

bool validateExpressionStructure(const vector<string>& tokens) {
    if (tokens.empty()) return false;

    // Basic structure validation - must have at least one operand
    bool hasOperand = false;
    for (const auto& token : tokens) {
        if (token != "(" && token != ")" && token != "+" && token != "-" && 
            token != "*" && token != "/" && token != "%" && token != "^" && token != "RES") {
            hasOperand = true;
            break;
        }
    }
    
    return hasOperand;
}

vector<string> parseExpressao(string linha, vector<string>& _tokens_) {
    _tokens_.clear();
    size_t index = 0;
    
    try {
        estadoInicial(linha, index, _tokens_);
        
        if (!validateExpressionStructure(_tokens_)) {
            throw invalid_argument("Estrutura da expressao invalida");
        }
        
        return _tokens_;
    } catch (const exception& e) {
        throw;
    }
}

float executarExpressao(const vector<string>& _tokens_, vector<float>& resultados, float& memoriaLocal) {
    stack<float> values;
    
    // Convert infix (with parentheses) to postfix for RPN evaluation
    vector<string> postfix;
    stack<string> operators;
    
    // Simple precedence map
    map<string, int> precedence = {{"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}, {"%", 2}, {"^", 3}};
    
    for (const auto& token : _tokens_) {
        if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1 && isdigit(token[1]))) {
            postfix.push_back(token);
        } else if (token == "(") {
            operators.push(token);
        } else if (token == ")") {
            while (!operators.empty() && operators.top() != "(") {
                postfix.push_back(operators.top());
                operators.pop();
            }
            if (!operators.empty()) operators.pop(); // Remove '('
        } else if (precedence.find(token) != precedence.end()) {
            while (!operators.empty() && operators.top() != "(" && 
                   precedence[operators.top()] >= precedence[token]) {
                postfix.push_back(operators.top());
                operators.pop();
            }
            operators.push(token);
        } else if (token == "RES" || token.length() >= 1) {
            postfix.push_back(token);
        }
    }
    
    while (!operators.empty()) {
        postfix.push_back(operators.top());
        operators.pop();
    }
    
    // Evaluate postfix expression
    for (const auto& token : postfix) {
        if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1 && isdigit(token[1]))) {
            values.push(stof(token));
        } else if (token == "RES") {
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
                // Store in memory
                float valor = values.top();
                values.pop();
                memoria[token] = valor;
                values.push(valor);
            } else {
                // Retrieve from memory
                if (memoria.find(token) != memoria.end()) {
                    values.push(memoria[token]);
                } else {
                    values.push(0.0f); // Uninitialized memory returns 0.0
                }
            }
        } else {
            // Operator
            if (values.size() < 2) {
                throw invalid_argument("Operandos insuficientes para operacao");
            }
            //cout<<values.top()<<endl; //sanity
            float b = values.top(); values.pop();
            //cout<<values.top()<<endl; //sanity
            float a = values.top(); values.pop();
            float result;
            
            if (token == "+") result = a + b;
            else if (token == "-") result = a - b;
            else if (token == "*") result = a * b;
            else if (token == "/") {
                if (b == 0) {
                    throw invalid_argument("Divisao por zero");
                }
                // Check if both are integers for integer division
                if (floor(a) == a && floor(b) == b) {
                    result = static_cast<float>(static_cast<int>(a) / static_cast<int>(b));
                } else {
                    result = a / b;
                }
            }
            else if (token == "%") {
                if (b == 0) {
                    throw invalid_argument("Divisao por zero");
                }
                result = static_cast<float>(static_cast<int>(a) % static_cast<int>(b));
            }
            else if (token == "^") {
                result = pow(a, b);
            }
            else throw invalid_argument("Operador desconhecido: " + token);
            
            values.push(result);
        }
    }
    
    if (values.size() != 1) {
        throw invalid_argument("Expressao malformada - resultado final invalido");
    }
    
    float resultado = values.top();
    resultados.push_back(resultado);
    return resultado;
}

void gerarAssembly(const vector<vector<string>>& todasExpressoes, string& codigoAssembly) {
    stringstream assembly;
    
    assembly << "; Codigo Assembly para Arduino UNO (ATmega328P)\n";
    assembly << "; === CALCULADORA RPN COM SOMA, SUBTRACAO E MEM ===\n";
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
    
    // Processar expressões
    int expr_num = 1;
    for (const auto& tokens : todasExpressoes) {
        assembly << "    ; === EXPRESSAO " << expr_num << " ===\n";
        
        // Reset stack for each expression
        assembly << "    clr temp\n";
        assembly << "    sts stack_ptr, temp\n\n";
        
        // Converter para RPN (soma, subtracao e MEM)
        vector<string> postfix;
        stack<string> ops;
        
        for (const auto& token : tokens) {
            if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
                // Número
                postfix.push_back(token);
            } else if (token == "(") {
                ops.push(token);
            } else if (token == ")") {
                while (!ops.empty() && ops.top() != "(") {
                    postfix.push_back(ops.top());
                    ops.pop();
                }
                if (!ops.empty()) ops.pop(); // Remove o "("
            } else if (token == "+" || token == "-") {
                // Operadores de soma e subtracao (mesma precedencia)
                while (!ops.empty() && ops.top() != "(") {
                    postfix.push_back(ops.top());
                    ops.pop();
                }
                ops.push(token);
            } else if (token == "MEM") {
                // MEM eh tratado como operando
                postfix.push_back(token);
            }
        }
        
        while (!ops.empty()) {
            postfix.push_back(ops.top());
            ops.pop();
        }
        
        // Gerar código para RPN
        for (const auto& token : postfix) {
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
                //todosTokensValidos.push_back(tokens); // Salvar tokens validos
                
                cout << "Tokens: ";
                for (const auto& token : tokens) {
                    cout << token << " ";
                }
                cout << endl;
                
                float memoriaLocal = 0.0f;
                float resultado = executarExpressao(tokens, resultados, memoriaLocal);
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

        // Salvar todos os tokens validos da ultima execucao
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
