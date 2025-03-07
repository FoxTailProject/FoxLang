workspace "FoxLang"
    architecture "x86_64"
    configurations { "Debug", "", "Release" }

project "FoxLang"
    kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	toolset "clang"
	targetdir "bin/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"
	-- libdirs { os.findlib("libLLVM") }
	links { "LLVM" }

	files {
	  "src/**.cpp",
      "vendor/**.cpp",
	  "vendor/**.c",
	  "vendor/**.cc",
	}

	includedirs {
	  "src/",
	  "vendor/",
	  "/usr/include/llvm/"
	}

	filter "configurations:Debug"
	    defines { "FOX_DEBUG" }
		symbols "On"
        warnings "Extra"

	filter "configurations:Release"
	    defines { "FOX_RELEASE" }
		optimize "On"
        warnings "Extra"

include "premake/actions.lua"
