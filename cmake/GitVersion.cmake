find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(GIT_COMMIT_HASH "unknown")
    set(GIT_BRANCH "unknown")
endif()

message(STATUS "Git Branch: ${GIT_BRANCH}, Commit: ${GIT_COMMIT_HASH}")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/templates/git_info.h.in
    ${CMAKE_BINARY_DIR}/generated/git_info.h 
    @ONLY 
)
