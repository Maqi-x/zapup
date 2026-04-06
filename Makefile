CC ?= cc
AR ?= ar
BUILD ?= release

ifeq ($(OS),Windows_NT)
	PLATFORM := windows
else
	PLATFORM := posix
endif

# Recursive wildcard function
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# directories
SRC_DIR     := src
INCLUDE_DIR := include

# output directories for binaries
OUT_DIR     ?= out/$(BUILD)
BUILD_DIR   ?= build/$(BUILD)
BIN_DIR     ?= $(OUT_DIR)

# object and dependency file root directories
OBJ_ROOT_DIR ?= $(BUILD_DIR)/$(BUILD)/obj
DEP_ROOT_DIR ?= $(BUILD_DIR)/$(BUILD)/dep

# outputs
TARGET      := $(BIN_DIR)/zapup

ifeq ($(PLATFORM),windows)
	EXE_EXT := .exe
	FIXPATH = $(subst /,\,$1)
	RM = del /Q /S
	RMDIR = rmdir /Q /S
	MKDIR = if not exist "$(call FIXPATH,$1)" mkdir "$(call FIXPATH,$1)"
	PREFIX ?= C:/zapup
	BINDIR ?= $(PREFIX)
else
	EXE_EXT := .elf
	FIXPATH = $1
	RM = rm -f
	RMDIR = rm -rf
	MKDIR = mkdir -p "$1"
	PREFIX ?= /usr/local
	BINDIR ?= $(PREFIX)/bin
endif

TARGET   := $(TARGET)$(EXE_EXT)

# flags
CSTD       := -std=c11 -D_GNU_SOURCE
WARNINGS   := -Wall -Wextra

COMMON_CFLAGS := $(CSTD) $(WARNINGS) -I$(INCLUDE_DIR)

ifeq ($(BUILD),debug)
	CFLAGS := $(COMMON_CFLAGS) -O0 -g -DEL_DEBUG -fsanitize=address,undefined
	LDFLAGS := -fsanitize=address,undefined
else ifeq ($(BUILD),release)
	CFLAGS := $(COMMON_CFLAGS) -O3 -DNDEBUG -DZAPUP_RELEASE
	LDFLAGS :=
else
	$(error Unknown BUILD=$(BUILD))
endif

ALL_C_SRCS := $(call rwildcard,$(SRC_DIR),*.c)

MAIN_C_SRC := $(SRC_DIR)/main.c
LIB_C_SRCS := $(filter-out $(MAIN_C_SRC), $(ALL_C_SRCS))

MAIN_OBJ := $(patsubst %.c,$(OBJ_ROOT_DIR)/%.o,$(MAIN_C_SRC))
LIB_OBJ_STATIC := $(patsubst %.c,$(OBJ_ROOT_DIR)/%.o,$(LIB_C_SRCS))

DEPS := $(patsubst %.c,$(DEP_ROOT_DIR)/%.d,$(ALL_C_SRCS))

.PHONY: all dirs submodules clean run install uninstall test

all: dirs $(TARGET)

-include external/yyjson.mk
-include external/libgit2.mk
-include tests/build.mk

submodules:
	git submodule update --init --recursive

dirs:
	$(call MKDIR,$(BIN_DIR))
	$(call MKDIR,$(OBJ_ROOT_DIR))
	$(call MKDIR,$(DEP_ROOT_DIR))

$(TARGET): $(MAIN_OBJ) $(LIB_OBJ_STATIC) $(EXT_LIBS)
	$(CC) $(MAIN_OBJ) $(LIB_OBJ_STATIC) $(EXT_LIBS) $(LDFLAGS) -o $@

$(OBJ_ROOT_DIR)/%.o: %.c
	$(call MKDIR,$(dir $@))
	$(call MKDIR,$(DEP_ROOT_DIR)/$(dir $<))
	$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_ROOT_DIR)/$*.d -c $< -o $@

run: all
	$(TARGET)

install: all
ifeq ($(PLATFORM),posix)
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/zapup
else
	$(call MKDIR,$(DESTDIR)$(BINDIR))
	copy /Y $(call FIXPATH,$(TARGET)) $(call FIXPATH,$(DESTDIR)$(BINDIR)/zapup$(EXE_EXT))
endif

uninstall:
ifeq ($(PLATFORM),posix)
	rm -f $(DESTDIR)$(BINDIR)/zapup
else
	del /Q $(call FIXPATH,$(DESTDIR)$(BINDIR)/zapup$(EXE_EXT))
endif

-include $(DEPS)

clean:
ifeq ($(PLATFORM),posix)
	$(RMDIR) build out
else
	if exist build $(RMDIR) build
	if exist out $(RMDIR) out
endif
