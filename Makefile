# Makefile for json2relcsv

CC      := gcc
CFLAGS  := -Wall -g

LEX     := flex
YACC    := bison -d --defines=parser.h

# Source files
SRC_C   := ast.c util.c error.c csvgen.c schema.c main.c
SRC_L   := scanner.l
SRC_Y   := parser.y

# Generated sources
GEN_L   := lex.yy.c
GEN_Y   := parser.c

# Object files
OBJS    := $(SRC_C:.c=.o) parser.o lex.yy.o

# Target binary
TARGET  := json2relcsv

all: $(TARGET)

# Flex step: scanner.l → lex.yy.c
$(GEN_L): $(SRC_L) parser.h
	$(LEX) -o $@ $(SRC_L)

# Bison step: parser.y → parser.c + parser.h
$(GEN_Y) parser.h: $(SRC_Y)
	$(YACC) -o parser.c $(SRC_Y)

# Compile the Bison‐generated parser
parser.o: parser.c ast.h error.h util.h
	$(CC) $(CFLAGS) -c parser.c -o parser.o

# Compile the Flex‐generated scanner
lex.yy.o: lex.yy.c parser.h util.h error.h
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o

# Compile all other .c → .o
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

# Link everything
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Clean up generated and object files
clean:
	rm -f $(TARGET) \
	      $(GEN_L) $(GEN_Y) parser.h parser.output \
	      $(OBJS)

.PHONY: all clean
