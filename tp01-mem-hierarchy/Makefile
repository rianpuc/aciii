CC = gcc
CFLAGS = -static -g -std=gnu99 -I./gem5_resources/include
LDFLAGS = -L./gem5_resources/lib -lm5

TARGET_BFS = bfs_benchmark.bin
TARGET_MATRIX = matrix_benchmark.bin

all: $(TARGET_BFS) $(TARGET_MATRIX)

$(TARGET_BFS): src/bfs_benchmark.c src/graph_generator/graph_generator.c
	$(CC) $(CFLAGS) src/bfs_benchmark.c src/graph_generator/graph_generator.c $(LDFLAGS) -o $(TARGET_BFS)

$(TARGET_MATRIX): src/matrix_benchmark.c
	$(CC) $(CFLAGS) src/matrix_benchmark.c $(LDFLAGS) -o $(TARGET_MATRIX)

clean:
	rm -f $(TARGET_BFS) $(TARGET_MATRIX)
