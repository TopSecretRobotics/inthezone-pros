.PHONY: all app clean deps flash flash-pros format lsusb shell windocker

# Verbosity.

V ?= 0

verbose_0 = @
verbose_2 = set -x;
verbose = $(verbose_$(V))

# Platform detection.

ifeq ($(PLATFORM),)
ifeq ($(OS),Windows_NT)
PLATFORM = windows
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
PLATFORM = linux
else ifeq ($(UNAME_S),Darwin)
PLATFORM = darwin
else
$(error Unable to detect platform. Please open a ticket with the output of uname -a)
endif
endif
export PLATFORM
endif

ifeq ($(MACHINE),)
MACHINE = unknown
ifeq ($(PLATFORM),linux)
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),armv7l)
MACHINE = armv7l
endif
endif
export MACHINE
endif

ifeq ($(PLATFORM),darwin)
CORTEXFLASH ?= $(CURDIR)/tools/cortexflash.darwin
else ifeq ($(PLATFORM),linux)
ifeq ($(MACHINE),armv7l)
CORTEXFLASH ?= $(CURDIR)/tools/cortexflash.linux.armv7l
else
CORTEXFLASH ?= $(CURDIR)/tools/cortexflash.linux
endif
else ifeq ($(PLATFORM),windows)
CORTEXFLASH ?= $(CURDIR)/tools/cortexflash.exe
endif

# Core targets.

all:: app

clean::
	$(MAKE) -C pros clean

deps::

app::
	$(MAKE) -C pros

format::
	$(verbose) clang-format -i pros/src/*.c pros/include/*.h

ifeq ($(PLATFORM),windows)

VEX_DEVICE ?= $(word 1, $(shell (pros lsusb | grep -i com | head -n 1)))

else

VEX_DEVICE ?= $(word 1, $(shell (pros lsusb | grep -i vex | head -n 1)))

endif

flash-pros::
	$(MAKE) -C pros flash

ifeq ($(VEX_DEVICE),)

flash::
	$(error No device found. Connect USB device or specify with VEX_DEVICE environment variable)

shell::
	$(error No device found. Connect USB device or specify with VEX_DEVICE environment variable)

else

flash::
	-$(verbose) "$(CORTEXFLASH)" -X -w "$(CURDIR)/pros/bin/output.bin" -v -g 0x0 $(VEX_DEVICE)

ifeq ($(PLATFORM),windows)

shell::
	$(verbose) putty -serial $(VEX_DEVICE) -sercfg 115200

# $(error You will need to use PuTTY to connect to device $(VEX_DEVICE) at speed 115200)

else

shell::
	$(verbose) screen $(VEX_DEVICE) 115200

endif

endif

lsusb::
	$(verbose) pros lsusb

ifeq ($(PLATFORM),windows)

windocker::
	docker-compose build
	docker rm -f inthezonepros_project_data || true
	docker create -v /"$(shell pwd)":/build/project --name inthezonepros_project_data inthezonepros_project
	winpty docker run -it --rm --volumes-from inthezonepros_project_data inthezonepros_project

else

windocker::
	$(error This only works on Windows.)

endif
