newoption {
	trigger     = 'location',
	description = "Where to generate the project files"
}

workspace "test"

language "C++"
cppdialect "C++17"

configurations "test"

if _OPTIONS.location then
	location(_OPTIONS.location)
end

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
