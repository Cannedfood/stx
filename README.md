# STX

Stuff that's unfortunately not in the STL and more

## Contents

|Parsers        |                                                       | Notes |
|---------------|-------------------------------------------------------|-------|
|`xml.hpp`      | A xml parser                                          | No longer maintained, I use [pugi xml](https://github.com/zeux/pugixml) instead |
|`csv.hpp`      | A csv parser                                          |       |
|`config.hpp`   | A key-value store and .ini parser                     |       |

|Architecture   |                                                       | Notes |
|---------------|-------------------------------------------------------|-------|
|`injector.hpp` | A utility for dependency injection                    |       |
|`config.hpp`   | A key-value store and .ini parser                     |       |

|STL-Likes (containers and well-behaved types) |                                                  | Notes |
|------------------|------------------------------------------------------------------------------|-------|
|`shared.hpp`      | A re-imagining of std::shared_ptr (deals better with enable_shared_from_this)|       |
|`event.hpp`       | An event (signal-slot like)                                                  |       |
|`random.hpp`      | Easy random numbers (using std::random)                                      |       |
|`hash.hpp`        | Various has algorithm (actually only fnv-1a)                                 |       |
|`scoped.hpp`      | Execute stuff at end of scope                                                |       |
|`range.hpp`       | Iterator stuff (`cast_iterator` etc.)                                        |       |
|`guid.hpp`        | Guid                                                                         | not using proper generation algorithm |
|`timer.hpp`       | Stopwatch based on std::chrono                                               |       |
|`bitmap(3d).hpp`  | Bitmap views with some pixel operations (blitting etc.)                      |       |
|`bloom_filter.hpp`| A bloom filter                                                               |       |

|Platform abstractions|                                                       | Notes |
|---------------------|-------------------------------------------------------|-------|
|`shared_lib.hpp`     | Load shared libraries at runtime                      |       |

|Quick utilities   |                                                             | Notes |
|------------------|-------------------------------------------------------------|-------|
|`file2vector.hpp` | Read files into vectors and strings                         |       |
|`type.hpp`        | type_info::name demangling                                  |       |
|`allocator.hpp`   | Various allocators (not in the STL-sense of the word)       |       |
|`cache.hpp`       | Small utility for quickly implementing a very simple cache  |       |

## License
All files in this repository are either licensed under the MIT or CC0 license.
If not Noted otherwise they are licensed under MIT. (see LICENSE.txt)

## Building

Just include the headers.
The only exception currently is shared_lib, which should be trivial to build.

## Tests
Run the Makefile to run the tests (disclaimer: mediocre coverage).
