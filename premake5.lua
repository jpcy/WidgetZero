config =
{
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
	if config.sdl2Path == nil then
		printf("Error: SDL2 path not set")
		os.exit(1)
	end
	
	os.mkdir("build");
	os.mkdir("build/bin_x86");
	os.mkdir("build/bin_x64");

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
	files { "src/*.*", "include/*.*" }
	
	includedirs
	{
		"include",
		config.nanovgPath .. "/src"
	}
	
	vpaths { ["*"] = { "src", "include" } }
		
-----------------------------------------------------------------------------

project "NanoVG"
	kind "StaticLib"
	files { config.nanovgPath .. "/src/*.*" }
	includedirs { config.nanovgPath .. "/src" }
		
-----------------------------------------------------------------------------

function createExampleProject(_name, _language)
	project("Example_" .. _name)
		language(_language)
		kind "WindowedApp"
		targetname("example_" .. _name)

		files
		{
			"examples/" .. _name .. "/*.*",
			"examples/gl/*.*"
		}
		
		includedirs
		{
			"include",
			"examples/gl",
			config.nanovgPath .. "/src"
		}
		
		configuration "linux"
			buildoptions { "`pkg-config --cflags sdl2`" }
			linkoptions { "`pkg-config --libs sdl2`" }
			links { "GL", "GLU" }
		configuration "vs*"
			includedirs(config.sdl2Path .. "/include")
			links { "glu32", "opengl32" }
		configuration { "vs*", "not x64" }
			libdirs(config.sdl2Path .. "/lib/x86")
		configuration { "vs*",  "x64" }
			libdirs(config.sdl2Path .. "/lib/x64")
		configuration {}
		
		links
		{
			"SDL2",
			"SDL2main",
			"NanoVG",
			"WidgetZero"
		}
		
		if _language == "C++" then
			configuration "vs2012"
				linkoptions { "/SAFESEH:NO" }
		end
end

-----------------------------------------------------------------------------

createExampleProject("demo", "C++")
createExampleProject("simple", "C")
