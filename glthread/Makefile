MODULE_NAME = glthread

SRC = ./src
PROJ_ROOT_DIR = ..
OBJ_DIR = $(PROJ_ROOT_DIR)/objs/$(MODULE_NAME)

INSTALLATION_PATH = $(shell echo $$INSTALLATION_PATH)
ifeq ($(INSTALLATION_PATH),)
        INSTALLATION_PATH = $(PROJ_ROOT_DIR)
endif

TARGET_DIR = $(INSTALLATION_PATH)/libs
TARGET = $(TARGET_DIR)/lib$(MODULE_NAME).a

# C compiler
CXX = $(shell echo $$CXX)
ifeq ($(CXX),)
CXX = gcc
endif

STDFLAG = -std=gnu99

INC = -Iinc/

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

WARN=-Wall -Wextra -Werror -Wwrite-strings -Wno-parentheses \
     -pedantic -Warray-bounds -Wno-unused-variable -Wno-unused-function \
     -Wno-unused-parameter -Wno-unused-result

CCFLAGS = $(STDFLAG) $(WARN) $(INC)

all: $(TARGET)

$(TARGET): $(OBJS)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC)/%.c
	$(CXX) $(CCFLAGS) -o $@ -c $<

build_dir:
	@echo Creating object and libs directory
	mkdir -p $(OBJ_DIR)
	mkdir -p $(TARGET_DIR)

clean:
	@echo Clean Build
	-rm $(OBJS)
	-rm -f $(TARGET)

.PHONY: clean build_dir all
