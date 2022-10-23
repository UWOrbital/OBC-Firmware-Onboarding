CC := gcc
BIN := challenge
BUILD_DIR := build

CFLAGS := -ggdb3
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Os
CFLAGS += -fstack-usage
CFLAGS += -std=c99

LDFLAGS := -ggdb3
LDFLAGS += -Os

INCLUDE_DIRS :=

CPPFLAGS := $(INCLUDE_DIRS) 

OBJS :=
OBJS += $(BUILD_DIR)/challenge.o

.PHONY: all
all: $(BUILD_DIR)/${BIN}

${BUILD_DIR}/${BIN} : ${OBJS}
	-mkdir -p ${@D}
	$(CC) $^ ${LDFLAGS} -o $@

${BUILD_DIR}/%.o : %.c
	-mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c $< -o $@

.PHONY: clean
clean:
	-rm -rf $(BUILD_DIR)
