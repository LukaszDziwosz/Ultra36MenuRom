# ===============================
# Ultra-36 ROM Switcher Build System (for C128 Function ROM)
# Makefile for CC65 cross-compiler (cart128 format)
# ===============================

# === Output folder ===
OUTDIR = build

# === Cartridge output format ===
# Note: This selects the ROM file size for CC65 output:
# cart128_16 = 16KB cartridge binary
# cart128_32 = 32KB cartridge binary (default)
CARTTYPE = cart128_32

# === ROM Bank Definitions ===
# ROM bank count is controlled fully by ROM_NAMES_INIT and NUM_ROMS.
# This is independent of CARTTYPE! Bank 0 (first) reserved for this program!

# --- 8-bank version (SST39SF020A, 256KB flash) ---
#DEFS = -DROM_NAMES_INIT='"Empty_Bank","GEOS_1581","GEOS_1571","Servant","DiskMaster","Basic8","KeyDOS"' \
#       -DNUM_ROMS=7

# --- 16-bank version (SST39SF040, 512KB flash) ---
DEFS = -DROM_NAMES_INIT='"Empty_Bank","GEOS_1581","GEOS_1571","Servant","DiskMaster","Basic8","KeyDOS","SuperChip","StartApps_v1","StartApps_v2","StartApps_v3","StartApps_v4","StartApps_v5","StartApps_v6","c128_diag"' \
       -DNUM_ROMS=15

# === Build target ===
TARGET = $(OUTDIR)/ultra36_$(subst cart128_,,$(CARTTYPE)).bin

# === Source files ===
CFG = $(wildcard $(CARTTYPE)/*.cfg)
ASRC = $(wildcard $(CARTTYPE)/*.s)
CSRC = src/main.c src/vdc_info_screen.c src/sid_info_screen.c

OBJ = $(ASRC:.s=.o) $(CSRC:.c=.o)

# === Toolchain ===
CL = cl65
CA = ca65
CC = cc65
AR = ar65
LD = ld65

# === Compiler flags ===
CFLAGS = -Cl -Oris -t c128 $(DEFS)

# === Build rule ===
$(TARGET): $(ASRC) $(CSRC)
	$(CL) --config $(CFG) $(CFLAGS) -o $@ $^

# === Run in VICE (Linux/MacOS default) ===
.PHONY: run
# Windows users comment out this one.
run: $(TARGET)
	@echo "Running C128 with attached function ROM..."
	x128 -cartfrom $(TARGET)

# === Windows Users ===
# Uncomment and adjust VICE path below:
#VICE = "C:/Path/To/WinVICE/x128.exe"
#run: $(TARGET)
#	$(VICE) -cartfrom $(TARGET)

# === Clean ===
.PHONY: clean
clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

# === Additional build targets for convenience ===
.PHONY: 16k 32k
16k:
	$(MAKE) CARTTYPE=cart128_16

32k:
	$(MAKE) CARTTYPE=cart128_32
