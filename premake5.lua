wz =
{
	glewPath = nil,
	nanovgPath = nil,
	sdl2Path = nil
}

local customFilename = "premake5-custom.lua"

if not os.isfile(customFilename) then
	printf("Error: file premake5-custom.lua doesn't exist")
	os.exit(1)
else
	dofile(customFilename)
end

if os.get() == "windows" then
	if wz.glewPath == nil then
		printf("Error: GLEW path not set")
		os.exit(1)
	end

	if wz.sdl2Path == nil then
		printf("Error: SDL2 path not set")
		os.exit(1)
	end
	
	os.mkdir("build");

	-- Copy the GLEW dlls to the build directory.
	os.copyfile(wz.glewPath .. "/bin/Release/Win32/glew32.dll", "build/glew32.dll");
	os.copyfile(wz.glewPath .. "/bin/Release/x64/glew32.dll", "build/glew64.dll");
	
	-- Copy the SDL2 dlls to the build directory.
	os.copyfile(wz.sdl2Path .. "/lib/x86/SDL2.dll", "build/SDL2.dll");
	os.copyfile(wz.sdl2Path .. "/lib/x64/SDL2.dll", "build/SDL2-64.dll");
end

if wz.nanovgPath == nil then
	printf("Error: NanoVG path not set")
	os.exit(1)
end

-----------------------------------------------------------------------------

solution "WidgetZero"
	language "C"
	location "build"
	startproject "Example"
	
	configurations { "Debug", "Release" }
	
	configuration "Debug"
		optimize "Debug"
		defines "_DEBUG"
		flags "Symbols"
				
	configuration "Release"
		optimize "Full"
		defines "NDEBUG"
		
	configuration "vs*"
		defines { "_CRT_SECURE_NO_DEPRECATE" }
	
-----------------------------------------------------------------------------

project "WidgetZero"
	kind "StaticLib"
	files { "src/*.*", "include/wz.h" }
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
		wz.glewPath .. "/include",
		wz.nanovgPath .. "/src"
	}
		
-----------------------------------------------------------------------------

project "NanoVG"
	kind "StaticLib"
	files { wz.nanovgPath .. "/src/*.*" }
	includedirs { wz.nanovgPath .. "/src" }
		
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
		links { "GL", "GLU" }
    configuration "vs*"
	    includedirs(wz.sdl2Path .. "/include")
		links { "glu32", "opengl32" }
	configuration { "vs*", "not x64" }
		libdirs(wz.sdl2Path .. "/lib/x86")
		libdirs(wz.glewPath .. "/lib/Release/Win32")
	configuration { "vs*",  "x64" }
		libdirs(wz.sdl2Path .. "/lib/x64")
		libdirs(wz.glewPath .. "/lib/Release/x64")
	configuration {}
	
	links
	{
		"GLEW",
		"SDL2",
		"SDL2main",
		"NanoVG",
		"WidgetZero",
		"WidgetZeroCpp",
		"WidgetZeroGL"
	}
	
	configuration "vs2012"
		linkoptions { "/SAFESEH:NO" }
