add_project_target_library(ipc STATIC
  SOURCES
    ipc.cpp
  PUBLIC_INCLUDES
    ${CMAKE_SOURCE_DIR}/platform-ipc
    ${CMAKE_BINARY_DIR}/generated
)
