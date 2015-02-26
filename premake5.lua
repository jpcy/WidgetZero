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
	warnings "Extra"
	startproject "Example 1 - Demo"
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
	pchheader "wz.h"
	pchsource "src/wz_pch.cpp"
	defines "WZ_USE_PCH"
	files { "src/*.*" }
	includedirs { "nanovg" }
		
-----------------------------------------------------------------------------

project "NanoVG"
	language "C"
	kind "StaticLib"
	files { "nanovg/*.*" }
	includedirs { "nanovg" }
		
-----------------------------------------------------------------------------

function createExampleProject(_title, _name, _file)
	project(_title)
		kind "WindowedApp"
		targetname("example_" .. _name)
		files { "examples/" .. _file, "examples/gl/*.*" }
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

createExampleProject("Example 1 - Demo", "demo", "Demo.cpp")
createExampleProject("Example 2 - Simple", "simple", "Simple.cpp")

-----------------------------------------------------------------------------

project "Example 3 - Custom Renderer"
	kind "WindowedApp"
	targetname "example_custom_renderer"
	files { "examples/CustomRenderer.cpp", "examples/tigr/*.*" }
	includedirs { "src", "examples/tigr" }
	links { "WidgetZero" }
	
	configuration "vs*"
		links { "d3d9" }
	
	configuration "vs2012"
		linkoptions { "/SAFESEH:NO" }
