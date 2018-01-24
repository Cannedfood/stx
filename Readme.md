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
- `timer.hpp`
	- `stx::timer`
		- Used to get the duration of stuff:
			- Restart the timer with `timer.reset()` (Will be automatically called on construction)
			- Get the time with `timer.nanos()` ... `timer.days()`
			- Get the time and reset the timer with `timer.poll_<unit>()`
			- Get the time since epoch with `stx::timer::<unit>s_now()`
- `plaform.hpp`
	- A header which will contain various information about the current compiler
		- `STX_OS_(UNIX|LINUX|WINDOWS|MACOSX)`
			- `STX_OS_UNIX` is defined on Linux & Macosx
		- `STX_COMPILER_(GCC|CLANG|MSCV)`
		- `STX_ARCHITECTURE_(ARM|x64|x86)`
		- `STX_BYTEORDER_(BIG|LITTLE)_ENDIAN`
		- `STX_FORCEINLINE`
		- `STX_CONSTEXPR` (More lax constexpr)
- `handle.hpp`
	- `stx::handle_socket`
		- An interface for attachment to a handle
	- `stx::handle`
		- A list of handle_sockets, which are notified if the handle is destroyed
- `list.hpp` `list_mt.hpp`
	- `stx::list_element<T>`
		- Usage: `class my_class : public stx::list_element<my_class>`
		- Utility class for single directional lists (Used by handle for example)
	- `stx::list_element_mt<T>`
		- A thread-safe version of `list_element<T>`
- `shared_lib.hpp`
	- `stx::shared_lib`
		- A .dll or .so library
		- Load a library with `lib.load("<path>", stx::shared_lib::flag | ...)`
		- Get a pointer to a symbol with `lib.get<T>(const char* name)`
- `string.hpp`
	- `stx::token`
		- A non-owning string (Basically a `const char* begin` and a `size_t length`)
	- `stx::string`
		- A owning string, which extends stx::token
- `utf8.hpp`
	- `unsigned utf32to8(uint32_t codepoint, char* out)`
		- Convert codepoint to utf8 and write it to out. Returns how many bytes were written (Up to 4)
	- `unsigned utf8to32(const char* in, uint32_t& out)`
		- Convert in to UTF32 write it to out and return how many bytes were read

## License
The code is licensed under the MIT license, see License.txt

## Building

Just include the headers.
The only exception currently is shared_lib, which should be trivial to build.

## Tests
Run the Makefile to run the tests (disclaimer: mediocre coverage).
