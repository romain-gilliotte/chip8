cmake_minimum_required(VERSION 3.13)

find_package(cmocka REQUIRED)

# Is find_package defining the cmocka target on it's own?
# Can't find it on the find-cmocka.cmake file... but it seems to work!

# add_library(cmocka SHARED IMPORTED GLOBAL)

# set_target_properties(
#     cmocka
#     PROPERTIES
#     INTERFACE_INCLUDE_DIRECTORIES "${CMOCKA_INCLUDE_DIRS}"
#     IMPORTED_LOCATION "${CMOCKA_LIBRARY}"
# )

