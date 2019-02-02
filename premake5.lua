location_dir = "../build/"

include(location_dir .. "conanbuildinfo.premake.lua")

workspace("RPC")
	location(location_dir)
	conan_basic_setup()

	project("RPC")
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		targetdir = location_dir .. "bin/%{cfg.buildcfg}"

		files{
			"src/*",
		}

		defines{"_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS"}

		filter "configurations:Debug"
			defines { "DEBUG" }
			symbols "On"

		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"