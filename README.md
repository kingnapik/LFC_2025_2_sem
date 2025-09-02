# Informação do Grupo
Guilherme knapik - kingnapik

Nome do grupo no Canvas: Grupo 3 - RA1

Disciplina: Linguagem Formal de Compiladores

Pontificia Universidade Católica do Paraná - PUC-PR

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
Versão 42 - disponível dentro da pasta RA1, estão o código C++ assim como os arquivos de teste .txt;

A versão mais atual do código Assembly AVR gerada pelo script C++ também se encontra na pasta;

### Compilando
Tenha os arquivos de teste e executáveis no mesmo diretório.

Compile o arquivo pela linha de comando
```bash
g++ -o analizador main24.cpp
```

### Executando
execute o arquivo passando o arquivo de teste desejado juntamente do executável na linha de comando
```bash
./analizador teste.txt
```

### Formato das operações
Arquivo de teste deve conter operações no formato RPN:
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

A atual versão do Gerador de código assembly tem algumas limitações:

#### 1. **Operações Limitadas**
- **Suporte**: Adição (`+`), Subtração (`-`), Multiplicação (`*`), Divisão Real (`/`)
- **Suporte Limitado**: Divisão Inteira (`/`), Potenciação (`^`) e Memória (`MEM`)
- **Não Suporta**: Módulo (`%`) e Recall de Valor (`RES`)
- As operações de multiplicação, divisão e potenciaçaõ utilizam algoritmos simplificados, dessa maneira podem gerar resultados arrados quando usadas em números muito grandes (ver mais abaixo). 

#### 2. **Fixed-Point Arithmetic**
- Todos os números são transformados em formato 'Fixed-Point' (multiplicados por 100 para tratar as casas decimais), exceto o exponencial da operação de potenciação por motivos de servir como contador de loop, dessa maneira ele é mantido como int.
- ex.: 3.14 se torna 314.

#### 3. **Memória limitada**
- O código Assembly AVR permite apenas uma variavel de memória `MEM`.

#### 4. **Limitações de Integer 16bits**
- Todos os valores são armazenados como 16bit ints (-32,768 to 32,767)
- Após a conversão de ponto fixo, o range efetivo se torna **-327.68 to 327.67**
- **Overflow/underflow se tornam um problema**

### 🔧 **Estrutura do Código Assembly**

- **Comunicação UART** para output (9600 baud)
- **RPN stack** em 64 bytes, 32 valores máximos
- **Output em HEX**
