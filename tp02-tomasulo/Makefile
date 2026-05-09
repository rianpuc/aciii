ifeq ($(OS),Windows_NT)
    RM = rmdir /S /Q
    MKDIR = mkdir
    FIX_PATH = $(subst /,\,$1)
    TARGET = main.exe
else
    RM = rm -rf
    MKDIR = mkdir -p
    FIX_PATH = $1
    TARGET = main
endif

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include

SRC_DIR = src
OBJ_DIR = obj

SRCS = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/*/*.cpp $(SRC_DIR)/*/*/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linkando o executavel..."
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regra para compilar os .cpp em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if not exist "$(call FIX_PATH,$(dir $@))" $(MKDIR) "$(call FIX_PATH,$(dir $@))"
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-$(RM) "$(call FIX_PATH,$(OBJ_DIR))"
	-del /Q "$(TARGET)" 2>nul || rm -f $(TARGET)

# Roda o programa
run: all
	./$(TARGET)