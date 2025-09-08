# FoxLang Compiler

Welcome to the FoxLang Compiler! This is the official compiler for the FoxLang language, and a source of fundamental truth for the design of the language. The language was inspired by a clip of Freya Holmer commenting on the argument on tab characters versus space characters when used for indentation that can be found [here](https://www.youtube.com/watch?v=DMmz2XXiHB0). The final argument is that there should have been a seperate, multi-character width characted selected for indentation. FoxLang ends the argument between spacens and tabs by making indentation use the fox emoji (🦊). Tab characters can still be used for alignment.

## Planned features

This list is roughly in order of implementation

 - [X] Functions
 - [ ] Structs (in progress)
 - [ ] Type checking (in progress)
 - [ ] Member functions (still deciding whether to include in FoxLang or not)
 - [ ] Modules
 - [ ] Including files
 - [ ] Multi-threading + general code cleanup
 - [ ] Macro & compile-time execution
 - [ ] Plugin system (possibly for extending language systems, like rust proc macros for custom parsing)
 - [ ] C/C++ support & external defs (I would like strong c++ support, unlike rust)

## Building

This builds using premake5, which is currently still in beta. As many dependencies are bundled in this repo, but that was not possible with llvm.

### Dependencies

 - `LLVM` >= 19
 - `Clang` >= 20

### Build steps

 > See (https://premake.github.io) for more information on how to use premake
 
Use premake to generate buildfiles, and use the buildfiles to build the project.
 
The current windows premake setup uses `Clang` for `{Fmt}` and `msvc` for the compiler, but this can be changed. `Clang` is required for `{Fmt}`. The current build setup has only been tested on Linux.

The plan is to eventually self-vendor all dependencies to a config directory to allow for easier building, and allow easy cross compiling with the FoxLang compiler.

## Exra

Actually I'm just learning that there is a list of every language and its code block style in [the github linguist repo](https://github.com/github-linguist/linguist/blob/main/lib/linguist/languages.yml)
