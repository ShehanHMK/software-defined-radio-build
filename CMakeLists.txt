cmake_minimum_required(VERSION 3.10)

project(
    SDR 
    VERSION 1.0.0 
    LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(ProjectOptions)
include(CompilerWarnings)
include(GitVersion)
#include(Dependencies)
#include(Sanitizers)
include(StaticAnalyzers)

add_subdirectory(audio_path)
