project "Fmt"
	language "C++"
	kind "StaticLib"

	files { "fmt/*.cc" }
	includedirs { "./" }

	cppdialect "C++23"
	toolset "clang"
