LIB_THREAD = libintswthread.a
ARFLAGS = -rv
INCLUDES = -I./ -I../main -I../intCmd
EXT = c
EXTP = cc

SRCS = $(wildcard *.$(EXT)) $(wildcard *.$(EXTP))
OBJS = $(SRCS:.$(EXT)=.o) $(SRCS:.$(EXTP)=.o)

all: $(LIB_THREAD)

.$(EXT).o:
	$(CCOMPILE) $(FLAGS) $(INCLUDES) -o $@ -c $<

.$(EXTP).o:
	$(CPPCOMPILE) $(FLAGS) $(INCLUDES) -o -std=c++17 -o $@ -c $<

$(LIB_THREAD): $(OBJS)
	ar $(ARFLAGS) $(LIB_THREAD) $(OBJS) 

clean:
	rm -rf *.o
	rm -rf *.a
