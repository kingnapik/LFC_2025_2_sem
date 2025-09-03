/*
guilherme knapik - kingnapik
grupo 3 RA1
*/
#include <vector>
#include <string>
#include <cctype> //manipular caracter individualmente
#include <stdexcept> //excessoes
#include <iostream>
#include <fstream> //arquivos
#include <stack>
#include <sstream> //string stream
#include <cmath> //matematica
#include <map> //vai mapear as multiplas MEMs
//#include <algorithm>
#include <iomanip> //formatando saida

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
    
    assembly << "; === DEFINICOES DE REGISTRADORES ===\n";
    assembly << "#define temp     r16\n";
    assembly << "#define temp2    r17\n";
    assembly << "#define val_l    r24\n";
    assembly << "#define val_h    r25\n";
    assembly << "#define op_l     r20\n";
    assembly << "#define op_h     r21\n\n";
    
    assembly << "; === AREA DE DADOS ===\n";
    assembly << ".section .bss\n";
    assembly << "rpn_stack:     .space 64  ; Pilha RPN (32 valores de 16-bit)\n";
    assembly << "memory_var:    .space 2   ; Variavel MEM (16-bit)\n";
    assembly << "stack_ptr:     .space 1   ; Ponteiro da pilha RPN\n\n";
    
    assembly << ".section .text\n";
    assembly << ".global main\n\n";
    
    assembly << "main:\n";
    assembly << "    ; Inicializar sistema\n";
    assembly << "    ldi temp, lo8(RAMEND)\n";
    assembly << "    out _SFR_IO_ADDR(SPL), temp\n";
    assembly << "    ldi temp, hi8(RAMEND)\n";
    assembly << "    out _SFR_IO_ADDR(SPH), temp\n\n";
    
    assembly << "    ; Zera stack pointer e memoria\n";
    assembly << "    clr temp\n";
    assembly << "    sts stack_ptr, temp\n";
    assembly << "    sts memory_var, temp      ; MEM = 0\n";
    assembly << "    sts memory_var+1, temp\n\n";
    
    assembly << "    ; Inicial UART (9600 baud @ 16MHz)\n";
    assembly << "    ldi temp, 103             ; 103 = 9600 baud\n";
    assembly << "    sts UBRR0L, temp\n";
    assembly << "    clr temp\n";
    assembly << "    sts UBRR0H, temp\n";
    assembly << "    ldi temp, (1<<TXEN0)\n";  
    assembly << "    sts UCSR0B, temp\n";
    assembly << "    ldi temp, (1<<UCSZ01)|(1<<UCSZ00)  ; 8-bit data\n";
    assembly << "    sts UCSR0C, temp\n\n";
    
    //processa expressoes RPN
    int expr_num = 1;
    for (const auto& tokens : todasExpressoes) {
        assembly << "    ; === EXPRESSAO " << expr_num << " ===\n";
        
        //so reseta se não é a primeira expressão
        if (expr_num > 1) {
            assembly << "    clr temp\n";
            assembly << "    sts stack_ptr, temp\n\n";
        }
        
        //processamento de tokens
        for (size_t i = 0; i < tokens.size(); i++) {
            const auto& token = tokens[i];

            if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
                //se número: converter para fixed-point
                float valor = stof(token);
                int fixed_val;

                if (i + 1 < tokens.size() && tokens[i + 1]=="^"){ //se o valor for seguido por ^ entao e exponente de potenciacao, logo, preciso que seja 2 e nao 200 por conta do fixed ponit
                    fixed_val = (int)valor;//entao, se for exponente, so passa o valor como int e pronto
                    //cout << i << ": " << valor << endl;
                }
                else{
                    fixed_val = (int)(valor * 100);
                    //cout << i <<": " << fixed_val << endl;
                }

                //cout << fixed_val << endl;
                assembly << "    ; Push: " << token << " -> " << fixed_val; //passando num para a pilha para serem usados dpeois nas subrotinas
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
            //call de operacoes, al ogica e quase a mesma para todas elas, difere se vai chamar subrrotina ou nao
            else if (token == "+") {
                assembly << "    ; Adicao\n"; //A B +
                assembly << "    rcall stack_pop    ; op2\n"; //tira da pilha o B
                assembly << "    mov op_l, val_l\n"; //guarda a parte baixa e alta de B
                assembly << "    mov op_h, val_h\n";
                assembly << "    rcall stack_pop    ; op1\n"; //tira da pilha o A
                assembly << "    add val_l, op_l\n"; //soma partes baixas
                assembly << "    adc val_h, op_h\n"; //soma COM CARRY a parte alta
                assembly << "    rcall stack_push\n\n"; //retorna para a pilha o resultado
            }
            else if (token == "-") { //essencialmente a mesma coisa que add mas com sub
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
                assembly << "    rcall stack_pop    ; op2\n"; //tira B da pilha
                assembly << "    mov op_l, val_l\n"; //passa parte alta e baixa de B
                assembly << "    mov op_h, val_h\n";
                assembly << "    rcall stack_pop    ; op1\n"; //tira A
                assembly << "    rcall mul16x16\n"; //chama a subrotina
                assembly << "    rcall stack_push\n\n";
            }
            else if (token == "/") { //mesma logica que *
                assembly << "    ; Divisao\n";
                assembly << "    rcall stack_pop    ; op2\n";
                assembly << "    mov op_l, val_l\n";
                assembly << "    mov op_h, val_h\n";
                assembly << "    rcall stack_pop    ; op1\n";
                assembly << "    rcall div16x16\n";
                assembly << "    rcall stack_push\n\n";
            }
            else if (token == "^") { //mesma logica que *
                assembly << "    ; Potencia (a ^ b)\n";
                assembly << "    rcall stack_pop    ; op2\n";
                assembly << "    mov op_l, val_l\n";
                assembly << "    mov op_h, val_h\n";
                assembly << "    rcall stack_pop    ; op1\n";
                assembly << "    rcall pow16x16_int\n";
                assembly << "    rcall stack_push\n";
            }
            else if (token == "MEM") { //cahma a subrotina MEM
                assembly << "    ; Comando MEM\n";
                assembly << "    rcall handle_mem\n\n";
            }
        }
        
        // resultado e impressão
        assembly << "    ; Resultado final\n";
        assembly << "    rcall stack_pop\n";
        assembly << "    rcall print_hex_result\n";
        assembly << "    rcall print_newline\n\n";
        
        expr_num++;
    }
    
    assembly << "main_loop:\n";
    assembly << "    rjmp main_loop\n\n";
    
    assembly << "; ========== SUBROTINAS ==========\n\n";
    //LIMITE DA PILHA DE 32 NUMEROS por conta da resolucao de 16bits
    assembly << "stack_push:\n";
    assembly << "    ; val_h:val_l -> pilha\n";
    assembly << "    lds ZL, stack_ptr\n"; //passando o ponteiro para ZL e zerando o ZH, fazemos o parzinho Z e enderacamos direto para ele
    assembly << "    clr ZH\n";
    assembly << "    lsl ZL\n"; //shift e necessario por conta de serem 2 bytes independentes tendo que ser um em sequencia do outro.
    assembly << "    rol ZH\n";
    assembly << "    subi ZL, lo8(-(rpn_stack))\n"; //posicao na RAM onde sera salvo
    assembly << "    sbci ZH, hi8(-(rpn_stack))\n";
    assembly << "    st Z+, val_l\n"; //escrve o val baixo e sobe pos de Z
    assembly << "    st Z, val_h\n"; //escreve val alto
    assembly << "    \n";
    assembly << "    ; Incrementar stack pointer\n"; //aumenta o ponteiro e retorna para a pilha
    assembly << "    lds temp, stack_ptr\n";
    assembly << "    inc temp\n";
    assembly << "    sts stack_ptr, temp\n";
    assembly << "    ret\n\n";
    
    assembly << "stack_pop:\n";
    assembly << "    ; pilha -> val_h:val_l\n"; //le a pilha, o processo e o mesmo que o push, so que ao contrario
    assembly << "    lds temp, stack_ptr\n";
    assembly << "    dec temp\n"; //decrementa stack_poiter
    assembly << "    sts stack_ptr, temp\n";
    assembly << "    \n";
    assembly << "    mov ZL, temp\n"; //passa o novo ponteiro para o par Z
    assembly << "    clr ZH\n";
    assembly << "    lsl ZL
    assembly << "    rol ZH\n";
    assembly << "    subi ZL, lo8(-(rpn_stack))\n"; //Z apontando para o elemento que tem que ser tirado
    assembly << "    sbci ZH, hi8(-(rpn_stack))\n";
    assembly << "    ld val_l, Z+\n"; //le o low e hgih
    assembly << "    ld val_h, Z\n";
    assembly << "    ret\n\n";

    //multiplicacao *
    assembly << "mul16x16:\n";
    assembly << "    ; 16x16 multiplication with fixed-point division by 100\n";
    assembly << "    ; Input: val_h:val_l and op_h:op_l\n";
    assembly << "    ; Output: val_h:val_l = (val * op) / 100\n";
    //preserva estado atual dos regs na pilha, isso e preciso pois vamos manipular eles 
    assembly << "    push r18\n";
    assembly << "    push r22\n";
    assembly << "    push r23\n";
    assembly << "    push r26\n";
    assembly << "    push r27\n";
    assembly << "    \n";
    assembly << "    clr r18\n"; //flag serial para negativos setado pra 0
    assembly << "    sbrs val_h, 7\n"; //check bit 7, se tiver set e negativo e nao pula. se 0 e positivo e pula
    assembly << "    rjmp mul16_check_op\n"; //para checar o operador
    //passa para positivo com o complemento do numero
    assembly << "    com val_l\n"; 
    assembly << "    com val_h\n";
    assembly << "    subi val_l, 255\n";
    assembly << "    sbci val_h, 255\n";
    //aumenta contador de negativos em 1
    assembly << "    inc r18\n";
    //faz o check do operador, mesmo processo que acima ^^^
    assembly << "mul16_check_op:\n";
    assembly << "    sbrs op_h, 7\n";
    assembly << "    rjmp mul16_do_mult\n";
    assembly << "    com op_l\n";
    assembly << "    com op_h\n";
    assembly << "    subi op_l, 255\n";
    assembly << "    sbci op_h, 255\n";
    assembly << "    inc r18\n";
    assembly << "    \n";
    assembly << "mul16_do_mult:\n"; //A × B = (AH × 256 + AL) × (BH × 256 + BL)
    assembly << "    ; 16x16 -> 32-bit\n";
    assembly << "    mul val_l, op_l            ; AL * BL\n";
    assembly << "    movw r26, r0\n";
    assembly << "    clr r23\n";
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
    assembly << "    clr val_l\n";
    assembly << "    clr val_h\n";
    assembly << "    clr r1\n";
    assembly << "    \n";
    assembly << "mul16_div100:\n"; //subtracao repetida de 100 para poder restaurar valor para estado antes do fixed point fix. quando restar menos que 100, termina a rotina
    assembly << "    cpi r26, 100\n";
    assembly << "    cpc r27, r1\n";
    assembly << "    cpc r23, r1\n";
    assembly << "    brlo mul16_div_done\n"; //comando branch if lower. se for menor que 100 ta pronto
    assembly << "    \n";
    assembly << "    subi r26, 100\n";
    assembly << "    sbci r27, 0\n";
    assembly << "    sbci r23, 0\n";
    assembly << "    \n";
    assembly << "    subi val_l, 255\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    \n";
    assembly << "    rjmp mul16_div100\n";
    assembly << "    \n";
    assembly << "mul16_div_done:\n"; //terminada a divisao, ver se tem que arredondar. se o que sobrar for mais que 50, arredonda pra cima
    assembly << "    cpi r26, 50\n"; //vale ressaltar que isso esta olhando para os valores depois do segundo casa decimal. ex.: 105550 depois de dividido da 1055 e sobra 50, ai vira 1056, que e 10.5550 sendo arredondado para 10.56
    assembly << "    brlo mul16_no_round\n";
    assembly << "    subi val_l, 255\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    \n";
    assembly << "mul16_no_round:\n"; //retaurando o sinal de menos se tiver
    assembly << "    sbrc r18, 0\n"; //se 0 ambos positivos e resultado positivo, se 1 tem negativo e positivo e resultado e negativo, se 2 ambos negativos e resultado e positivo
    assembly << "    rjmp mul16_negate\n";
    assembly << "    rjmp mul16_done\n";
    assembly << "    \n";
    assembly << "mul16_negate:\n";
    assembly << "    com val_l\n";
    assembly << "    com val_h\n";
    assembly << "    subi val_l, 255\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    \n";
    assembly << "mul16_done:\n";
    assembly << "    clr r1\n"; //restaura r1 para 0 por boas praticas
    assembly << "    pop r27\n"; //retorna regs em ordme inversa
    assembly << "    pop r26\n";
    assembly << "    pop r23\n";
    assembly << "    pop r22\n";
    assembly << "    pop r18\n";
    assembly << "    ret\n\n";

    assembly << "div16x16:\n";
    assembly << "    ; 16x16 division with fixed-point multiplication by 100\n";
    assembly << "    ; Input: val_h:val_l (dividend, scaled x100) and op_h:op_l (divisor, scaled x100)\n";
    assembly << "    ; Output: val_h:val_l = (val * 100) / op\n";
    //mesma coisa, preservando os registradores
    assembly << "    push r18\n";
    assembly << "    push r19\n";
    assembly << "    push r20\n";
    assembly << "    push r21\n";
    assembly << "    push r22\n";
    assembly << "    push r23\n";
    assembly << "    push r26\n";
    assembly << "    push r27\n";
    assembly << "\n";
    assembly << "    clr r1\n"; //garantindo que r1 foi limpo
    assembly << "\n";
    //mesmo gerencia de sinal que na multiplicacao
    assembly << "    clr r18\n";
    assembly << "    sbrs val_h, 7\n";
    assembly << "    rjmp div16_check_divisor\n";
    assembly << "    com val_l\n";
    assembly << "    com val_h\n";
    assembly << "    subi val_l, 255\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    inc r18\n";
    assembly << "div16_check_divisor:\n";
    assembly << "    sbrs op_h, 7\n";
    assembly << "    rjmp div16_make_num\n";
    assembly << "    com op_l\n";
    assembly << "    com op_h\n";
    assembly << "    subi op_l, 255\n";
    assembly << "    sbci op_h, 255\n";
    assembly << "    inc r18\n";
    assembly << "\n";
    
    assembly << "div16_make_num:\n";
    assembly << "    clr r26\n";
    assembly << "    clr r27\n";
    assembly << "    clr r23\n";
    assembly << "    ldi r19, 100\n"; //contador de iteracao
    //necessario multiplicar por 100 por conta da "escalabilidade" do fixed point. ex.: 6 / 2. sem mult extra 600 / 200 = 3, que e 0.03. agora ocm a correcao: (600 * 100) / 200 = 300, que e o 3.00 esperado
    assembly << "div16_mult_loop:\n";
    assembly << "    add r26, val_l\n";
    assembly << "    adc r27, val_h\n";
    assembly << "    adc r23, r1\n";
    assembly << "    dec r19\n";
    assembly << "    brne div16_mult_loop\n";
    assembly << "\n";
    assembly << "    clr val_l\n";
    assembly << "    clr val_h\n";
    assembly << "\n";
    //divisao por subtracao repetidas, quantas vezes N cabe em op
    assembly << "div16_loop:\n";
    assembly << "    cp  r26, op_l\n";
    assembly << "    cpc r27, op_h\n";
    assembly << "    cpc r23, r1\n";
    assembly << "    brlo div16_div_done\n"; //se o N for menos que o op termina
    assembly << "    sub r26, op_l\n";
    assembly << "    sbc r27, op_h\n";
    assembly << "    sbc r23, r1\n";
    assembly << "    subi val_l, 255\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "    rjmp div16_loop\n";
    assembly << "\n";
    assembly << "div16_div_done:\n";
    assembly << "    sbrc r18, 0\n"; //restaura o sinal. se ele estiver em set (1), e negativo, se clear (0 ou 2) e positivo
    assembly << "    rjmp div16_negate\n";
    assembly << "    rjmp div16_done\n";
    assembly << "\n";
    assembly << "div16_negate:\n";
    assembly << "    com val_l\n";
    assembly << "    com val_h\n";
    assembly << "    subi val_l, 255\n";
    assembly << "    sbci val_h, 255\n";
    assembly << "\n";
    assembly << "div16_done:\n"; //clear do r1 e restauro dos registradores
    assembly << "    clr r1\n";
    assembly << "    pop r27\n";
    assembly << "    pop r26\n";
    assembly << "    pop r23\n";
    assembly << "    pop r22\n";
    assembly << "    pop r21\n";
    assembly << "    pop r20\n";
    assembly << "    pop r19\n";
    assembly << "    pop r18\n";
    assembly << "    ret\n";

    // potenicacao ^
    assembly << "pow16x16_int:\n";
    assembly << "    push r28\n";
    assembly << "    push r29\n";
    assembly << "\n";
    assembly << "    mov r28, op_l\n";
    assembly << "\n";
    assembly << "    cpi r28, 0\n"; // exp = 0, retorna 1
    assembly << "    breq pow_ret_one\n";
    assembly << "\n";
    assembly << "    cpi r28, 1\n"; // exp = 1 retrorna a base
    assembly << "    breq pow_done\n"; //antes ele pulava para a retorna base, mas se tornu inutil, entao vai direto para resultado
    assembly << "\n";
    assembly << "    mov r29, val_l\n";
    assembly << "    mov r30, val_h\n";
    assembly << "\n";
    assembly << "    dec r28\n"; //decrementa 1 porque ja temos o A^1 sendo a base
    assembly << "\n";
    assembly << "pow_loop:\n";
    assembly << "    cpi r28, 0\n"; //se der 0 terminou
    assembly << "    breq pow_done\n";
    assembly << "\n";
    // carrega a base e multiplica o resultado pela base, ja que ex.: 2^3 é 2*2*2 entao base 2 * ela mesma 2 da 4. carrega esse e multiplica denovo por 2. deu 8 e encerrou
    assembly << "    mov op_l, r29\n";
    assembly << "    mov op_h, r30\n";
    assembly << "    rcall mul16x16\n";
    assembly << "\n";
    assembly << "    dec r28\n"; //decrementa cada vez que multiplica
    assembly << "    rjmp pow_loop\n";
    assembly << "\n";
    assembly << "pow_ret_one:\n";
    assembly << "    ldi val_l, 100\n"; //retorna 1 em fixed point.
    assembly << "    clr val_h\n";
    assembly << "    rjmp pow_done\n";
// logica velha deixada para artefato
//    assembly << "\n";
//    assembly << "pow_ret_base:\n";
//    assembly << "    ; val_l:val_h already contains base\n";
//    assembly << "    rjmp pow_done\n";
    assembly << "\n";
    assembly << "pow_done:\n";
    assembly << "    pop r29\n";
    assembly << "    pop r28\n";
    assembly << "    ret\n";
    assembly << "\n";

    assembly << "handle_mem:\n";
    assembly << "    lds temp, stack_ptr\n";
    assembly << "    cpi temp, 0\n";
    assembly << "    breq load_mem\n"; //se pilha vasia, carrega MEM
    assembly << "    \n";
    assembly << "store_mem:\n"; //guardar valor
    assembly << "    rcall stack_pop\n";
    assembly << "    sts memory_var, val_l\n";
    assembly << "    sts memory_var+1, val_h\n";
    assembly << "    rcall stack_push\n"; //push devolta o valor
    assembly << "    ret\n";
    assembly << "    \n";
    assembly << "load_mem:\n"; //le valor e faz outro push
    assembly << "    lds val_l, memory_var\n";
    assembly << "    lds val_h, memory_var+1\n";
    assembly << "    rcall stack_push\n";
    assembly << "    ret\n\n";
    
    assembly << "print_hex_result:\n";
    assembly << "    sbrs val_h, 7\n"; //check se positivo ou negativo do mesmo jeito
    assembly << "    rjmp print_positive\n";
    assembly << "    \n";
    assembly << "    ldi temp, '-'\n"; //print - se negativo
    assembly << "    rcall uart_send\n";
    assembly << "    \n";
    assembly << "    com val_l\n"; //converte positivo para poder mostrar 
    assembly << "    com val_h\n";
    assembly << "    subi val_l, -1\n";
    assembly << "    sbci val_h, -1\n";
    assembly << "    \n";
    assembly << "print_positive:\n";
    assembly << "    ldi temp, '0'\n"; //print do 0X de hex
    assembly << "    rcall uart_send\n";
    assembly << "    ldi temp, 'x'\n";
    assembly << "    rcall uart_send\n";
    assembly << "    \n";
    //print high
    assembly << "    mov temp, val_h\n";
    assembly << "    rcall print_hex_byte\n";
    assembly << "    \n";
    //print low
    assembly << "    mov temp, val_l\n";
    assembly << "    rcall print_hex_byte\n";
    assembly << "    ret\n\n";
    
    assembly << "print_hex_byte:\n";
    assembly << "    push temp\n";
    assembly << "    \n";
    //parte alta
    assembly << "    mov temp2, temp\n";
    assembly << "    swap temp2\n";
    assembly << "    andi temp2, 0x0F\n";
    assembly << "    cpi temp2, 10\n";
    assembly << "    brlo hex_high_digit\n";
    assembly << "    subi temp2, -55\n"; //converte a~f para 10~15 formula --> ASCII('A' + n-10) = n + 55
    assembly << "    rjmp hex_high_send\n";
    assembly << "hex_high_digit:\n";
    assembly << "    subi temp2, -48\n"; //0~9 para 0~9 em ascii formula --> ASCII('n') = n + 48
    assembly << "hex_high_send:\n";
    assembly << "    mov temp, temp2\n";
    assembly << "    rcall uart_send\n";
    assembly << "    \n";
    //parte baixa, mesma logica
    assembly << "    pop temp\n";
    assembly << "    andi temp, 0x0F\n";
    assembly << "    cpi temp, 10\n";
    assembly << "    brlo hex_low_digit\n";
    assembly << "    subi temp, -55\n";
    assembly << "    rjmp hex_low_send\n";
    assembly << "hex_low_digit:\n";
    assembly << "    subi temp, -48'\n";
    assembly << "hex_low_send:\n";
    assembly << "    rcall uart_send\n";
    assembly << "    ret\n\n";
    
    assembly << "print_newline:\n"; //pula linha
    assembly << "    ldi temp, '\\n'\n";
    assembly << "    rcall uart_send\n";
    assembly << "    ret\n\n";
    
    assembly << "uart_send:\n"; //manda para o serial
    assembly << "uart_wait:\n";
    assembly << "    lds temp2, UCSR0A\n";
    assembly << "    sbrs temp2, UDRE0\n";
    assembly << "    rjmp uart_wait\n";
    assembly << "    sts UDR0, temp\n";
    assembly << "    ret\n\n";
    
    codigoAssembly = assembly.str();
}

void lerArquivo(string nomeArquivo, vector<string>& linhas) { //le o arquivo, autoexplicativo
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

void exibirResultados(const vector<float>& resultados) { //exibe no prompt, tbm autoexplicativo
    cout << "\n=== RESULTADOS ===" << endl;
    for (size_t i = 0; i < resultados.size(); i++) {
        cout << "Linha " << (i + 1) << ": " << fixed << setprecision(2) << resultados[i] << endl;
    }
    cout << "===================" << endl;
}

void exibirMenu() { //lembrete de como fazer a chamada caso facam errado
    cout << "Analisador Lexico RPN com Automatos Finitos Deterministicos" << endl;
    cout << "Uso: ./programa <arquivo.txt>" << endl;
}

bool testaParentesesAntes(const string& linha){ //tratando parenteses antes de fazer a operacao em si
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
