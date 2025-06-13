# Set path to build folder
OUTDIR = build

# Set cart type (cart16, cart32)
CARTTYPE = cart128_16

# Custom ROM bank labels
DEFS = -DROM1_NAME=\"GEOS\" \
       -DROM2_NAME=\"TASS\" \
       -DROM3_NAME=\"Servant\" \
       -DROM4_NAME=\"SuperA\" \
       -DROM5_NAME=\"SuperB\" \
       -DROM6_NAME=\"Basic8\" \
       -DROM7_NAME=\"KeyDOS\" \
       -DNUM_ROMS=7

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

