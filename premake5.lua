config =
{
	glewPath = nil,
	nanovgPath = nil,
	sdl2Path = nil
}

local configFilename = "config.lua"

if not os.isfile(configFilename) then
	printf("Error: config file \"" .. configFilename .. "\" doesn't exist")
	os.exit(1)
else
	dofile(configFilename)
end

if os.get() == "windows" then
	if config.glewPath == nil then
		printf("Error: GLEW path not set")
		os.exit(1)
	end

	if config.sdl2Path == nil then
		printf("Error: SDL2 path not set")
		os.exit(1)
	end
	
	os.mkdir("build");
	os.mkdir("build/bin_x86");
	os.mkdir("build/bin_x64");

	-- Copy the GLEW dlls to the build directory.
	os.copyfile(config.glewPath .. "/bin/Release/Win32/glew32.dll", "build/bin_x86/glew32.dll");
	os.copyfile(config.glewPath .. "/bin/Release/x64/glew32.dll", "build/bin_x64/glew32.dll");
	
	-- Copy the SDL2 dlls to the build directory.
	os.copyfile(config.sdl2Path .. "/lib/x86/SDL2.dll", "build/bin_x86/SDL2.dll");
	os.copyfile(config.sdl2Path .. "/lib/x64/SDL2.dll", "build/bin_x64/SDL2.dll");
end

if config.nanovgPath == nil then
	printf("Error: NanoVG path not set")
	os.exit(1)
end

-----------------------------------------------------------------------------

solution "WidgetZero"
	language "C"
	location "build"
	startproject "Example"
	
	configurations { "Debug", "Release" }
	platforms { "native", "x64", "x32" }
	
	configuration "Debug"
		optimize "Debug"
		defines "_DEBUG"
		flags "Symbols"
				
	configuration "Release"
		optimize "Full"
		defines "NDEBUG"
		
	configuration "vs*"
		defines { "_CRT_SECURE_NO_DEPRECATE" }
		
	configuration { "not x64" }
		targetdir "build/bin_x86"

	configuration { "x64" }
		targetdir "build/bin_x64"
	
-----------------------------------------------------------------------------

project "WidgetZero"
	kind "StaticLib"
	files { "src/*.*", "include/config.h" }
	includedirs { "include" }
		
-----------------------------------------------------------------------------

project "WidgetZeroCpp"
	language "C++"
	kind "StaticLib"

	files
	{
		"addons/wzcpp/src/*.cpp",
		"addons/wzcpp/include/*.h"
	}
	
	includedirs
	{
		"include",
		"addons/shared/include",
		"addons/wzcpp/include"
	}
	
	configuration "vs2012"
		linkoptions { "/SAFESEH:NO" }
		
-----------------------------------------------------------------------------

project "WidgetZeroGL"
	kind "StaticLib"

	files
	{
		"addons/wzgl/src/*.c",
		"addons/wzgl/include/*.h"
	}
	
	includedirs
	{
		"include",
		"addons/shared/include",
		"addons/wzgl/include",
		config.glewPath .. "/include",
		config.nanovgPath .. "/src"
	}
		
-----------------------------------------------------------------------------

project "NanoVG"
	kind "StaticLib"
	files { config.nanovgPath .. "/src/*.*" }
	includedirs { config.nanovgPath .. "/src" }
		
-----------------------------------------------------------------------------

project "Example"
	language "C++"
	kind "WindowedApp"
	targetname "example"

	files { "example/*.cpp" }
	
	includedirs
	{
		"include",
		"addons/shared/include",
		"addons/wzcpp/include",
		"addons/wzgl/include"
	}
	
	configuration "linux"
		buildoptions { "`pkg-config --cflags sdl2`" }
		linkoptions { "`pkg-config --libs sdl2`" }
		links { "GL", "GLU", "GLEW" }
    configuration "vs*"
	    includedirs(config.sdl2Path .. "/include")
		links { "glu32", "opengl32", "glew32" }
	configuration { "vs*", "not x64" }
		libdirs(config.sdl2Path .. "/lib/x86")
		libdirs(config.glewPath .. "/lib/Release/Win32")
	configuration { "vs*",  "x64" }
		libdirs(config.sdl2Path .. "/lib/x64")
		libdirs(config.glewPath .. "/lib/Release/x64")
	configuration {}
	
	links
	{
		"SDL2",
		"SDL2main",
		"NanoVG",
		"WidgetZero",
		"WidgetZeroCpp",
		"WidgetZeroGL"
	}
	
	configuration "vs2012"
		linkoptions { "/SAFESEH:NO" }
