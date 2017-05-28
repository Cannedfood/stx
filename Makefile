run_test: test.run
	./test.run

test.run: $(wildcard test/*.cpp) $(wildcard src/*.cpp) $(wildcard include/stx/*.hpp)
	clang++ --std=c++14 -Iinclude "-DSTX_DEBUG=2" $(wildcard src/*.cpp) $(wildcard test/*.cpp) -lpthread -ldl -o $@

.PHONY: run_test
