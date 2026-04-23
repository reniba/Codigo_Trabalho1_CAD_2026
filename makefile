# ==========================================
# Variáveis de Compilação
# ==========================================
CC = gcc
CFLAGS = -fopenmp -w -fopt-info-vec-optimized -O3 -ftree-vectorize -ffast-math -march=native
LIBS = -lm

# ==========================================
# Variáveis de Arquivos
# ==========================================
SRC_SEQ = studentsseq.c
SRC_PAR = studentspar.c
EXEC_SEQ = s
EXEC_PAR = p

INPUT = Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt

# ==========================================
# Regras Principais
# ==========================================

# 'all' é a regra padrão ao digitar apenas 'make'
.PHONY: all clean run-seq run-par

all: $(EXEC_SEQ) $(EXEC_PAR)

# Compilação do código Sequencial
$(EXEC_SEQ): $(SRC_SEQ)
	$(CC) $(SRC_SEQ) $(CFLAGS) $(LIBS) -o $(EXEC_SEQ)

# Compilação do código Paralelo
$(EXEC_PAR): $(SRC_PAR)
	$(CC) $(SRC_PAR) $(CFLAGS) $(LIBS) -o $(EXEC_PAR)

# ==========================================
# Regras de Execução
# ==========================================

# Roda o código sequencial
run-seq: $(EXEC_SEQ)
	./$(EXEC_SEQ) $(INPUT)

# Roda o código paralelo
run-par: $(EXEC_PAR)
	./$(EXEC_PAR) $(INPUT)

# ==========================================
# Limpeza
# ==========================================

# Remove os arquivos compilados
clean:
	rm -f $(EXEC_SEQ) $(EXEC_PAR)