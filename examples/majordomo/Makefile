# Set Compiler
CC := g++

# Folders
SRC_DIR := src
INC_DIR := include
BUILD_DIR = build
DEP_DIR := $(BUILD_DIR)/deps
DEP_FLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.$(DEP_EXT)

TARGET_DIR := bin
TARGET_CLIENT := $(TARGET_DIR)/client
TARGET_WORKER := $(TARGET_DIR)/worker
TARGET_BROKER := $(TARGET_DIR)/broker

# Sources and Objects
SRC_EXT := cpp
DEP_EXT := d
OBJ_EXT := o
SOURCES := $(shell find $(SRC_DIR) -type f -name *.$(SRC_EXT))
SOURCES_CLIENT := $(filter-out $(SRC_DIR)/mdworkermain.cpp $(SRC_DIR)/mdbrokermain.cpp, $(SOURCES))
SOURCES_WORKER := $(filter-out $(SRC_DIR)/mdclientmain.cpp $(SRC_DIR)/mdbrokermain.cpp, $(SOURCES))
SOURCES_BROKER := $(filter-out $(SRC_DIR)/mdclientmain.cpp $(SRC_DIR)/mdworkermain.cpp, $(SOURCES))

OBJECTS_CLIENT := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SOURCES_CLIENT:.$(SRC_EXT)=.$(OBJ_EXT)))
OBJECTS_WORKER := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SOURCES_WORKER:.$(SRC_EXT)=.$(OBJ_EXT)))
OBJECTS_BROKER := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SOURCES_BROKER:.$(SRC_EXT)=.$(OBJ_EXT)))


# Compiler Flags
CFLAGS := -Wall -g -std=c++17
INC := -I $(INC_DIR) -I /Users/danparrott/git/zmq/cppzmq
LIB := -lzmq
RPATH := -rpath /usr/local/lib

$(TARGET_CLIENT): $(OBJECTS_CLIENT) 
	@mkdir -p $(TARGET_DIR)
	@echo "Linking..."
	@echo "  Linking $(TARGET_CLIENT)"
	@($(CC) $^ -o $(TARGET_CLIENT) $(CFLAGS) $(LIB) $(INC) $(RPATH))

$(TARGET_WORKER): $(OBJECTS_WORKER) 
	@mkdir -p $(TARGET_DIR)
	@echo "Linking..."
	@echo "  Linking $(TARGET_WORKER)"
	@($(CC) $^ -o $(TARGET_WORKER) $(CFLAGS) $(LIB) $(INC) $(RPATH))

$(TARGET_BROKER): $(OBJECTS_BROKER)
	@mkdir -p $(TARGET_DIR)
	@echo "Linking..."
	@echo "  Linking $(TARGET_BROKER)"
	@($(CC) $^ -o $(TARGET_BROKER) $(CFLAGS) $(LIB) $(INC) $(RPATH))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT) $(DEP_DIR)/%.d | $(DEP_DIR)
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<"
	@($(CC) $(DEP_FLAGS) $(CFLAGS) $(INC) -c -o $@ $<)

# Dependency stuff
$(DEP_DIR): ; @mkdir -p $@
DEP_FILES := $(SOURCES:$(SRC_DIR)/%.$(SRC_EXT)=$(DEP_DIR)/%.d)
$(DEP_FILES):
include $(wildcard $(DEP_FILES))

client: $(TARGET_CLIENT)

worker: $(TARGET_WORKER)

broker: $(TARGET_BROKER)

clean:
	@if [ -z "$(BUILD_DIR)" ]; then \
		echo "BUILD_DIR is not set"; \
		exit 1; \
	fi

	@echo "Cleaning..."
	$(RM) $(BUILD_DIR)/*.o
	$(RM) $(BUILD_DIR)/deps/*.$(DEP_EXT)
	$(RM) $(TARGET_CLIENT)
	$(RM) $(TARGET_WORKER)
	$(RM) $(TARGET_BROKER)