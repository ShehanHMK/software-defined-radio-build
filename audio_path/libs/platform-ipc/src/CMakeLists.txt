set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${AUDIO_PATH_PLATFORM_IPC_LIBRARY_OUTPUT_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${AUDIO_PATH_PLATFORM_IPC_LIBRARY_OUTPUT_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AUDIO_PATH_PLATFORM_IPC_LIBRARY_OUTPUT_DIR}/bin)   

add_library(ipc STATIC
    ipc.cpp
)

target_include_directories(ipc
    PUBLIC
        ${CMAKE_SOURCE_DIR}/audio_path/libs/platform-ipc
        ${CMAKE_BINARY_DIR}/generated
)

set_project_warnings(ipc)