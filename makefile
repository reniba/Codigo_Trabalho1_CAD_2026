# ==========================================
# Variáveis de Compilação
# ==========================================
CC = gcc
CFLAGS = -fopenmp -w -fopt-info-vec-optimized -O3 -ftree-vectorize -ffast-math -march=native
LIBS = -lm

# ==========================================
# Variáveis de Arquivos (Código Original)
# ==========================================
SRC_SEQ = studentsseq.c
SRC_PAR = studentspar.c
EXEC_SEQ = s
EXEC_PAR = p

# ==========================================
# Variáveis de Arquivos (Versão com Tabulação/Abordagem 2)
# ==========================================
SRC_SEQ_TABS = studentsseqtabs.c
SRC_PAR_TABS = studentspartabs.c
EXEC_SEQ_TABS = st
EXEC_PAR_TABS = pt

INPUT = Trab01-AvalEstudantes-ExemploArqEntrada0-v2.txt

# ==========================================
# Regras Principais
# ==========================================

.PHONY: all clean run-seq run-par run-seq-tabs run-par-tabs

# Agora o 'make all' compila as 4 versões
all: $(EXEC_SEQ) $(EXEC_PAR) $(EXEC_SEQ_TABS) $(EXEC_PAR_TABS)

# Compilação Original
$(EXEC_SEQ): $(SRC_SEQ)
	$(CC) $(SRC_SEQ) $(CFLAGS) $(LIBS) -o $(EXEC_SEQ)

$(EXEC_PAR): $(SRC_PAR)
	$(CC) $(SRC_PAR) $(CFLAGS) $(LIBS) -o $(EXEC_PAR)

# Compilação das Versões 'Tabs'
$(EXEC_SEQ_TABS): $(SRC_SEQ_TABS)
	$(CC) $(SRC_SEQ_TABS) $(CFLAGS) $(LIBS) -o $(EXEC_SEQ_TABS)

$(EXEC_PAR_TABS): $(SRC_PAR_TABS)
	$(CC) $(SRC_PAR_TABS) $(CFLAGS) $(LIBS) -o $(EXEC_PAR_TABS)

# ==========================================
# Regras de Execução
# ==========================================

run-seq: $(EXEC_SEQ)
	./$(EXEC_SEQ) $(INPUT)

run-par: $(EXEC_PAR)
	./$(EXEC_PAR) $(INPUT)

run-seq-tabs: $(EXEC_SEQ_TABS)
	./$(EXEC_SEQ_TABS) $(INPUT)

run-par-tabs: $(EXEC_PAR_TABS)
	./$(EXEC_PAR_TABS) $(INPUT)

# ==========================================
# Limpeza
# ==========================================

clean:
	rm -f $(EXEC_SEQ) $(EXEC_PAR) $(EXEC_SEQ_TABS) $(EXEC_PAR_TABS)