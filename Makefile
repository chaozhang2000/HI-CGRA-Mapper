#doc :
#	@mkdir -p $(DOC_DIR)
#	@doxygen Doxyfile

HOME = $(MAPPER_HOME)
include $(HOME)/scripts/config.mk
-include $(HOME)/include/config/auto.conf
-include $(HOME)/include/config/auto.conf.cmd

remove_quote = $(patsubst "%",%,$(1))
SHARE = 1
NAME = mapperPass
CC = $(call remove_quote,$(CONFIG_CC))
CXXSRCS += $(shell find ./src -name "*.cpp")
CFLAGS_BUILD += $(call remove_quote,$(CONFIG_CC_OPT))
CFLAGS_BUILD += $(if $(CONFIG_CC_DEBUG),-Og -ggdb3,)
CFLAGS += $(shell llvm-config-12 --cxxflags) $(CFLAGS_BUILD)
LDFLAGS += $(shell llvm-config-12 --ldflags) $(CFLAGS_BUILD)
include $(HOME)/scripts/build.mk
