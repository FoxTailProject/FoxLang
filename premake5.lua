workspace "FoxLang"
    architecture "x86_64"
    configurations { "Debug", "", "Release" }

project "FoxLang"
    kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"

	files {
	  "src/**.hpp",
	  "src/**.h",
	  "src/**.cpp",
	  "vendor/**.hpp",
	  "vendor/**.h"
	}

	includedirs {
	  "src",
	  "vendor"
	}

	filter "configurations:Debug"
	    defines { "FOX_DEBUG" }
		symbols "On"

	filter "configurations:Release"
	    defines { "FOX_RELEASE" }
		optimize "On"
