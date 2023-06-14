BUILD = make
CLEAN = make clean
BUILD_DIR = make build_dir

SOURCE_DIR := ./src

OBJ_DIR := ./objs/
LIB_DIR := ./libs/
BIN_DIR := ./bins/

define makeallmodules
	for dir in $(SOURCE_DIR)/*; \
		do \
		make -C $$dir $(1); \
		done
endef

# to export the installation path to child Makefiles - use absolute path to libraries 
INSTALLATION_PATH = $(shell echo $$INSTALLATION_PATH)
ifeq ($(INSTALLATION_PATH),)
        INSTALLATION_PATH = $(shell echo $$PWD)
		export INSTALLATION_PATH
endif

all:
	@echo "###### BUILDING ########"
	$(call makeallmodules, all)

build_dir:
	@echo "####### BUILDING DIRECTORIES FOR OUTPUT BINARIES #######"
	$(call makeallmodules, build_dir)

clean:
	@echo "####### CLEANING ALL BUILD #######"
	$(call makeallmodules, clean)

delete:
	@echo "####### DELETING ALL BUILD FILES #######"
	-rm -rf $(OBJ_DIR)
	-rm -rf $(LIB_DIR)
	-rm -rf $(BIN_DIR)

.PHONY: all build_dir clean delete
