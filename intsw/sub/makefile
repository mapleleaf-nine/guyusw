LIB_SUB = libintswsub.a
ARFLAGS = -rv
INCLUDES = -I./ -I../main
EXT = c
EXTP = cc

SRC = $(wildcard *.$(EXT)) $(wildcard *.$(EXTP))
OBJS = $(SRC:.$(EXT)=.o) $(SRC:.$(EXTP)=.o)

all: $(SRC) $(LIB_SUB)

.$(EXT).o:
	$(CCOMPILE) $(FLAGS) $(INCLUDES) -o $@ -c $<

.$(EXTP).o:
	$(CPPCOMPILE) $(FLAGS) $(INCLUDES) -o -std=c++17 -o $@ -c $<

$(LIB_SUB): $(OBJS)
	ar $(ARFLAGS) $(LIB_SUB) $(OBJS)

clean:
	rm -rf *.o
	rm -rf *.a
