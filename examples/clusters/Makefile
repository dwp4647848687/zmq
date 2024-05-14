# Set Compiler
CC := g++

# Folders
SRC_DIR := src
INC_DIR := include
BUILD_DIR = build
DEP_DIR := $(BUILD_DIR)/deps
DEP_FLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.$(DEP_EXT)

TARGET_DIR := bin
EXE := $(notdir $(patsubst %/,%,$(CURDIR)))
TARGET := $(TARGET_DIR)/$(EXE)

# Sources and Objects
SRC_EXT := cpp
DEP_EXT := d
OBJ_EXT := o
SOURCES := $(shell find $(SRC_DIR) -type f -name *.$(SRC_EXT))
OBJECTS := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SOURCES:.$(SRC_EXT)=.$(OBJ_EXT)))

# Compiler Flags
CFLAGS := -Wall -g -std=c++17
INC := -I $(INC_DIR) -I /Users/danparrott/git/zmq/cppzmq
LIB := -lzmq

$(TARGET): $(OBJECTS) 
	@mkdir -p $(TARGET_DIR)
	@echo "Linking..."
	@echo "  Linking $(TARGET)"
	@($(CC) $^ -o $(TARGET) $(CFLAGS) $(LIB) $(INC))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT) $(DEP_DIR)/%.d | $(DEP_DIR)
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<"
	@($(CC) $(DEP_FLAGS) $(CFLAGS) $(INC) -c -o $@ $<)

# Dependency stuff
$(DEP_DIR): ; @mkdir -p $@
DEP_FILES := $(SOURCES:$(SRC_DIR)/%.$(SRC_EXT)=$(DEP_DIR)/%.d)
$(DEP_FILES):
include $(wildcard $(DEP_FILES))
all: $(TARGET)

clean:
	@if [ -z "$(BUILD_DIR)" ]; then \
		echo "BUILD_DIR is not set"; \
		exit 1; \
	fi

	@echo "Cleaning $(TARGET)..."
	$(RM) $(BUILD_DIR)/*.o
	$(RM) $(BUILD_DIR)/deps/*.$(DEP_EXT)
	$(RM) $(TARGET)

run: $(TARGET)
	@echo " "
	@echo "Running $(EXE) application"
	@echo " "
	@($<)