CC := i686-elf-gcc
AR := i686-elf-ar
NASM := nasm

BUILD_DIR := build
SRC_DIR := src
INC_DIR := include

CFLAGS := -std=gnu11 -ffreestanding -O2 -Wall -Wextra -m32 \
	-fno-pie -fno-stack-protector -fno-builtin -I $(INC_DIR)

C_SRCS := $(SRC_DIR)/syscall.c \
          $(SRC_DIR)/string.c \
          $(SRC_DIR)/stdlib.c \
          $(SRC_DIR)/stdio.c \
          $(SRC_DIR)/errno.c \
          $(SRC_DIR)/signal.c \
          $(SRC_DIR)/getopt.c \
          $(SRC_DIR)/dirent.c \
          $(SRC_DIR)/time.c \
          $(SRC_DIR)/pwd.c \
          $(SRC_DIR)/fnmatch.c \
          $(SRC_DIR)/regex.c \
          $(SRC_DIR)/net.c \
          $(SRC_DIR)/compat.c

C_OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRCS))
SETJMP_OBJ := $(BUILD_DIR)/setjmp.o
CRT0_OBJ := $(BUILD_DIR)/crt0.o
LIBC_A := $(BUILD_DIR)/libc.a
LIBM_A := $(BUILD_DIR)/libm.a

.PHONY: all clean test

all: $(LIBC_A) $(LIBM_A) $(CRT0_OBJ)

$(BUILD_DIR):
	mkdir -p $@

$(CRT0_OBJ): $(SRC_DIR)/crt0.asm | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SETJMP_OBJ): $(SRC_DIR)/setjmp.asm | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

$(LIBC_A): $(C_OBJS) $(SETJMP_OBJ) | $(BUILD_DIR)
	$(AR) rcs $@ $^

$(LIBM_A): | $(BUILD_DIR)
	$(AR) rcs $@

clean:
	rm -rf $(BUILD_DIR)
