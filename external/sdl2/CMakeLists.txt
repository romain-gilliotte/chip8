cmake_minimum_required(VERSION 3.13)

find_package(SDL2 REQUIRED)

# add_library(sdl2 SHARED IMPORTED GLOBAL)

# set_target_properties(
#     sdl2
#     PROPERTIES
#     INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
#     
#     # Hardcode the path, because SDL2's config script does not export it...
#     IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libSDL2.so" 
# )
