ifeq ($(origin CXX), default)
    CXX = clang++
endif

INCLUDES+= -Iinclude

DEFINES+= -DSTX_WIP=1\
          -DSTX_UNSTABLE=1\
          -DSTX_DEBUG=2

CXX_FLAGS+= --std=c++14 -Wall -Wextra

LD_FLAGS+= -lpthread -ldl

run_test: test.run
	./test.run

test.run: $(wildcard test/*.cpp) $(wildcard src/*.cpp) $(wildcard include/stx/*.hpp)
	${CXX}\
	 ${INCLUDES}\
	 ${DEFINES}\
	 ${CXX_FLAGS}\
	 ${LD_FLAGS}\
	 $(wildcard src/*.cpp)\
	 $(wildcard test/*.cpp)\
	 -o $@

.PHONY: run_test
