solution "WidgetZero"
	language "C"
	location "build"
	startproject "example"
	
	defines
	{
		"_CRT_SECURE_NO_DEPRECATE"
	}

	configurations { "Debug", "Release" }
	
	configuration "Debug"
		optimize "Debug"
		defines "_DEBUG"
		flags "Symbols"
				
	configuration "Release"
		optimize "Full"
		defines "NDEBUG"
	
-----------------------------------------------------------------------------

project "libwz"
	kind "StaticLib"

	files
	{
		"src/*.c",
		"src/*.h",
		"include/widgetzero/wz.h"
	}
	
	includedirs
	{
		"include"
	}
	
	configuration "Debug"
		targetdir "build/libwz_debug"
		objdir "build/libwz_debug"
				
	configuration "Release"
		targetdir "build/libwz_release"
		objdir "build/libwz_release"

-----------------------------------------------------------------------------

project "example"
	language "C++"
	kind "WindowedApp"
	targetname "example"
	
	defines
	{
		"_WIN32",
		"WIN32"
	}

	files
	{
		"include/widgetzero/wz.h",
		"example/*.cpp",
		"example/*.h",
	}
	
	includedirs
	{
		"include"
	}
	
	local sdlPath = os.getenv("SDL2_DEV_PATH")
	
	if sdlPath == nil then
		printf("Error: SDL2_DEV_PATH environment variable not set")
		os.exit(1)
	else
		includedirs(sdlPath .. "/include")
		
		configuration "not x64"
			libdirs(sdlPath .. "/lib/x86")
		configuration "x64"
			libdirs(sdlPath .. "/lib/x64")
		configuration {}
		
		links
		{
			"SDL2",
			"SDL2main"
		}
	end
	
	links
	{
		"libwz"
	}
	
	linkoptions
	{
		"/SAFESEH:NO" -- for MSVC2012
	}
	
	configuration "Debug"
		targetdir "build/bin"
		objdir "build/example_debug"
		
	configuration "Release"
		targetdir "build/bin"
		objdir "build/example_release"
