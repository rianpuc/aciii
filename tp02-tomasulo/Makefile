ifeq ($(OS),Windows_NT)
    # No Windows, usamos rmdir para pastas e del para arquivos
    RM_DIR = rmdir /S /Q
    RM_FILE = del /Q
    MKDIR = mkdir
    # Note o (1) entre parênteses
    FIX_PATH = $(subst /,\,$(1))
    TARGET = main.exe
    # Comando para criar diretório se não existir (Windows)
    CREATE_DIR = if not exist "$(call FIX_PATH,$(dir $@))" $(MKDIR) "$(call FIX_PATH,$(dir $@))"
else
    RM_DIR = rm -rf
    RM_FILE = rm -f
    MKDIR = mkdir -p
    FIX_PATH = $(1)
    TARGET = main
    # No Linux/Mac, o mkdir -p já é silencioso se a pasta existir
    CREATE_DIR = $(MKDIR) $(dir $@)
endif

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include

SRC_DIR = src
OBJ_DIR = obj

# Encontra todos os arquivos .cpp
SRCS = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/*/*.cpp $(SRC_DIR)/*/*/*.cpp)
# Mapeia os arquivos .cpp para .o dentro da pasta obj
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linkando o executavel: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^

# ÚNICA REGRA para compilar os .cpp em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(CREATE_DIR)
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Limpando..."
	-$(RM_DIR) "$(call FIX_PATH,$(OBJ_DIR))" 2>nul || true
	-$(RM_FILE) $(TARGET) 2>nul || true

# Roda o programa
run: all
	./$(TARGET)

.PHONY: all clean run
