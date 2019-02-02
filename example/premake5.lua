newoption {
   trigger     = "location",
   value       = "./",
   description = "Where to generate the project.",
}

if not _OPTIONS["location"] then
   _OPTIONS["location"] = "./"
end

-- include RPC lib's dependencies
include("../../build/conanbuildinfo.premake.lua")

workspace("RPC example")
	location(_OPTIONS["location"])
	conan_basic_setup()
	startproject "RPC example"

	externalproject "RPC"
		location "../../build/"
		kind "StaticLib"
		language "C++"

	project("RPC example")
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		targetdir = "bin/%{cfg.buildcfg}"
		
		files{
			"src/**",
		}

		includedirs{
			"../src"
		}
		links{"RPC"}
		defines{"_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS"}

		filter "configurations:Debug"
			defines { "DEBUG" }
			symbols "On"

		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"