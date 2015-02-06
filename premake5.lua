if os.get() == "windows" then
	os.mkdir("build");
	os.mkdir("build/bin_x86");
	os.mkdir("build/bin_x64");

	-- Copy the SDL2 dlls to the build directory.
	os.copyfile("examples/sdl/lib/x86/SDL2.dll", "build/bin_x86/SDL2.dll");
	os.copyfile("examples/sdl/lib/x64/SDL2.dll", "build/bin_x64/SDL2.dll");
end

-----------------------------------------------------------------------------

solution "WidgetZero"
	language "C++"
	location "build"
	startproject "Example_demo"
	
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
	pchheader "wz_internal.h"
	pchsource "src/wz_pch.cpp"
	files { "src/*.*" }
	includedirs { "nanovg" }
	configuration "vs*"
		-- disable warning "new behavior: elements of array 'array' will be default initialized"
		buildoptions { "/wd\"4351\"" }
		
-----------------------------------------------------------------------------

project "NanoVG"
	language "C"
	kind "StaticLib"
	files { "nanovg/*.*" }
	includedirs { "nanovg" }
		
-----------------------------------------------------------------------------

function createExampleProject(_name)
	project("Example_" .. _name)
		kind "WindowedApp"
		targetname("example_" .. _name)
		files { "examples/" .. _name .. "/*.*", "examples/gl/*.*" }
		includedirs { "src", "nanovg", "examples/gl" }
		
		configuration "linux"
			buildoptions { "`pkg-config --cflags sdl2`" }
			linkoptions { "`pkg-config --libs sdl2`" }
			links { "GL", "GLU" }
		configuration "vs*"
			includedirs("examples/sdl/include")
			links { "glu32", "opengl32" }
		configuration { "vs*", "not x64" }
			libdirs("examples/sdl/lib/x86")
		configuration { "vs*",  "x64" }
			libdirs("examples/sdl/lib/x64")
		configuration {}
		
		links { "SDL2", "SDL2main", "NanoVG", "WidgetZero" }
		
		configuration "vs2012"
			linkoptions { "/SAFESEH:NO" }
end

-----------------------------------------------------------------------------

createExampleProject("demo")
createExampleProject("simple")
