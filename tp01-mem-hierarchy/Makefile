CC = gcc
CFLAGS = -static -g -std=gnu99 -I./gem5_resources/include
LDFLAGS = -L./gem5_resources/lib -lm5

TARGET = bfs_benchmark.bin

SOURCES = src/main.c src/graph_generator.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
