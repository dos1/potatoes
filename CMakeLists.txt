cmake_minimum_required(VERSION 3.6)

project(NonCompetitiveSingingPotatoes C)

SET(LIBSUPERDERPY_GAMENAME "potatoes" CACHE INTERNAL "")
SET(LIBSUPERDERPY_GAMENAME_PRETTY "Non-Competitive Singing Potatoes" CACHE INTERNAL "")

SET(LIBSUPERDERPY_VENDOR "Holy Pangolin")
SET(LIBSUPERDERPY_APPID "net.dosowisko.potatoes" CACHE INTERNAL "")
SET(LIBSUPERDERPY_ORIENTATION "LANDSCAPE" CACHE INTERNAL "")
SET(LIBSUPERDERPY_VERSION "1.1" CACHE INTERNAL "")

set(EMSCRIPTEN_TOTAL_MEMORY "320" CACHE INTERNAL "")

set(FLACTOLOSSY OFF CACHE INTERNAL "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_SOURCE_DIR}/libsuperderpy/cmake")

include(libsuperderpy)

add_subdirectory(libsuperderpy)
add_subdirectory(src)
add_subdirectory(data)
