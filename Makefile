# Curse of the Fractured Veil - Makefile
# ⚠️ IMPORTANT: Avant de compiler, exécute: make setup-raylib

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -fPIC -MMD -MP
LDFLAGS = -lm -lpthread -ldl -lrt -lX11

# Répertoires
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = .
RAYLIB_DIR = $(PWD)/raylib
RAYLIB_SRC = $(RAYLIB_DIR)/src

# Fichiers sources et objets
SRCS = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/**/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
TARGET = $(BIN_DIR)/curse-of-the-fractured-veil

# Inclure les fichiers de dépendances
-include $(DEPS)

# Raylib flags
RAYLIB_CFLAGS = -I$(RAYLIB_SRC)
RAYLIB_LDFLAGS = -L$(RAYLIB_SRC) -lraylib

# === RÈGLES PRINCIPALES ===

all: setup-raylib $(TARGET)

$(TARGET): $(OBJS)
	@echo "🔗 Linking..."
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(RAYLIB_CFLAGS) $^ $(RAYLIB_LDFLAGS) $(LDFLAGS) -o $@
	@echo "✅ Compilation réussie !"
	@echo "🎮 Lance: $(TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "📝 Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(RAYLIB_CFLAGS) -c $< -o $@

# === SETUP & MAINTENANCE ===

setup-raylib:
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		echo "📥 Cloning raylib from GitHub..."; \
		git clone --depth 1 --quiet https://github.com/raysan5/raylib.git $(RAYLIB_DIR) || { \
			echo "❌ Git clone failed. Downloading ZIP instead..."; \
			cd /tmp && curl -sL https://github.com/raysan5/raylib/archive/master.zip -o raylib.zip && \
			unzip -q raylib.zip && mv raylib-master $(RAYLIB_DIR) 2>/dev/null || true; \
		}; \
	fi
	@if [ -d "$(RAYLIB_SRC)" ]; then \
		echo "🔨 Compiling raylib (this may take a while)..."; \
		cd $(RAYLIB_SRC) && make -j4 > /dev/null 2>&1; \
		echo "✅ Raylib ready !"; \
	else \
		echo "❌ Raylib setup failed"; \
		exit 1; \
	fi

clean:
	rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(TARGET)
	@echo "🧹 Build artifacts cleaned"

fclean: clean
	rm -rf $(RAYLIB_DIR)
	@echo "🧹 Everything cleaned (raylib removed)"

run: all
	$(TARGET)

re : fclean all

.PHONY: all clean clean-all run setup-raylib re
