project "Fmt"
	language "C++"
	kind "StaticLib"

	files { "fmt/src/*.cc" }
	includedirs { "fmt" }

	cppdialect "C++23"
	toolset "clang"
