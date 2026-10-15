find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET portaudio-2.0)


add_project_target_library(audio-stream-manager
  STATIC
  SOURCES
    audio_stream_manager.cpp
  PUBLIC_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_LIBS
    PkgConfig::PORTAUDIO
)
