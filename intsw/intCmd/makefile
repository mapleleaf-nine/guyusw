BIN_INT_CMD = preCmd
ARFLAGS = -rv
INCLUDES = -I./ -I../multiThread
EXT = c
EXTP = cc
CC = $(CCOMPILE)

SRCS = $(wildcard *.$(EXT)) $(wildcard *.$(EXTP))
OBJS = $(SRCS:.$(EXT)=.o) $(SRCS:.$(EXTP)=.o)

all: $(BIN_INT_CMD)

.$(EXT).o:
	$(CC) $(FLAGS) $(INCLUDES) -o $@ -c $<

.$(EXTP).o:
	$(CXX) $(FLAGS) $(INCLUDES) -o -std=c++17 -o $@ -c $<

$(BIN_INT_CMD): $(SRCS)
	$(CC) $(FLAGS) $(SRCS) -o $@
clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf preCmd
