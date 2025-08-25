# Informação do Grupo
Guilherme knapik - kingnapik
Nome do grupo no Canvas: Grupo 3 - RA1


# Calculadora de expressões RPN com AFD
A mathematical expression calculator that uses finite state automata (DFA) for lexical analysis, evaluates expressions using RPN (Reverse Polish Notation), and generates Assembly code for Arduino UNO (ATmega328P).

## Features

- **Lexical Analysis**: Uses Deterministic Finite Automaton (DFA) to tokenize mathematical expressions
- **Expression Evaluation**: Converts infix notation to postfix (RPN) and evaluates using stack-based approach
- **Memory Support**: Multiple memory variables (any uppercase letter combination)
- **Assembly Generation**: Generates ATmega328P assembly code for Arduino UNO
- **File Processing**: Reads expressions from text files and processes them line by line
- **Error Handling**: Comprehensive error checking for invalid expressions and syntax

## Supported Operations

- **Arithmetic**: `+`, `-`, `*`, `/`, `%`, `^` (power)
- **Parentheses**: `(`, `)` for grouping expressions
- **Memory Operations**: 
  - `RES(n)`: Access the nth previous result
  - Any uppercase letters (e.g., `A`, `MEM`, `VAR`): Memory variables
- **Numbers**: Integer and decimal numbers (positive and negative)

## Usage

### Compilation
```bash
g++ -o calculator main24.cpp
```

### Running
```bash
./calculator input.txt
```

### Input File Format
Create a text file with mathematical expressions, one per line:
```
2 + 3
5 * (10 - 3)
A = 15
A + 5
RES(1) * 2
```

### Example Output
```
Processando arquivo: input.txt
Total de linhas: 5

Linha 1: 2 + 3
Tokens: 2 + 3
Resultado: 5

Linha 2: 5 * (10 - 3)
Tokens: 5 * ( 10 - 3 )
Resultado: 35

=== RESULTADOS ===
Linha 1: 5
Linha 2: 35
===================

Arquivos gerados:
- tokens.txt (todos os tokens validos da execucao)
- codigo.S (codigo Assembly de todas as operacoes corretas da execucao)
```

## Generated Files

1. **tokens.txt**: Contains all valid tokens from the execution
2. **codigo.S**: Assembly code for Arduino UNO implementing the calculations

## Assembly Code Limitations

The generated Assembly code has several important limitations:

### ⚠️ **Critical Limitations**

#### 1. **Limited Arithmetic Operations**
- **Only supports**: Addition (`+`) and subtraction (`-`)
- **Does NOT support**: Multiplication (`*`), division (`/`), modulo (`%`), or power (`^`) operations
- The Assembly generation code includes multiplication and division functions, but they are **incomplete and may not work correctly**

#### 2. **Fixed-Point Arithmetic Only**
- All numbers are converted to **fixed-point format** (multiplied by 100 for 2 decimal places)
- **No floating-point support** - this significantly limits precision
- Example: `3.14` becomes `314` in the Assembly code

#### 3. **Limited Memory Support**
- Only supports **one memory variable** called `MEM`
- The C++ evaluator supports multiple memory variables (`A`, `VAR`, etc.), but the Assembly only implements `MEM`
- **Memory behavior differs** between C++ evaluation and Assembly generation

#### 4. **16-bit Integer Limitations**
- All values are stored as **16-bit signed integers** (-32,768 to 32,767)
- After fixed-point conversion, the effective range is **-327.68 to 327.67**
- **Overflow/underflow not properly handled**

#### 5. **No RES() Function Support**
- The Assembly code **does not implement** the `RES(n)` function
- Expressions using `RES()` will be parsed by C++ but **ignored in Assembly generation**

#### 6. **Simplified Expression Processing**
- Complex expressions with multiple operations may **not be correctly translated**
- The Assembly generator uses a **simplified RPN conversion** that may miss edge cases

#### 7. **No Error Handling in Assembly**
- **Division by zero**: Minimal error handling (returns error code `0xF1D8`)
- **Stack overflow/underflow**: Not properly handled
- **Invalid operations**: May cause undefined behavior

#### 8. **Hardware-Specific Code**
- Generated code is **specifically for ATmega328P** (Arduino UNO)
- **Not portable** to other microcontrollers without modification
- Uses **specific register names and memory addresses**

### 🔧 **Assembly Code Structure**

The generated Assembly code includes:
- **UART communication** setup for output (9600 baud)
- **RPN stack implementation** (64 bytes, 32 values max)
- **Basic arithmetic functions** (addition, subtraction only)
- **Memory variable support** (single `MEM` variable)
- **Hexadecimal output formatting**

### 📝 **Recommendation**

**The Assembly generation feature should be considered experimental and is primarily useful for:**
- Educational purposes to understand RPN evaluation in Assembly
- Simple arithmetic expressions with only addition and subtraction
- Learning ATmega328P Assembly programming

**For production use:**
- Use only the C++ expression evaluator
- The Assembly code requires significant improvements to handle all supported operations
- Consider the Assembly output as a **starting template** rather than a complete implementation

## Technical Implementation

### Finite State Automaton (DFA) States
- `estadoInicial`: Entry state, dispatches to appropriate states
- `estadoNumero`: Handles numeric tokens (integers and decimals)
- `estadoOperador`: Handles arithmetic operators
- `estadoParenteses`: Handles parentheses
- `estadoComando`: Handles memory variables and commands

### Expression Evaluation Process
1. **Tokenization**: Input string → tokens using DFA
2. **Validation**: Check parentheses balance and expression structure
3. **Infix to Postfix**: Convert using Shunting Yard algorithm
4. **Evaluation**: Stack-based RPN evaluation

### Memory System
- **Global memory map**: `map<string, float> memoria`
- **Results history**: `vector<float> resultados`
- **RES() function**: Access previous results by index

## Error Handling

The program handles various error conditions:
- Invalid characters in input
- Malformed numbers (multiple decimal points)
- Unbalanced parentheses
- Division by zero
- Invalid RES() indices
- File not found errors

## Dependencies

- C++11 or later
- Standard libraries: `<vector>`, `<string>`, `<iostream>`, `<fstream>`, `<stack>`, `<map>`, `<algorithm>`, `<cmath>`
