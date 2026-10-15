find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET portaudio-2.0)


add_project_target_executable(audio_loopback
  SOURCES
    audio_loopback.cpp
  INCLUDES
    ${CMAKE_SOURCE_DIR}/audio_path
  LIBS
    sdr-audiocodec
    PkgConfig::PORTAUDIO
)
