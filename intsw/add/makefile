LIB_ADD = libintswadd.a
ARFLAGS = -rv
INCLUDES = -I./ -I../main 
EXT = c
EXTP = cc

SRC = $(wildcard *.$(EXT)) $(wildcard *.$(EXTP))
OBJS = $(SRC:.$(EXT)=.o) $(SRC:.$(EXTP)=.o)

all: $(SRC) $(LIB_ADD)

.$(EXT).o:
	$(CCOMPILE) $(FLAGS) $(INCLUDES) -o $@ -c $<

.$(EXTP).o:
	$(CPPCOMPILE) $(FLAGS) $(INCLUDES) -o -std=c++17 -o $@ -c $<

$(LIB_ADD): $(OBJS)
	ar $(ARFLAGS) $(LIB_ADD) $(OBJS)

clean:
	rm -rf *.o
	rm -rf *.a
