# ⚙️ Simulador Tomasulo Superescalar

Um simulador de processador superescalar com execução Fora de Ordem (Out-of-Order Execution) implementando o **Algoritmo de Tomasulo**, desenvolvido em C++.

Este projeto foi construído para explorar os limites do Paralelismo em Nível de Instrução (ILP), lidando com hazards estruturais e de dados (RAW, WAW, WAR) através de renomeação de registradores, desambiguação avançada de memória e controle rigoroso de barramento.

## 🚀 Principais Diferenciais (Arquitetura Avançada)

Além do fluxo clássico de Tomasulo (Issue, Execute, Write Result, Commit), este simulador implementa características de hardwares reais de alta performance:

* **Store-to-Load Forwarding (Bypass de Memória):** Loads podem "roubar" dados diretamente de Stores que estão aguardando no Reorder Buffer (ROB) destinados ao mesmo endereço, economizando ciclos de acesso à RAM.
* **Desambiguação Dinâmica de Memória:** O simulador trata o *Blind Address Trap*. Se um `LW` sabe seu endereço, mas um `SW` mais antigo no ROB ainda está calculando o seu, o `LW` sofre um stall conservador para evitar leitura suja de memória, garantindo coerência absoluta.
* **Limite de Largura de Banda do CDB & Passe-Livre:** O Common Data Bus (CDB) possui um parâmetro configurável (`cdbWidth`). Múltiplas Unidades Funcionais terminam no mesmo ciclo e competem pelo barramento em estado de *Stall*. Instruções `SW`, por não gerarem dados para outras RS, ganham "Passe-Livre", atualizando o ROB sem consumir banda da rede de interconexão.
* **Escalonamento Anti-Starvation (Oldest-First):** Para evitar que instruções novas roubem ALUs de instruções antigas quando múltiplas Reservation Stations acordam no mesmo ciclo, o despachante (Dispatcher) aplica uma ordenação baseada na idade da instrução no ROB (`destROB`).
* **Exceções Precisas e Speculative Execution (Poison Bit):** Erros de execução fora de ordem (ex: Divisão por Zero) não derrubam o processador imediatamente. A ALU injeta um "Poison Bit" no fluxo de dados, permitindo a execução especulativa de instruções dependentes. O pipeline só sofre *flush* (a exceção só é lançada) quando a instrução defeituosa atinge o topo do ROB, garantindo a integridade do estado arquitetural.
* **Hardware Memory Management Unit (MMU):** Proteção de memória via *Wrap-around* (`std::abs(addr) % MEMORY_SIZE`). Evita *Segmentation Faults* quando instruções especulativas calculam endereços gigantes, espelhando-os silenciosamente para espaços físicos seguros.

## 🧠 Estruturas de Dados e Implementação

O core do simulador foi desenhado otimizando a manipulação temporal de instruções:

* **Reorder Buffer (ROB):** Implementado como um `std::deque<ReorderBufferEntry>`. Garante despacho (Issue) estritamente em ordem, execução fora de ordem e Commit estritamente em ordem. A varredura de *Aliasing* de memória utiliza iteradores reversos para garantir a precedência cronológica correta na fila.
* **Reservation Stations:** Vetores separados para `ADD/SUB`, `MUL/DIV` e `LW/SW`. Monitoram operandos prontos (`Vj`, `Vk`) e dependências (`Qj`, `Qk`).
* **Register Alias Table (RAT):** Gerencia a renomeação de registradores (32 registradores arquiteturais mapeados dinamicamente para tags do ROB).
* **Functional Units (ALUs):** Latências customizáveis via construtor. Trabalham de forma autônoma processando *ticks* de clock independentes.

## 📂 Estrutura do Projeto

```text
├── include/
│   ├── core/         # Componentes centrais (Simulador, Instruções)
│   ├── hardware/     # Estruturas físicas (ALUs, ROB, CDB, RS, RAT)
│   └── utils/        # Ferramentas auxiliares (Parser, Logger, Exceptions)
├── src/
│   ├── core/
│   ├── utils/
│   └── main.cpp      # Ponto de entrada e CLI
├── tests/            # Arquivos .txt com códigos Assembly para teste
└── Makefile          # Script de automação de compilação
```
## 🛠️ Como Compilar e Rodar

### Pré-requisitos
- Compilador C++ com suporte ao padrão C++17 (g++ ou clang).
- Utilitário Make instalado.

### Compilação
Na raiz do diretório do projeto, execute o comando make para compilar o código fonte de acordo com as regras do `Makefile`:

```bash
make all
```
### Execução
Após a compilação, o executável main será gerado. Para rodar:
```bash
./main
```
## 🎮 O Menu de Injeção de Estado (Pré-Simulação)
Antes de iniciar o processamento do arquivo `.txt`, o simulador oferece uma interface (CLI) para injetar estados. É essencial para testar casos extremos isolados:
- Modificar a latência de qualquer ALU (ADD, MUL, LS).
- Modificar o número físico de Estações de Reserva e ALUs disponíveis.
- Forçar valores iniciais no Banco de Registradores (R0 a R31).
- Forçar valores iniciais na Memória RAM Física (Endereços 0 a 1023).

## 📁 Formato do Arquivo de Instruções (.txt)
O `Parser` do simulador suporta instruções separadas por espaço ou vírgula, ignorando automaticamente qualquer comentário com `//`. Coloque os arquivos de teste dentro da pasta `tests/`.

**Opcodes Suportados:** `ADD`, `SUB`, `MUL`, `DIV`, `LW`, `SW`

**Exemplo de código** (`tests/boss_final.txt`):

```Snippet de código
// Teste de Estresse Extremo: Caos do Silicio Superescalar
MUL R1, R2, R3      // Inst 1: A Âncora. Demora X ciclos e trava o topo do ROB.
SUB R1, R4, R5      // Inst 2: WAW (Falsa dependencia). A RAT renomeia R1 e deixa executar!
ADD R6, R1, R1      // Inst 3: RAW na Inst 2. Vai disputar a ALU com outras instrucoes.
SW  R6, 100(R0)     // Inst 4: Store sem usar o CDB.
LW  R7, 100(R0)     // Inst 5: BYPASS DE MEMORIA! Rouba o dado da Inst 4 do ROB.
DIV R11, R12, R10   // Inst 6: A BOMBA! Divisao por zero levanta flag de Poison Bit.
ADD R13, R11, R11   // Inst 7: Executa com o dado envenenado antes da maquina travar no Commit.
```

## 👥 Membros do Grupo
- Enzo Castro Figueredo - [Github](https://github.com/oEnzoCastro)
- Gustavo Pereira Rodrigues - [Github](https://github.com/GustavoPR35)
- Rafael Soares Iannini - [Github](https://github.com/rafaeliannini)
- Rian Eduardo Dias Lopes - [Github](https://github.com/rianpuc)
- Samuel Correia Pedrosa - [Github](https://github.com/samulkil)