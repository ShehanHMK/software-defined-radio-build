find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET portaudio-2.0)


add_project_target_executable(audio_tx_app
  SOURCES
    audio_tx_app.cpp
  INCLUDES
    ${CMAKE_SOURCE_DIR}/audio_path
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/
  LIBS
    sdr-audiocodec
    PkgConfig::PORTAUDIO
    ipc
)

add_project_target_executable(audio_rx_app
  SOURCES
    audio_rx_app.cpp
  INCLUDES
    ${CMAKE_SOURCE_DIR}/audio_path
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/
  LIBS
    sdr-audiocodec
    PkgConfig::PORTAUDIO
    ipc
)

add_subdirectory(audio_stream_manager)
