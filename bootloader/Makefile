NASM ?= nasm
SRC  := boot32.asm
BIN  := boot.bin

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC)
	$(NASM) $< -o $@ -f bin

clean:
	rm -f $(BIN)