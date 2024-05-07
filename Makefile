CC=g++
CFLAGS=-Wall -Wextra -Werror -Wswitch-enum -ggdb -fsanitize=address -fsanitize=undefined -std=c++17
LIBS=-lm -lraylib

ARGS=
OUT=main.out

SRC=src
DIST=build
CODE=$(addprefix $(SRC)/, ) # add source files here
OBJECTS=$(CODE:$(SRC)/%.cpp=$(DIST)/%.o)
DEPDIR:=$(DIST)/dependencies
DEPFLAGS=-MT $@ -MMD -MP -MF
DEPS:=$(CODE:$(SRC)/%.cpp=$(DEPDIR):%.d) $(DEPDIR)/main.d

.PHONY: all
all: $(OUT)

$(OUT): $(DIST)/$(OUT)

$(DIST)/$(OUT): $(OBJECTS) $(DIST)/main.o | $(DIST)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(DIST)/%.o: $(SRC)/%.cpp | $(DIST) $(DEPDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(DEPDIR)/$*.d -c $< -o $@ $(LIBS)

.PHONY: run
run: $(DIST)/$(OUT)
	./$^ $(ARGS)

.PHONY:
clean:
	rm -rfv $(DIST)/*

$(DIST):
	mkdir -p $(DIST)

$(DEPDIR):
	mkdir -p $(DEPDIR)

-include $(DEPS)
