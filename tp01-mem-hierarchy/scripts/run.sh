#!/bin/bash

GEM5_EXEC="gem5/build/ALL/gem5.opt"
SCRIPT_GEM5="../configs/simulator.py"

BIN_BFS="../bfs_benchmark.bin"
ARGS_BFS="2000 0 42" # 2000 vertices, esparso, seed 42

BIN_MATRIX_NAIVE="../matrix_naive_benchmark.bin"
BIN_MATRIX_OPTIMIZED="../matrix_optimized_benchmark.bin"
ARGS_MATRIX="128 42" # matriz 128x128, seed 42

mkdir -p resultados

run_tests() {
    local WORKLOAD_NAME=$1
    local BIN_PATH=$2
    local BIN_ARGS=$3

    echo " Iniciando testes para: $WORKLOAD_NAME"

    # --- Variando linha de cache (Block Size) ---
    # variaveis constantes: L1 size = 32kB, associacao = 2
    for LINHA in 16 32 64 128
    do
        DIR_OUT="resultados/${WORKLOAD_NAME}_BlockSize_Linha_${LINHA}B"
        echo "[Block Size] = ${LINHA}B..."
        
        $GEM5_EXEC -d $DIR_OUT $SCRIPT_GEM5 \
            --binary $BIN_PATH \
            --args "$BIN_ARGS" \
            --block_size $LINHA \
            --l1d_size 32kB \
            --l1_assoc 2 > /dev/null 2>&1
    done

    # --- Variando tamanho da L1 Cache ---
    # variaveis constantes: Linha (block size) = 64B, associacao = 2
    for CAPACIDADE in "8kB" "16kB" "32kB" "64kB" "128kB"
    do
        DIR_OUT="resultados/${WORKLOAD_NAME}_CacheSize_Capacidade_${CAPACIDADE}"
        echo "[L1 Size] = ${CAPACIDADE}..."
        
        $GEM5_EXEC -d $DIR_OUT $SCRIPT_GEM5 \
            --binary $BIN_PATH \
            --args "$BIN_ARGS" \
            --block_size 64 \
            --l1d_size $CAPACIDADE \
            --l1_assoc 2 > /dev/null 2>&1
    done

    # --- Variando associatividade (ways) ---
    # variaveis constantes: Linha (block size) = 64B, L1 size = 32kB
    for ASSOC in 1 2 4 8
    do
        DIR_OUT="resultados/${WORKLOAD_NAME}_Assoc_${ASSOC}way"
        echo "[Associacao] = ${ASSOC}-way..."
        
        $GEM5_EXEC -d $DIR_OUT $SCRIPT_GEM5 \
            --binary $BIN_PATH \
            --args "$BIN_ARGS" \
            --block_size 64 \
            --l1d_size 32kB \
            --l1_assoc $ASSOC > /dev/null 2>&1
    done
}


echo " Iniciando simulacao - gem5"

run_tests "BFS" "$BIN_BFS" "$ARGS_BFS"

run_tests "MATRIX_NAIVE" "$BIN_MATRIX_NAIVE" "$ARGS_MATRIX"

run_tests "MATRIX_OPTIMIZED" "$BIN_MATRIX_OPTIMIZED" "$ARGS_MATRIX"

echo " Simulacao concluida com sucesso!"
echo " Verifique a pasta 'resultados/' para analisar os stats.txt"
