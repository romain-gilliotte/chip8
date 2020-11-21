# Chip8 Emulator

![Preview](https://repository-images.githubusercontent.com/314837107/bf477a00-2c15-11eb-989b-ff0e10f526fe)

This repository is a toy project so don't expect everything to work.

It aims to be a working interpreter and a dynamic recompiler for Chip8 in C.

## Goals

As a web developer the goal is to learn new things and do something a bit different than Python, NodeJS, ReactJS and the likes.

Also, this should allow having a better understanding of how JIT compilers like V8 and SpiderMonkey work (besides "Magic makes them faster than interpreters").

Last time using C was C99 in school a decade ago.

The stack we used back then was Emacs, hand-crafted Makefiles, GCC and GDB, all in command line on FreeBSD computers. There was nothing for dependency management and no unit testing.

## Milestones

Some milestones for this project.

- [x] Basic build system with CMake
- [ ] Interpreter, graphics, input.
- [ ] Recompiler for a couple of supported instructions (it's ok to fallback to the interpreter all the time)
- [ ] Cross-compilation: Could this work in windows?
- [ ] Delivery: Distro packages, flatpak, windows installers...
- [ ] Dependency management: Nuget? Conan?
- [ ] Unit testing: Google Test? CTest?
- [ ] Recompiler supporting most instructions
- [ ] Write CPython native library
- [ ] Write NodeJS native library
- [ ] Use LLVM instead of x86?

## Resources

This section contains resources I'm using.

### Chip8

http://emulator101.com/

### Tooling

https://www.jetbrains.com/lp/devecosystem-2020/c/

#### CMake

https://cmake.org/documentation/

https://www.youtube.com/playlist?list=PLK6MXr8gasrGmIiSuVQXpfFuE1uPT615s

#### Nuget

https://www.nuget.org/packages/sdl2/

### Dependencies

#### SDL

https://benedicthenshaw.com/soft_render_sdl2.html
