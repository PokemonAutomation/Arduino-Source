#include SourceRoot.mk

SOURCE_ROOT := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

include Objects.mk
include Common.mk

device-logger: $(addprefix $(OUTPUT_DEVICELOGGER)/, $(SOURCES:.cpp=.o))
	mkdir -p $(OUTPUT_ROOT)/Binaries/
	$(CC) Main.cpp $^ $(CFLAGS_DEVICELOGGER) -o $(OUTPUT_ROOT)/Binaries/$@

