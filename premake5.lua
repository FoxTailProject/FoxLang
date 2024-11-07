workspace "FoxLang"
    architecture "x86_64"
    configurations { "Debug", "", "Release" }

project "FoxLang"
    kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"

	files {
	  "src/**.cpp",
      "vendor/**.cpp",
      "vendor/**.hpp"
	}

	includedirs {
	  "src/**",
	  "src/**",
	  "vendor/**"
	}

	filter "configurations:Debug"
	    defines { "FOX_DEBUG" }
		symbols "On"
        warnings "Extra"

	filter "configurations:Release"
	    defines { "FOX_RELEASE" }
		optimize "On"
        warnings "Default"

include "premake/actions.lua"
