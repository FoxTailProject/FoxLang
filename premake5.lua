workspace "FoxLang"
	architecture "x86_64"
	configurations { "Debug", "Release" }
	platforms { "Linux", "Windows" }

group "Deps"
	include "vendor/premake"
group ""

project "FoxLang"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	targetdir "bin/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"
	buildoptions { "-fmodules" }
	-- libdirs { os.findlib("libLLVM") }
	links { "LLVM", "Fmt" }

	postbuildcommands {
	  "doxygen > /dev/null &",
	}

	files {
	  "src/**.cpp",
	  "vendor/**.cpp",
	}

	includedirs {
	  "src/",
	  "vendor/",
	  "/usr/include/llvm/"
	}

	filter "platforms:Linux"
		toolset "gcc"
		system "Linux"

	filter "platforms:Windows"
		toolset "mingw"
		system "Windows"
		buildoptions "-fmodules"

	filter "configurations:Debug"
		defines { "FOX_DEBUG" }
		symbols "On"
		warnings "Extra"

	filter "configurations:Release"
		defines { "FOX_RELEASE" }
		optimize "On"
		warnings "Extra"

include "premake/actions.lua"
