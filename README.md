# Chip8 Emulator

This repository is a toy project so don't expect everything to work.

It aims to be a working interpreter and a dynamic recompiler for Chip8 in C.

# Goals

We're in 2020, no one needs a chip8 emulator, and besides, dozens are already available on the internet for all kinds of platforms.

This project's goal is to learn modern C/C++ tooling (build, delivery, dependency management, testing, ...).

As a warning, my last time using C for anything was C99 in school a decade ago.
The stack we used back then was Emacs, hand-crafted Makefiles, GCC and GDB, all in command line on FreeBSD computers. There was no dependency management, no unit testing and so on...

Be indulgent!

# Milestones

Some milestones for this project.

- [x] Basic build system with CMake
- [x] Unit testing
- [x] Interpreter: CPU
- [x] Interpreter: graphics
- [x] Interpreter: input
- [ ] Quality: Linter?
- [x] Recompiler supporting some instructions written by hand
- [ ] Write NodeJS native library
- [ ] Write CPython native library
- [ ] Cross-compilation: Make this work in both windows / linux
- [ ] Delivery: Distro packages, flatpak, windows installers...
- [ ] Dependency management: Nuget? Conan? How to use/bundle recent versions of dependencies? npm's package.json equivalent?
- [ ] CI/CD: What are the industry standart CI/CD systems for C/C++?
- [ ] Recompiler using asmjit?
- [ ] Recompiler using LLVM?

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

http://mattmik.com/files/chip8/mastering/chip8.html

## About Emulation

### x64 instruction encoding

http://www.c-jump.com/CIS77/CPU/x86/lecture.html#X77_0010_real_encoding

http://ref.x86asm.net/coder64.html#modrm_byte_32_64

https://defuse.ca/online-x86-assembler.htm#disassembly

https://wiki.osdev.org/X86-64_Instruction_Encoding#ModR.2FM_and_SIB_bytes

### x86 adressing

https://www.youtube.com/watch?v=t44pm0GzKvk

https://www.youtube.com/watch?v=XOnzjEd_dLg

### JIT

https://eli.thegreenplace.net/2013/11/05/how-to-jit-an-introduction

https://eli.thegreenplace.net/2017/adventures-in-jit-compilation-part-1-an-interpreter/

https://eli.thegreenplace.net/2017/adventures-in-jit-compilation-part-2-an-x64-jit/

https://eli.thegreenplace.net/2017/adventures-in-jit-compilation-part-3-llvm/

https://eli.thegreenplace.net/2017/adventures-in-jit-compilation-part-4-in-python/

http://www.multigesture.net/wp-content/uploads/mirror/zenogais/Dynamic%20Recompiler.html

## About Dependencies

### SDL

https://benedicthenshaw.com/soft_render_sdl2.html

## About Tooling

https://www.jetbrains.com/lp/devecosystem-2020/c/

### CMake

https://cmake.org/documentation/

https://www.youtube.com/playlist?list=PLK6MXr8gasrGmIiSuVQXpfFuE1uPT615s

https://www.youtube.com/watch?v=y7ndUhdQuU8

https://www.youtube.com/watch?v=y9kSr5enrSk

https://cliutils.gitlab.io/modern-cmake/

https://cgold.readthedocs.io/en/latest/

### Cmocka

https://api.cmocka.org/group__cmocka__exec.html#ga7c62fd0acf2235ce98268c28ee262a57

### Nuget

https://www.nuget.org/packages/sdl2/
