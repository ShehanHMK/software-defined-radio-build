add_project_target_executable(ipc_sender
  SOURCES
    ipc-sender.cpp
  INCLUDES
    ${CMAKE_SOURCE_DIR}/platform-ipc
    ${CMAKE_BINARY_DIR}/generated
  LIBS
    ipc
)


add_project_target_executable(ipc_receiver
  SOURCES
    ipc-receiver.cpp
  INCLUDES
    ${CMAKE_SOURCE_DIR}/platform-ipc
    ${CMAKE_BINARY_DIR}/generated
  LIBS
    ipc
)
