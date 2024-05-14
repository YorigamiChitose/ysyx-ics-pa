TOP_DIR   = $(PWD)
BUILD_DIR = $(TOP_DIR)/build
PRJ = playground

# Chisel Config
CHISEL_BUILD_DIR      = $(BUILD_DIR)/chisel
CHISEL_DIR            = $(TOP_DIR)/src
CHISEL_BUILD_TOP_VSRC = $(CHISEL_BUILD_DIR)/$(TOP_NAME).sv
CHISEL_BUILD_VSRC     = $(foreach dir,$(CHISEL_BUILD_DIR),$(wildcard $(dir)/*.v)) $(foreach dir,$(CHISEL_BUILD_DIR),$(wildcard $(dir)/*.sv))
CHISEL_MAIN_DIR       = $(CHISEL_DIR)/main
CHISEL_TEST_DIR       = $(CHISEL_DIR)/test
CHISEL_SRC_PATH       = $(foreach dir, $(shell find $(CHISEL_MAIN_DIR) -maxdepth 3 -type d), $(wildcard $(dir)/*.scala))
CHISEL_TEST_DIR       = $(TOP_DIR)/test_run_dir
CHISEL_TOOL           = main.Tools.build

verilog: $(CHISEL_BUILD_TOP_VSRC)

$(CHISEL_BUILD_TOP_VSRC): $(CHISEL_MAIN_PATH)
	@echo --- verilog start  ---
	@mkdir -p $(CHISEL_BUILD_DIR)
	@mill -i $(PRJ).runMain $(CHISEL_TOOL) --split-verilog -td $(CHISEL_BUILD_DIR)
	@echo --- verilog finish ---

test:
	mill -i $(PRJ).test

fmt:
	mill -i __.reformat

checkformat:
	mill -i __.checkFormat

compile:
	mill -i __.compile

help:
	@mill -i __.runMain $(CHISEL_TOOL) --help

clean-v:
	@rm -rf $(CHISEL_BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)