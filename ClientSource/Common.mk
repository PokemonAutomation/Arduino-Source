SOURCE_ROOT := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OUTPUT_ROOT := $(SOURCE_ROOT)/../Linux

CC=g++

CFLAGS := -I $(SOURCE_ROOT) -std=c++17 -fno-rtti -Wall -O2 -g -ggdb
CFLAGS += -lpthread


dump:
	echo $(SOURCES)

clean:
	rm -r $(OUTPUT_ROOT)

all: device-logger

.DEFAULT_GOAL := all


CFLAGS_DEVICELOGGER := $(CFLAGS)
OUTPUT_DEVICELOGGER := $(OUTPUT_ROOT)/device-logger
-include $(addprefix $(OUTPUT_DEVICELOGGER)/, $(SOURCES:.cpp=.d))
$(OUTPUT_DEVICELOGGER)/%.o: $(SOURCE_ROOT)/%.cpp
	mkdir -p $(shell dirname $@)
	$(CC) $< $(CFLAGS_DEVICELOGGER) -c  -o $@
	$(CC) $< $(CFLAGS_DEVICELOGGER) -MM -MT $@ > $(@:.o=.d)
	@cp -f $(@:.o=.d) $(@:.o=.d.tmp)
	@sed -e 's/.*://' -e 's/\\$$//' < $(@:.o=.d.tmp) | fmt -1 | \
	 sed -e 's/^ *//' -e 's/$$/:/' >> $(@:.o=.d)
	@rm -f $(@:.o=.d.tmp)
device-logger: $(addprefix $(OUTPUT_DEVICELOGGER)/, $(SOURCES:.cpp=.o))


