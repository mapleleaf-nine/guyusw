LIB_API = libintswapi.a
ARFLAGS = -rv
INCLUDES = -I./ -I../add -I../sub -I../main
EXT = c
EXTP = cc

SRC = $(wildcard *.$(EXT)) $(wildcard *.$(EXTP))
OBJS = $(SRC:.$(EXT)=.o) $(SRC:.$(EXTP)=.o)

all: $(SRC) $(LIB_API)

.$(EXT).o:
	$(CCOMPILE) $(FLAGS) $(INCLUDES) -o $@ -c $<

.$(EXTP).o:
	$(CPPCOMPILE) $(FLAGS) $(INCLUDES) -o -std=c++17 -o $@ -c $<

$(LIB_API): $(OBJS)
	ar $(ARFLAGS) $(LIB_API) $(OBJS)

clean:
	rm -rf *.o
	rm -rf *.a
