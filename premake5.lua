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

platforms {
	'x64',
	'x86',
	'ARM',
}
defaultplatform 'x64'

filter 'platforms:x64'
	architecture 'x86_64'
filter 'platforms:x86'
	architecture 'x86'
filter 'platforms:ARM'
	architecture 'ARM'
filter {}

flags { 'MultiProcessorCompile', 'NoIncrementalLink' }
vectorextensions 'SSE2'
floatingpoint 'Fast'

includedirs 'src/'

project 'stx'
	kind 'SharedLib'
	files 'src/**'
	filter 'system:not windows'
		links {'dl', 'pthread', 'stdc++fs'}
	filter {}

project 'test'
	kind 'ConsoleApp'
	files 'test/unit/**'
	links 'stx'
	filter 'system:not windows'
		links {'pthread'}
	filter {}
