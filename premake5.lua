local sdlPath = ""
	
if os.get() == "windows" then
    sdlPath = os.getenv("SDL2_DEV_PATH")

    if sdlPath == nil then
    	printf("Error: SDL2_DEV_PATH environment variable not set")
	    os.exit(1)
    end
end

solution "WidgetZero"
	language "C"
	location "build"
	startproject "example"
	
	configurations { "Debug", "Release" }
	
	configuration "Debug"
		optimize "Debug"
		defines "_DEBUG"
		flags "Symbols"
				
	configuration "Release"
		optimize "Full"
		defines "NDEBUG"
		
	configuration "vs*"
		defines
		{
			"_CRT_SECURE_NO_DEPRECATE"
		}
	
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

project "libwzcpp"
	language "C++"
	kind "StaticLib"

	files
	{
		"addons/wzcpp/src/*.*",
		"addons/wzcpp/include/widgetzero/*.*"
	}
	
	includedirs
	{
		"include",
		"addons/shared/include",
		"addons/wzcpp/include"
	}
	
	configuration "vs2012"
		linkoptions
		{
			"/SAFESEH:NO"
		}
	
	configuration "Debug"
		targetdir "build/libwzcpp_debug"
		objdir "build/libwzcpp_debug"
				
	configuration "Release"
		targetdir "build/libwzcpp_release"
		objdir "build/libwzcpp_release"
		
-----------------------------------------------------------------------------

project "libwzsdl2"
	kind "StaticLib"

	files
	{
		"addons/wzsdl2/src/*.*",
		"addons/wzsdl2/include/widgetzero/*.*"
	}
	
	includedirs
	{
		"include",
		"addons/shared/include",
		"addons/wzsdl2/include"
	}
	
	configuration "vs2012"
		linkoptions
		{
			"/SAFESEH:NO"
		}
	
	configuration "linux"
		buildoptions { "`pkg-config --cflags sdl2`" }
    configuration "vs*"
	    includedirs(sdlPath .. "/include")
	configuration {}
	
	configuration "Debug"
		targetdir "build/libwzsdl2_debug"
		objdir "build/libwzsdl2_debug"
				
	configuration "Release"
		targetdir "build/libwzsdl2_release"
		objdir "build/libwzsdl2_release"

-----------------------------------------------------------------------------

project "example"
	language "C++"
	kind "WindowedApp"
	targetname "example"

	files
	{
		"example/*.*"
	}
	
	includedirs
	{
		"include",
		"addons/shared/include",
		"addons/wzcpp/include",
		"addons/wzsdl2/include"
	}
	
	configuration "linux"
		buildoptions { "`pkg-config --cflags sdl2`" }
		linkoptions { "`pkg-config --libs sdl2`" }
    configuration "vs*"
	    includedirs(sdlPath .. "/include")
	configuration { "vs*", "not x64" }
		libdirs(sdlPath .. "/lib/x86")
	configuration { "vs*",  "x64" }
		libdirs(sdlPath .. "/lib/x64")
	configuration {}
	
	links
	{
		"SDL2",
		"SDL2main",
		"libwz",
		"libwzcpp",
		"libwzsdl2"
	}
	
	configuration "vs2012"
		linkoptions
		{
			"/SAFESEH:NO"
		}
	
	configuration "Debug"
		targetdir "build/bin"
		objdir "build/example_debug"
		
	configuration "Release"
		targetdir "build/bin"
		objdir "build/example_release"
