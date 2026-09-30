# Pasta onde o CMake joga os arquivos temporários
BUILD_DIR = build

# 🍌 CORREÇÃO: O seu executável se chama 'main' e não 'meu_jogo'
TARGET = main

# Regra padrão: Se digitar só 'make', ele apenas configura e compila
all:
	@cmake -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

# Compila e roda o jogo direto!
run: all
	@./$(BUILD_DIR)/$(TARGET)

# Atalho para limpar a bagunça do CMake e recomeçar do zero
clean:
	@rm -rf $(BUILD_DIR)
	@echo "Pasta $(BUILD_DIR) apagada com sucesso!"

.PHONY: all run clean
