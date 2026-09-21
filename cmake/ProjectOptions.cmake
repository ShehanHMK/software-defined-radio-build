set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


option(ENABLE_WARNINGS "Enable compiler warnings" ON)
option(ENABLE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

option(ENABLE_CPPLINT "Enable cpplint" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)


#option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
#option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

#option(ENABLE_COVERAGE "Enable code coverage" OFF)

#option(ENABLE_IPO "Enable interprocedural optimization" OFF)

#option(BUILD_TESTS "Build unit tests" ON)
#option(BUILD_BENCHMARKS "Build benchmarks" OFF)