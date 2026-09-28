set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AUDIO_PATH_PLATFORM_IPC_LIBRARY_OUTPUT_DIR}/examples)   


# ipc sender example
add_executable(ipc_sender
    ipc-sender.cpp
)

target_include_directories(ipc_sender
    PRIVATE
    ${CMAKE_SOURCE_DIR}/audio_path/libs/platform-ipc
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(ipc_sender
    PRIVATE
    ipc 
)    

set_project_warnings(ipc)


# ipc receiver example
add_executable(ipc_receiver
    ipc-receiver.cpp
)


target_include_directories(ipc_receiver
    PRIVATE
    ${CMAKE_SOURCE_DIR}/audio_path/libs/platform-ipc
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(ipc_receiver
    PRIVATE
    ipc
)

set_project_warnings(ipc_receiver)
