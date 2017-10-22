
EXE=hex_compare

SRC=main.c

$(EXE):$(SRC)
	gcc -o $@ $^

install:$(EXE)
	sudo cp $(EXE) /usr/bin
