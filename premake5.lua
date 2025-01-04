workspace "FoxLang"
    architecture "x86_64"
    configurations { "Debug", "", "Release" }

project "FoxLang"
    kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	toolset "clang"
	targetdir "bin/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"

	--[[ if _OPTIONS["gcc"] then
		buildoptions { "-std=c++20" }
	elseif _OPTIONS["clang"] then
		buildoptions { "-std=c++20" }
	elseif _OPTIONS["msvc"] then
		buildoptions { "/std:c++20" }
	end ]]--

	files {
	  "src/**.cpp",
      "vendor/**.cpp",
	  "vendor/**.c",
	  "vendor/**.cc",
	}

	includedirs {
	  "src/",
	  "vendor/"
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
