CFLAGS+=-std=c11 -Wall -pedantic
SOURCES=handler.c wac.c
OUTPUT=wac.exe

all:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(OUTPUT)

test:
	printcol.bat \| wac.exe

