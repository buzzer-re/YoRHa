# LIBPS4	:= $(PS4SDK)/libPS4

CC	:= gcc
OBJCOPY	:= objcopy
ODIR	:= build
SDIR	:= src
# LDIRS	:= -L$(LIBPS4)
MAPFILE := $(shell basename "$(CURDIR)").map
CFLAGS	:= $(IDIRS) -I./freebsd-headers/include/ -Os -std=c11 -ffunction-sections -fdata-sections -fno-builtin -nostartfiles -nostdlib -Wall -Wextra -march=btver2 -mtune=btver2 -m64 -mabi=sysv -mcmodel=small -fpie -fPIC
LFLAGS	:= -Xlinker -T ./linker.x -Xlinker -Map="$(MAPFILE)" -Wl,--build-id=none -Wl,--gc-sections
CFILES	:= $(wildcard $(SDIR)/*.c) $(wildcard $(SDIR)/dbg_commands/*.c)
SFILES	:= $(wildcard $(SDIR)/*.s)
OBJS	:= $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(CFILES)) $(patsubst $(SDIR)/%.s, $(ODIR)/%.o, $(SFILES)) $(patsubst $(SDIR)/dbg_commands/%.c, $(ODIR)/dbg_commands/%.o, $(CFILES))

# LIBS	:= -lPS4

TARGET = $(shell basename "$(CURDIR)").bin
ELF    = $(shell basename "$(CURDIR)").elf
$(TARGET): $(ODIR) $(OBJS)

	$(CC) $(ODIR)/*.o $(ODIR)/dbg_commands/*.o -o $(ELF) $(CFLAGS) $(LFLAGS) $(LIBS)
	$(OBJCOPY) -S -O binary $(ELF) "$(TARGET)"
	mv $(ELF) build/

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SDIR)/%.s
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR):
	@mkdir -p build/dbg_commands

.PHONY: clean

clean:
	rm -rf "$(TARGET)" "$(MAPFILE)" $(ODIR)
