ifeq ($(origin CXX), default)
    CXX = g++
endif

INCLUDES+= -Iinclude

DEFINES+= -DSTX_DEBUG=2

CXX_FLAGS+= --std=c++14 -Wall -Wextra -Wno-unused-parameter -ggdb -g3

LD_FLAGS+= -lpthread -ldl

test.run: $(wildcard test/*.cpp) $(wildcard src/*.cpp) $(wildcard include/stx/*.hpp) $(wildcard include/stx/wip/*.hpp)
	${CXX}\
	 ${INCLUDES}\
	 ${DEFINES}\
	 ${CXX_FLAGS}\
	 ${LD_FLAGS}\
	 $(wildcard src/*.cpp)\
	 $(wildcard test/*.cpp)\
	 -o $@
	-valgrind --leak-check=full --track-origins=yes ./test.run

run: test.run
	./test.run

.PHONY: run_test
