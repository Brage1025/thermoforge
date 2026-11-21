
CC = gcc

CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS = -Iinclude

LDLIBS = -lm

TARGET = thermoforge

SOURCES = \
    src/main.c \
    src/config.c \
    src/grid.c \
    src/solver.c \
    src/diagnostics.c \
    src/output.c

OBJECTS = $(SOURCES:.c=.o)

TEST_TARGET = tests/test_core

TEST_SOURCES = \
    tests/test_core.c \
    src/config.c \
    src/grid.c \
    src/solver.c \
    src/diagnostics.c


.PHONY: all clean test run


all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDLIBS)


src/%.o: src/%.c include/thermoforge.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


$(TEST_TARGET): $(TEST_SOURCES) include/thermoforge.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET) $(LDLIBS)


test: $(TEST_TARGET)
	./$(TEST_TARGET)


run: $(TARGET)
	mkdir -p output
	./$(TARGET) examples/heated_plate.cfg


clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)
	rm -f $(TEST_TARGET)
