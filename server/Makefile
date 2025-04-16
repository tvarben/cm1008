# Compiler
CC = gcc

# Paths
SRC_DIR = src
LIB_SRC_DIR = ../lib/src
BUILD_DIR = build
INCLUDE_DIR = ../lib/include
RESOURCE_DIR = ../lib/resources

# Output file (same directory as Makefile)
TARGET = gameServer.exe

# Source files
SRCS = \
    $(SRC_DIR)/serverMain.c \
    $(LIB_SRC_DIR)/ship.c \
    $(LIB_SRC_DIR)/sound.c \
    $(LIB_SRC_DIR)/text.c

# Object files in build/
OBJS = $(SRCS:.c=.o)
OBJS := $(patsubst %,$(BUILD_DIR)/%,$(notdir $(OBJS)))

# Flags
CFLAGS = -I$(INCLUDE_DIR) -IC:/msys64/mingw64/include/SDL2 -D_REENTRANT
LDFLAGS = -LC:/msys64/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net

# Default rule
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile main.c
$(BUILD_DIR)/serverMain.o: $(SRC_DIR)/serverMain.c
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile other sources
$(BUILD_DIR)/ship.o: $(LIB_SRC_DIR)/ship.c
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/sound.o: $(LIB_SRC_DIR)/sound.c
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/text.o: $(LIB_SRC_DIR)/text.c
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@if exist $(TARGET) del /q $(TARGET)

.PHONY: all clean
