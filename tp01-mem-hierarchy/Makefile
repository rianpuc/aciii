CC = gcc
CFLAGS = -static -g -std=gnu99 -I./gem5_resources/include
LDFLAGS = -L./gem5_resources/lib -lm5

TARGET_BFS = bfs_benchmark.bin
TARGET_MATRIX_NAIVE = matrix_naive_benchmark.bin
TARGET_MATRIX_OPTIMIZED = matrix_optimized_benchmark.bin

all: $(TARGET_BFS) $(TARGET_MATRIX_NAIVE) $(TARGET_MATRIX_OPTIMIZED)

$(TARGET_BFS): src/bfs_benchmark.c src/graph_generator/graph_generator.c
	$(CC) $(CFLAGS) src/bfs_benchmark.c src/graph_generator/graph_generator.c $(LDFLAGS) -o $(TARGET_BFS)

$(TARGET_MATRIX_NAIVE): src/matrix_naive_benchmark.c
	$(CC) $(CFLAGS) src/matrix_naive_benchmark.c $(LDFLAGS) -o $(TARGET_MATRIX_NAIVE)

$(TARGET_MATRIX_OPTIMIZED): src/matrix_optimized_benchmark.c
	$(CC) $(CFLAGS) src/matrix_optimized_benchmark.c $(LDFLAGS) -o $(TARGET_MATRIX_OPTIMIZED)

clean:
	rm -f $(TARGET_BFS) $(TARGET_MATRIX)
