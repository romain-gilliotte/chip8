# Chip8 Emulator

This repository is a toy project so don't expect everything to work.

It aims to be a working interpreter and a dynamic recompiler for Chip8 in C.

# Goals

This project's goal is to learn modern C/C++ and associated tooling (build, delivery, dependency management, testing, ...).

As a warning, my last time using C for anything was C99 in school a decade ago.
The stack we used back then was Emacs, hand-crafted Makefiles, GCC and GDB, all in command line on FreeBSD computers. There was no dependency management, no unit testing and so on...

Be indulgent!

# Milestones

Some milestones for this project.

- [x] Basic build system with CMake
- [ ] Interpreter, graphics, input.
- [ ] Quality: Linter?
- [ ] Recompiler for a couple of supported instructions (it's ok to fallback to the interpreter all the time)
- [ ] Write NodeJS native library
- [ ] Write CPython native library
- [ ] Cross-compilation: Make this work in both windows / linux
- [ ] Delivery: Distro packages, flatpak, windows installers...
- [ ] Dependency management: Nuget? Conan? How to use/bundle recent versions of dependencies? npm's package.json equivalent?
- [ ] Unit testing: Google Test? CTest?
- [ ] CI/CD: What are the industry standart CI/CD systems for C/C++?
- [ ] Recompiler supporting most instructions
- [ ] Use LLVM instead of x86?

# Doc

## Build & Run

```sh
# In the project folder
mkdir build
cd build
cmake ..
make
./emulator "your_rom.ch8"
```

# Resources

This section contains resources I'm using.

## About Chip8 itself

http://emulator101.com/
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

## About Dependencies

### SDL

https://benedicthenshaw.com/soft_render_sdl2.html

## About Tooling

https://www.jetbrains.com/lp/devecosystem-2020/c/

### CMake

https://cmake.org/documentation/

https://www.youtube.com/playlist?list=PLK6MXr8gasrGmIiSuVQXpfFuE1uPT615s

### Nuget

https://www.nuget.org/packages/sdl2/
