ifeq ($(origin CXX), default)
    CXX = clang++
endif

INCLUDES+= -Iinclude

DEFINES+= -DSTX_DEBUG=2

CXX_FLAGS+= --std=c++14 -Wall -Wextra -Wno-unused-parameter

LD_FLAGS+= -lpthread -ldl

test.run: $(wildcard test/*.cpp) $(wildcard src/*.cpp) $(wildcard include/stx/*.hpp)
	${CXX}\
	 ${INCLUDES}\
	 ${DEFINES}\
	 ${CXX_FLAGS}\
	 ${LD_FLAGS}\
	 $(wildcard src/*.cpp)\
	 $(wildcard test/*.cpp)\
	 -o $@
	
	./test.run

.PHONY: run_test
