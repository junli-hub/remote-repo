Srcs:= $(wildcard *.c)
OBJs:= $(patsubst %.c,%.o,$(Srcs))
EXE := main

CFLAGS := -Wall -g -lpthread -lcrypt


.PHONY: clean rebuild o_clean


$(EXE): $(OBJs)
	@if [ -z "$(OBJs)" ]; then \
		echo "No .c files found. Nothing to build."; \
		else \
		gcc $^ -o $@ $(CFLAGS); \
		fi  

%.o: %.c
	gcc -c $< -o $@ $(CFLAGS)  

clean:
	$(RM) $(EXE)

o_clean:
	$(RM) $(OBJs)

rebuild: clean $(EXE)


