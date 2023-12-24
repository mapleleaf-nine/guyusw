#Include paths
COMMON_PATH = ..
INTREFACE_INCL = $(COMMON_PATH)/interface
ADD_INCL = $(COMMON_PATH)/add
SUB_INCL = $(COMMON_PATH)/sub
THREAD_INCL = $(COMMON_PATH)/multiThread
INT_CMD_INCL= $(COMMON_PATH)/intCmd

#LIBs to link
LIB_INTERFACE = ../interface/libintswapi.a
LIB_ADD = ../add/libintswadd.a
LIB_SUB = ../sub/libintswsub.a
LIB_THREAD = ../multiThread/libintswthread.a

export CCOMPILE = gcc
export CPPCOMPILE = g++
#export CCOMPILE = arm-none-linux-gnueabihf-gcc
#export CPPCOMPILE = arm-none-linux-gnueabihf-g++

FIFO_PATH = ../fifoPath/cmd.fifo

export LINKD = $(CPPCOMPILE)
ARFLAGS = -rv
export FLAGS = -fPIC -O3 -Wall -Wchar-subscripts -Wshadow -DDEBUG=1 -DGTEST=1
INCLUES = -I$(INTREFACE_INCL) -I$(ADD_INCL) -I$(SUB_INCL) -I$(THREAD_INCL)

EXT = c
INT = intsw

#LD_FLAGS = -L$(AFE_AD9026_INCL)

SRC = ./main.c ./app.c ./cmdRegister.c
OBJS = $(SRC:.$(EXT)=.o)

#All DEPS are built
DEPS = $(LIB_INTERFACE) $(LIB_ADD) $(LIB_SUB) $(LIB_THREAD)

$(INT): interface_lib add_lib sub_lib thread_lib preCmd\
	$(INT)

$(INT): interface_lib add_lib sub_lib thread_lib $(OBJS)
	$(LINKD) $(LD_FLAGS) -o $@ $(OBJS) $(DEPS) -lpthread -lm

.$(EXT).o:
	$(CCOMPILE) $(FLAGS) $(INCLUES) -o $@ -c $<

interface_lib:
	$(MAKE) -C $(INTREFACE_INCL)

add_lib:
	$(MAKE) -C $(ADD_INCL)

sub_lib:
	$(MAKE) -C $(SUB_INCL)

thread_lib:
	$(MAKE) -C $(THREAD_INCL)

preCmd:
	$(MAKE) -C $(INT_CMD_INCL)

clean:
	$(MAKE) -C $(INTREFACE_INCL) clean
	$(MAKE) -C $(ADD_INCL) clean
	$(MAKE) -C $(SUB_INCL) clean
	$(MAKE) -C $(THREAD_INCL) clean
	$(MAKE) -C $(INT_CMD_INCL) clean
	rm -rf *.o
	rm -rf *.a
	rm -rf $(FIFO_PATH)
	rm -rf $(INT)
