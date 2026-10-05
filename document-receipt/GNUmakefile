BASEDIR:=$(CURDIR)/build
THRU_C_SDK_DIR?=$(HOME)/.thru/sdk/c/thru-sdk
include $(THRU_C_SDK_DIR)/thru_c_program.mk

.PHONY: test
test:
	python3 scripts/test.py "$(THRU_C_SDK_DIR)"
