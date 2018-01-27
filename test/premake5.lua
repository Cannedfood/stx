workspace "test"

language "C++"
cppdialect "C++17"

configurations "test"

project "test.run" targetdir "."
	kind "ConsoleApp"
	files { "*.cpp", "../src/*.cpp" }
	links {
		"dl",
		"pthread",
		"sqlite3"
	}
	optimize "Debug"
	symbols "On"
