OUTDIR := ../build
TARGET := $(OUTDIR)/protocol_ack_test.prg
D64 := $(OUTDIR)/protocol_ack_test.d64

.PHONY: all clean

all: $(TARGET) $(D64)

$(OUTDIR):
	mkdir -p "$(OUTDIR)"

$(TARGET): protocol_ack_test.c | $(OUTDIR)
	cl65 -t c128 -O -o "$(TARGET)" protocol_ack_test.c

$(D64): $(TARGET)
	c1541 -format "ultra36dbg,ud" d64 "$(D64)" \
		-write "$(TARGET)" "protocol ack test"

clean:
	rm -f "$(TARGET)" "$(D64)" protocol_ack_test.o
