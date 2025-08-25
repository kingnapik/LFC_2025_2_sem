# Informação do Grupo
Guilherme knapik - kingnapik

Nome do grupo no Canvas: Grupo 3 - RA1

Disciplina: Linguagem Formal de Compiladores

Prof.: Frank Coelho de Alcantara


# Calculadora de expressões RPN com analisador léxico baseado em AFD
O trabalho consiste em uma calculadora de expressões RPN (Reverse Polish Notation) usando analisador léxico baseado em AFD (Autômato Finito Deterministico).

## Funcionamento
1. O código deve ler um arquivo de texto contendo uma expressão por linha, todas escritas em formato RPN;
2. Analisar as expressões usando o analisador léxico;
3. Filtrar, validar e executar as operações;
4. Gerar código Assembly AVR para execução em plataforma Arduino (microchip: ATmega328).

## Operações aceitas (C++)

- **Operadores**: `+`, `-`, `*`, `/`, `%`, `^`
- **Parenteses**: `(`, `)` com suporte de aninhamento;
- **Operações de Memória**: 
  - `(n RES)`: Acesso do resultado n linhas antes;
  - `(MEM)`: Variaveis de memória;
- **Números**: Inteiros ou float com precisão de 2 casas decimais (positivo e negativo)

## Uso

### compilando
```bash
g++ -o analizador main24.cpp
```

### executando
```bash
./analizador teste.txt
```

### Formato das operações
Arquivo texto contendo operações no formato RPN:
```
(2 3 +)
(4 7 *)
(10.1 3.23 /)
((2 5 *) (20 0.2 +) /)
...
```

### Exemplo de output
```
Processando arquivo: teste.txt
Total de linhas: 1

Linha 1: (10 10 +)
Tokens: ( 10 10 + ) 
Resultado: 20

=== RESULTADOS ===
Linha 1: 20
===================

Arquivos gerados:
- tokens.txt (todos os tokens validos da execucao)
- codigo.S (codigo Assembly de todas as operacoes corretas da execucao)
```

## Arquivos gerados

1. **tokens.txt**: Todos os tokens gerados a partir das expressões válidas.
2. **codigo.S**: Código Assembly AVR

## Limitações ASM

A primeira versão do Gerador de código assembly tem algumas limitações em decorrencia do curto intervalo de tempo disponibilisado para pesquisa e desenvolvimento:

#### 1. **Operações Limitadas**
- **Suporte**: Adição (`+`) e Subtração (`-`)
- **Suporte Limitado**: Multiplicação (`*`), Divisão (`/`) e Memória (`MEM`)
- **Não Suporta**: Módulo (`%`), Potenciação (`^`) e Recall de Valor (`RES`)
- As operações de multiplicação e divisão utilizam algoritmos simplificados, dessa maneira podem gerar resultados arrados quando usadas em números muito grandes (ver mais abaixo).

#### 2. **Fixed-Point Arithmetic**
- Todos os números são transformados em formato 'Fixed-Point' (multiplicados por 100 para tratar as casas decimais)
- ex.: 3.14 se torna 314.

#### 3. **Memória limitada**
- O código Assembly AVR permite apenas uma variavel de memória `MEM`

#### 4. **Limitações de Integer 16bits**
- Todos os valores são armazenados como 16bit ints (-32,768 to 32,767)
- Após a conversão de ponto fixo, o range efetivo se torna **-327.68 to 327.67**
- **Overflow/underflow se tornam um problema**

### 🔧 **Estrutura do Código Assembly**

- **Comunicação UART** para output (9600 baud)
- **RPN stack** em 64 bytes, 32 valores máximos
- **Output em HEX**

## Dependencias

- C++11 or mais recente
- Standard libs: `<vector>`, `<string>`, `<iostream>`, `<fstream>`, `<stack>`, `<map>`, `<algorithm>`, `<cmath>`
