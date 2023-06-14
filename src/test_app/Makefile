MODULE_NAME = test_app

SRC = ./src
PROJ_ROOT_DIR = ../..
OBJ_DIR = $(PROJ_ROOT_DIR)/objs/$(MODULE_NAME)

INSTALLATION_PATH = $(shell echo $$INSTALLATION_PATH)
ifeq ($(INSTALLATION_PATH),)
        INSTALLATION_PATH = $(PROJ_ROOT_DIR)
endif

TARGET_DIR = $(INSTALLATION_PATH)/bins
TARGET = $(TARGET_DIR)/$(MODULE_NAME)

LIBRARY_DIR = $(INSTALLATION_PATH)/libs

# C compiler
CXX = $(shell echo $$CXX)
ifeq ($(CXX),)
CXX = gcc
endif

# linker
LDXX = $(shell echo $$CXX)
ifeq ($(LDXX),)
LDXX = gcc
endif

STDFLAG = -std=gnu99

INC = -I../mem_mang/inc/

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

WARN=-Wall -Wextra -Werror -Wwrite-strings -Wno-parentheses \
     -pedantic -Warray-bounds -Wno-unused-variable -Wno-unused-function \
     -Wno-unused-parameter -Wno-unused-result

DEP_LIBS = -L$(LIBRARY_DIR) -lglthreads -lmem_mang

CCFLAGS = $(STDFLAG) $(WARN) $(INC)
LDFLAGS = $(DEP_LIBS) 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LDXX) -o $@ $^ $(LDFLAGS)

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
