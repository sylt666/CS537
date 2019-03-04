# The make utility automatically determines which pieces of a large program
# need to be recompiled, and issues commands to recompile them. This file tells
# make how to compile and link the program. This makefile also tells make how
# to run miscellaneous commands when explicitly asked (for example, to run an
# emulator after building the correct filesystem image, or to remove certain
# files as a clean-up operation).
#
# http://www.gnu.org/software/make/manual/

# Default make target
.PHONY: all
all: mangoOS.img fs.img

################################################################################
# Build options
################################################################################

# delete target if error building it
.DELETE_ON_ERROR:

PREFIX := i686-elf-
# C compiler options
# http://gcc.gnu.org/onlinedocs/gcc-4.4.6/gcc/Invoking-GCC.html
CC = $(PREFIX)gcc
# enable extra warnings
CFLAGS += -Wall
# treat warnings as errors
CFLAGS += -Werror
# produce debugging information for use by gdb
CFLAGS += -ggdb

# uncomment to enable optimizations. improves performance, but may make
# debugging more difficult
#CFLAGS += -O2

# C Preprocessor
CPP := $(PREFIX)cpp

# Assembler options
# http://sourceware.org/binutils/docs/as/Invoking.html
AS := $(PREFIX)gcc
ASFLAGS += -ggdb # produce debugging information for use by gdb

# Linker options
# http://sourceware.org/binutils/docs/ld/Options.html
LD := $(PREFIX)ld

OBJCOPY := $(PREFIX)objcopy

OBJDUMP := $(PREFIX)objdump

################################################################################
# Emulator Options
################################################################################

# If the makefile can't find QEMU, specify its path here
QEMU := qemu-system-i386

# Try to infer the correct QEMU if not specified
ifndef QEMU
QEMU := $(shell if which qemu 1> /dev/null 2> /dev/null; \
	then echo qemu; exit; \
	else \
	qemu=/u/c/s/cs537-2/ta/tools/qemu; \
	if test -x $$qemu; then echo $$qemu; exit; fi; fi; \
	echo "***" 1>&2; \
	echo "*** Error: Couldn't find a working QEMU executable." 1>&2; \
	echo "*** Is the directory containing the qemu binary in your " 1>&2; \
	echo "*** PATH or have you tried setting the QEMU variable in " 1>&2; \
	echo "*** Makefile?" 1>&2; \
	echo "***" 1>&2; exit 1)
endif

# try to generate a unique GDB port
GDBPORT := $(shell expr `id -u` % 5000 + 25000)

# QEMU's gdb stub command line changed in 0.11
QEMUGDB := $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

# number of CPUs to emulate in QEMU
ifndef CPUS
CPUS := 1
endif

QEMUOPTS := -hdb fs.img mangoOS.img -smp $(CPUS) -snapshot

################################################################################
# Main Targets
################################################################################
include kernel/makefile.mk
include user/makefile.mk
include tools/makefile.mk
DEPS := $(KERNEL_DEPS) $(USER_DEPS) $(TOOLS_DEPS)
CLEAN := $(KERNEL_CLEAN) $(USER_CLEAN) $(TOOLS_CLEAN) \
	fs fs.img .gdbinit .bochsrc dist isodir *.iso

.PHONY: clean distclean run depend qemu qemu-nox qemu-gdb qemu-nox-gdb bochs

# remove all generated files
clean:
	rm -rf $(CLEAN)

run: qemu

# run mangoOS in qemu
qemu: fs.img mangoOS.img
	@echo Ctrl+a h for help
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

# run mangoOS in qemu without a display (serial only)
qemu-nox: fs.img mangoOS.img
	@echo Ctrl+a h for help
	$(QEMU) -nographic $(QEMUOPTS)

# run mangoOS in qemu in debug mode
qemu-gdb: fs.img mangoOS.img .gdbinit
	@echo "Now run 'gdb' from another terminal." 1>&2
	@echo Ctrl+a h for help
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

# run mangoOS in qemu without a display (serial only) in debug mode
qemu-nox-gdb: fs.img mangoOS.img .gdbinit
	@echo "Now run 'gdb' from another terminal." 1>&2
	@echo Ctrl+a h for help
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUGDB)

# run mangoOS in bochs
bochs: fs.img mangoOS.img .bochsrc
	bochs -q

# generate dependency files
depend: $(DEPS)

dist: clean
	mkdir -p dist/mangoOS
	git describe --long > version
	cp -r version include kernel tools user FILES Makefile \
		README .gitignore dist/mangoOS/ logo
	cd dist && tar -czf mangoOS.tar.gz mangoOS
	mv dist/mangoOS.tar.gz .
	rm -rf dist


################################################################################
# Build Recipies
################################################################################



%.asm: %.o
	$(OBJDUMP) -S $< > $@

%.sym: %.o
	$(OBJDUMP) -t $< | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $@

fs:
	mkdir -p fs

fs/%: user/bin/% | fs
	cp $< $@

fs/README: README | fs
	cp $< $@

fs/logo: logo | fs
	cp $< $@

USER_BINS := $(notdir $(USER_PROGS))
fs.img: tools/mkfs fs/README fs/logo $(addprefix fs/,$(USER_BINS))
	./tools/mkfs fs.img fs

.gdbinit: tools/dot-gdbinit
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

.bochsrc: tools/dot-bochsrc
	cp dot-bochsrc .bochsrc

.DEFAULT:
	@echo "No rule to make target $@"

qemu-grub: fs.img iso
	$(QEMU) -cdrom mangoOS.iso  -k en-us  -hdb fs.img
iso:kernel/kernel
	mkdir -p isodir
	mkdir -p isodir/boot
	mkdir -p isodir/boot/grub
	cp kernel/kernel isodir/boot/mangoOS.kernel
	cp anotherOS.kernel isodir/boot/anotherOS.kernel
	echo 'menuentry "mangoOS" {multiboot /boot/mangoOS.kernel}; menuentry "anotherOS" {multiboot /boot/anotherOS.kernel}' > isodir/boot/grub/grub.cfg
	grub-mkrescue -o mangoOS.iso isodir