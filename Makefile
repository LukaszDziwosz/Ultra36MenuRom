# Set path to build folder
OUTDIR = build

# Set cart type (cart16, cart32)
CARTTYPE = cart128_32

# Custom ROM bank labels (no whitespaces!)
# Ensure that Bank 1 is empty!, pad 16k banks to 32k
#DEFS = -DROM_NAMES_INIT='"Empty_Bank","GEOS_1581","GEOS_1571","Servant","StartApps_v1","Basic8","KeyDOS"' \
#    -DNUM_ROMS=7

# For 16-bank version:
 DEFS = -DROM_NAMES_INIT='"Empty_Bank","GEOS_1581","GEOS_1571","Servant","StartApps_v1","Basic8","KeyDOS","CP/M_Z80","StartApps_v2","Basic8_Plus","SuperCPU","GEOS_Mega","Utilities","Games_Pack","DevTools"' \
	 -DNUM_ROMS=15

TARGET = $(OUTDIR)/$(CARTTYPE).bin

CFG = $(wildcard $(CARTTYPE)/*.cfg)
ASRC = $(wildcard $(CARTTYPE)/*.s)
CSRC = src/main.c src/vdc_info_screen.c src/sid_info_screen.c

OBJ = $(ASRC:.s=.o) $(CSRC:.c=.o)

CL = cl65
CA = ca65
CC = cc65
AR = ar65
LD = ld65

CFLAGS = -Cl -Oris -t c128 $(DEFS)

$(TARGET): $(ASRC) $(CSRC)
	$(CL) --config $(CFG) $(CFLAGS) -o $@ $^

# Run in VICE with attached function ROM
.PHONY: run
run: $(TARGET)
	@echo "Running C128 with attached function ROM..."
	x128 -cartfrom $(TARGET)

.PHONY: clean
clean:
	rm $(OBJ)
	rm $(TARGET)

