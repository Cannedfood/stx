# STX

Replacements for some stl classes with additional functionality and also new classes which I wish were in there.

- More goals:
	- More debugging capabilities (from off to insane, insane would be for example full iterator debugging)

## Replacements
- memory -> xmemory
	- std::unique_ptr -> stx::owned
	- std::shared_ptr -> stx::shared
	- std::weak_ptr   -> stx::weak

## New
- xplatform (WIP)
	- A header which will contain various
- xthreadsafe_queue (WIP)
	- A threadsafe queue
- xthreadpool (WIP)
	- A threadpool
- xsockets (WIP) `#include <xsockets>` or `#include <stx/sockets.hpp>`
	- sockets and html requests (WIP)
- xhandles (WIP) `#include <xhandles>` or `#include <stx/handles.hpp>`
	- list_element<T>
		- Usage: `class my_class : public stx::list_element<my_class>`
		- Utility to make a class a doubly linked list

## License
The code is lincensed under the MIT license, see License.txt

## Building

Simply compile everything in src with your favorite compiler.<br>
**If you do not use xsockets this is currently not necessary**
```bash
c++ --std=c++14 src/* -o stx.o
```

## Tests
Run the makefile to run the tests, although they have horrible coverage.
