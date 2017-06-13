# STX

Replacements for some stl classes with additional functionality and also new classes which I wish were in there.

- More goals:
	- More debugging capabilities (from off to insane, insane would be for example full iterator debugging)

## Disclaimer
The tests are very lacking at some points so don't yet rely on this stuff.
I use it in my own projects, so I'll catch the worst errors there but still.

## Replacements
- `memory` -> `xmemory`
	- `std::unique_ptr<T, Tdel>` -> `stx::owned<T, Tdel>`
	- `std::shared_ptr<T>` -> `stx::shared<T>`
	- `std::weak_ptr<T>`   -> `stx::weak<T>`
- `cassert`/`assert.h` -> xassert
	- `xfatal(MSG)` throws an exception with the current location
	- `xassert(TEST)` throws and exception if test is false
	- `xassertmsg(TEST, MSG)` throws and exception if test is false; includes the message in the exception
	- `important<T>` marks a value as important and gives a warning in debug mode if it is not handled

## TODO

See [here](Todo.md)

## New
- xaccess
	- A header which contains makros for getters and setters
	- `xaccess(fn_name, member)` Read and write access to a member
	- `xsetter(fn_name, member)` Write-only access to a member (via. `value const& object.fn_name()`)
	- `xgetter(fn_name, member)` Read-only access to a member (via. `object& object.fn_name(value)`)
- xtimer
	- `timer`
		- Used to get the duration of stuff:
			- Restart the timer with `timer.begin()` (Will be automatically called on construction)
			- Get the time with `timer.nanos()` ... `timer.days()`
- xhash
	- implements various hash algorithms
	- `symbol<hash_algorithm>` Similar to rubys symbol, this is essentially a compile-time string with a compile-time precomputed hash
- xevent
	- `event` attach callbacks get handles back
- xshared_lib
	- `shared_lib` Represents a shared library.
- xlogging
	- contains printf-styled logging functions for C++
		- `writelog<level>(format, ...)`
		- `writelog<level>(...)`
		- `info(...)` (Can use format string as first argument)
		- `warn(...)` (Can use format string as first argument)
		- `perfwarn(...)` (Can use format string as first argument)
		- `error(...)` (Can use format string as first argument)
		- format: Like printf but instead of `%s` `%u` etc. You only use `%%` e.g.
		```C++
		stx::error("At %% he realized, he %% all along.", that_moment(), "followed the nullptr");
		```
- xplatform
	- A header which will contain various information about the current compiler
		- `STX_OS_(UNIX|LINUX|WINDOWS|MACOSX)`
			- `STX_OS_UNIX` is defined on Linux & Macosx
		- `STX_COMPILER_(GCC|CLANG|MSCV)`
		- `STX_ARCHITECTURE_(ARM|x64|x86)`
		- `STX_BYTEORDER_(BIG|LITTLE)_ENDIAN`
		- `STX_FORCEINLINE`
		- `STX_CONSTEXPR` (More lax constexpr)
- xhandles (WIP) `#include <xhandles>` or `#include <stx/handles.hpp>`
	- list_element<T>
		- Usage: `class my_class : public stx::list_element<my_class>`
		- Utility to make a class a doubly linked list
- xsockets (WIP)
	- sockets and html requests (WIP)
- xthreadsafe_queue (WIP)
	- A threadsafe queue
- xthreadpool (WIP)
	- A threadpool

## Build options
- `STX_DEBUG_TOOLS=1`
	- Enables runtime assertions and warnings
- `STX_DEBUG=1`
	- Enables internal assertions (to debug the stx libraries)
- `STX_MODULE_NAME=ModuleName`
	- Mainly used for logging, prefixes logs from this module with '[ModuleName]'
- `STX_UNSTABLE`
	- Enable untested/VERY unstable features
- `STX_WIP`
	- Enable totally work in progress, pretty much unusable features

## License
The code is licensed under the MIT license, see License.txt

## Building

Simply compile everything in src with your favorite compiler.<br>
**If you do not use xsockets or xshared_lib this is currently not necessary**
```bash
c++ --std=c++14 src/* -o stx.o
```

## Tests
Run the Makefile to run the tests, although they have horrible coverage.
