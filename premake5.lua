workspace 'stx'

language   'C++'
cppdialect 'C++17'

configurations {
	'dev',
	'debug',
	'release',
}

filter 'configurations:release or dev'
	optimize 'Speed'
filter 'configurations:dev or debug'
	symbols 'On'
filter 'configurations:debug'
	optimize 'Debug'
filter 'configurations:release'
	flags 'LinkTimeOptimization'
filter {}

filter { 'configurations:dev or debug', 'toolset:gcc or clang' }
	buildoptions { '-Wall', '-Wextra', '-Wno-unused-parameter' }
filter {}

flags { 'MultiProcessorCompile', 'NoIncrementalLink' }
vectorextensions 'SSE2'
floatingpoint 'Fast'

includedirs 'include/'

project 'stx'
	kind 'SharedLib'
	files 'include/stx/src/**.cpp'
	links {'dl', 'pthread'}

project 'test'
	kind 'ConsoleApp'
	files 'test/unit/**.cpp'
	links 'stx'

project 'demo-game'
	kind 'ConsoleApp'
	files 'test/demo-game/**.cpp'
	links 'stx'
