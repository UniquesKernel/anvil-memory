# Configuration variables
BUILD_DIR = build
PROJECT = project
VERSION ?= 0.1.0

# Installation paths
PREFIX ?= /usr/local
DEV_PREFIX ?= $(HOME)/anvil-dev

# Python configuration
VENV_PATH = venv/bin/activate
PYTHON = python3
ifneq ("$(wildcard $(VENV_PATH))","")
    PYTHON_CMD = source $(VENV_PATH) && python3
else
    PYTHON_CMD = $(PYTHON)
endif

# Build configuration
CMAKE = cmake
CMAKE_FLAGS = -DPROJECT_VERSION=$(VERSION)

# Default target
.PHONY: all
all: build

# Build targets
.PHONY: build build-test
build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_FLAGS) -DBUILD_TESTING=OFF .. && make

build-test:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_FLAGS) -DBUILD_TESTING=ON .. && make

# Installation targets
.PHONY: install install-dev
install: build
	@cd $(BUILD_DIR) && $(CMAKE) --install . --prefix=$(PREFIX)
	@echo "Installed to $(PREFIX)"

install-dev: build
	@cd $(BUILD_DIR) && $(CMAKE) --install . --prefix=$(DEV_PREFIX)
	@echo "Installed to development directory $(DEV_PREFIX)"

# Development environment setup
.PHONY: setup-dev
setup-dev:
	@mkdir -p $(DEV_PREFIX)
	@echo "export CMAKE_PREFIX_PATH=$(DEV_PREFIX):\$$CMAKE_PREFIX_PATH" > setup-dev-env.sh
	@echo "export LD_LIBRARY_PATH=$(DEV_PREFIX)/lib:\$$LD_LIBRARY_PATH" >> setup-dev-env.sh
	@chmod +x setup-dev-env.sh
	@echo "Created development environment setup script"
	@echo "Run 'source setup-dev-env.sh' to set up your environment"

# Package creation
.PHONY: package
package: build
	@cd $(BUILD_DIR) && cpack -G TGZ
	@echo "Package created in $(BUILD_DIR)"

# Testing targets
.PHONY: test test-memcheck
test: build-test
	PYTHONPATH=./tests/python ./venv/bin/python3 -m pytest \
		--hypothesis-show-statistics ./tests/python

test-memcheck: build-test
	. ./venv/bin/activate && \
	PYTHONPATH=./tests/python valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		python -m pytest ./tests/python

# Debug targets
.PHONY: debug memcheck
debug: build
	@gdb ./$(BUILD_DIR)/$(PROJECT)

memcheck: build
	@valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		./$(BUILD_DIR)/$(PROJECT)

# Documentation targets
.PHONY: docs docs-clean
docs:
	@doxygen ./Doxyfile
	@echo "Documentation generated in docs/doxygen/html and docs/doxygen/latex"

docs-clean:
	@rm -rf docs/doxygen
	@echo "Documentation in docs/doxygen has been removed successfully"

# Cleaning targets
.PHONY: clean clean-all
clean:
	@rm -rf $(BUILD_DIR)
	@find ./tests/python/ -type d -name "__pycache__" -exec rm -rf {} +

clean-all: clean docs-clean
	@rm -rf venv setup-dev-env.sh
	@echo "Removed build directory, virtual environment, and environment setup"

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  build        - Build the project"
	@echo "  install      - Install to system (PREFIX=$(PREFIX))"
	@echo "  install-dev  - Install to development directory (DEV_PREFIX=$(DEV_PREFIX))"
	@echo "  setup-dev    - Set up development environment"
	@echo "  package      - Create distribution package"
	@echo "  test         - Run tests"
	@echo "  docs         - Generate documentation"
	@echo "  clean        - Clean build files"
	@echo "  clean-all    - Clean everything"
	@echo ""
	@echo "Configuration:"
	@echo "  VERSION      - Project version (default: $(VERSION))"
	@echo "  PREFIX       - Installation prefix (default: $(PREFIX))"
	@echo "  DEV_PREFIX   - Development installation prefix (default: $(DEV_PREFIX))"
