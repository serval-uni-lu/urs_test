_OBJ=BDDSampler.o cuddAdapter.o humanNums.o mytime.o synExp.o cuddAlgo.o TravInfo.o OneComponent.o MultiComponents.o Traverser.o Trie.o
OBJ = $(patsubst %,../obj/%,$(_OBJ))
DEP = $(OBJ:.o=.d)  # one dependency file for each source


INCLUDEFLAGS=-I../include -I../tools/src -Isrc -I/opt/local/include -I/usr/local/include 

LDFLAGS=-static -lgmp -lgmpxx -lcudd -L../lib -L/usr/local/lib -L/opt/local/lib 

ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CCFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CCFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CCFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
       CXXFLAGS=-std=c++11 -O3 -g -I. -I/usr/local/include  
       LDFLAGS += -lpthread
    endif
    ifeq ($(UNAME_S),Darwin)
        CXXFLAGS=-fcolor-diagnostics -O3 -Wno-deprecated-register -std=c++11 -stdlib=libc++ -g -mtune=native
	LDFLAGS += -Wl,-stack_size,0x40000000,-stack_addr,0xf0000000
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += -D AMD64:wq
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -D ARM
    endif
endif

CC=c++

../bin/BDDSampler: ${OBJ} 
	@echo \\t"[CXX]  " \\t$@
	@$(CC) $(CXXFLAGS) -o $@ ${OBJ}  $(INCLUDEFLAGS) $(LDFLAGS)

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
../obj/%.d: src/%.cpp
	@$(CPP) $(CFLAGS) $(INCLUDEFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	-@rm -f ${OBJ} ${DEP}  ../bin/BDDSampler

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP)
endif

../obj/%.o: src/%.cpp src/%.hpp
	@echo \\t"[CXX]  " \\t$@
	@$(CC) -c $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $<

../obj/%.o: src/%.cpp
	@echo \\t"[CXX]  " \\t$@
	@$(CC) -c $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $<
