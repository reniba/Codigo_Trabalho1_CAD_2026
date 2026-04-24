# Trabalho 1 - Computação de Alto Desempenho

## 📋 Descrição

Este projeto implementa uma solução para avaliação de estudantes utilizando estatísticas descritivas, desenvolvido como trabalho prático da disciplina de Computação de Alto Desempenho (CAD). 

O programa calcula estatísticas sobre as avaliações de estudantes (como média, mediana, variância e desvio padrão) tanto em versão **sequencial** quanto **paralela** (com OpenMP).

## 👥 Autores

- Renan Correia Monteiro Soares

## 🎯 Objetivos

- Implementar cálculo de estatísticas descritivas
- Comparar performance entre versão sequencial e paralela
- Otimização de código com vetorização (SIMD)
- Análise de desempenho em diferentes configurações

## 📂 Estrutura do Projeto

```
.
├── studentsseq.c                        # Implementação sequencial original
├── studentspar.c                        # Implementação paralela original
├── studentsseqtabs.c                    # Versão sequencial com tabulações
├── studentspartabs.c                    # Versão paralela com tabulações
├── makefile                             # Script de compilação (4 versões)
├── parametros.csv                       # Configurações de teste
├── Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt  # Arquivo de entrada
├── resultados/                          # Diretório com resultados de benchmarks
│   ├── resultados_desempenho_seq.csv
│   ├── resultados_desempenho_par.csv
│   ├── resultados_desempenho_seq3.csv
│   ├── resultados_desempenho_par3.csv
│   ├── resultados_desempenho_seq4.csv
│   ├── resultados_desempenho_par4.csv
│   ├── resultados_desempenho_seq5.csv
│   └── resultados_desempenho_par5.csv
└── README.md                            # Este arquivo
```

## ⚙️ Parâmetros de Entrada

O programa lê um arquivo de configuração com 6 parâmetros:

| Parâmetro | Descrição | Tipo |
|-----------|-----------|------|
| R | Índice inicial do segmento | int |
| C | Índice final do segmento | int |
| A | Posição desejada (0-based) do elemento | int |
| N | Número de avaliações por atividade | int |
| T | Número de threads para paralelização | int |
| SEED | Semente para geração pseudoaleatória | int |

### Exemplo de Arquivo de Entrada

```
200
200
200
200
8
7
```

## 🔧 Compilação

### Compilar todas as versões
```bash
make all
```
Compila as 4 versões: `s`, `p`, `st` e `pt`

### Compilar versões específicas
```bash
make s              # Versão sequencial (original)
make p              # Versão paralela (original)
make st             # Versão sequencial com tabulações
make pt             # Versão paralela com tabulações
```

### Limpar arquivos compilados
```bash
make clean
```

## 🚀 Execução

### Versão Sequencial (Original)
```bash
make run-seq
```
ou
```bash
./s Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt
```

### Versão Paralela (Original)
```bash
make run-par
```
ou
```bash
./p Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt
```

### Versão Sequencial com Tabulações
```bash
make run-seq-tabs
```
ou
```bash
./st Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt
```

### Versão Paralela com Tabulações
```bash
make run-par-tabs
```
ou
```bash
./pt Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt
```

## 📊 Configurações de Compilação

O Makefile utiliza as seguintes flags de compilação:

```makefile
CFLAGS = -fopenmp -w -fopt-info-vec-optimized -O3 -ftree-vectorize -ffast-math -march=native
```

**Explicação das flags:**
- `-fopenmp`: Habilita suporte a OpenMP para paralelização
- `-O3`: Otimização nível 3 (máxima otimização)
- `-ftree-vectorize`: Ativa vetorização SIMD
- `-ffast-math`: Habilita otimizações matemáticas agressivas
- `-march=native`: Otimiza para o processador local

## 📈 Variantes Implementadas

### Versões Básicas
- **studentsseq.c**: Implementação sequencial
- **studentspar.c**: Implementação paralela com OpenMP

### Versões com Tabulações
- **studentsseqtabs.c**: Versão sequencial otimizada
- **studentspartabs.c**: Versão paralela otimizada

## 💡 Algoritmos Principais

### QuickSelect
Algoritmo para encontrar o k-ésimo elemento menor em tempo O(n) em média.

### Cálculo de Estatísticas
- **Média**: Soma de todos os valores dividida pela quantidade
- **Mediana**: Valor central (encontrado via QuickSelect)
- **Variância**: Medida de dispersão dos dados
- **Desvio Padrão**: Raiz quadrada da variância

## 🔀 Variantes Implementadas

O projeto implementa **4 versões diferentes** do programa:

### 1. Versão Original Sequencial (`s`)
- Implementação sequencial básica
- Arquivo: `studentsseq.c`
- Comando: `./s Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt`

### 2. Versão Original Paralela (`p`)
- Implementação paralela com OpenMP
- Arquivo: `studentspar.c`
- Comando: `./p Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt`

### 3. Versão Sequencial com Tabulações (`st`)
- Versão sequencial otimizada com abordagem 2
- Arquivo: `studentsseqtabs.c`
- Comando: `./st Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt`

### 4. Versão Paralela com Tabulações (`pt`)
- Versão paralela otimizada com abordagem 2
- Arquivo: `studentspartabs.c`
- Comando: `./pt Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt`

## ## 📊 Resultados de Benchmark

Os resultados de desempenho estão armazenados em `resultados/`:

```
resultados/
├── resultados_desempenho_seq.csv      # Versão sequencial original
├── resultados_desempenho_par.csv      # Versão paralela original
├── resultados_desempenho_seq3.csv     # Variante 3
├── resultados_desempenho_par3.csv     # Variante 3
├── resultados_desempenho_seq4.csv     # Variante 4
├── resultados_desempenho_par4.csv     # Variante 4
├── resultados_desempenho_seq5.csv     # Variante 5
└── resultados_desempenho_par5.csv     # Variante 5
```

## 🔍 Observações Importantes

⚠️ **Dependências:**
- O programa utiliza a biblioteca de matemática (`math.h`), portanto a compilação deve incluir a flag `-lm`
- O compilador deve suportar OpenMP (`gcc` recomendado)

⚠️ **Arquivo de Entrada:**
- O caminho para o arquivo de entrada é especificado na linha de comando
- O formato esperado é 6 inteiros separados por quebras de linha

## 📝 Exemplo de Uso Completo

```bash
# Compilar todas as versões
make all

# Executar versão sequencial original
./s Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt

# Executar versão paralela original
./p Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt

# Executar versão sequencial com tabulações
./st Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt

# Executar versão paralela com tabulações
./pt Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt

# Limpar arquivos compilados
make clean
```

## 🛠️ Troubleshooting

**Erro: arquivo não encontrado**
```
Erro: Não foi possível abrir o arquivo 'xyz.txt'
```
✓ Certifique-se de que o arquivo de entrada existe no diretório de execução

**Erro: formato inválido**
```
Erro: Arquivo com formato inválido. Esperados 6 valores
```
✓ Verifique se o arquivo tem exatamente 6 inteiros no formato correto

**Erro de compilação**
✓ Certifique-se de que o `gcc` está instalado com suporte a OpenMP
✓ Em sistemas Linux: `sudo apt-get install build-essential libomp-dev`

## 📚 Referências

- [OpenMP Documentation](https://www.openmp.org/)
- [GCC Vectorization Options](https://gcc.gnu.org/projects/tree-ssa/vectorization.html)
- [QuickSelect Algorithm](https://en.wikipedia.org/wiki/Quickselect)

---

**Última atualização:** 2026
