find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET portaudio-2.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AUDIO_PATH_CODEC_LIBRARY_OUTPUT_DIR}/examples)   

add_executable(audio_loopback
    audio_loopback.cpp
)


target_include_directories(audio_loopback
    PRIVATE
    ${CMAKE_SOURCE_DIR}/audio_path/libs/codec
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(audio_loopback
    PRIVATE
    sdr_audio_codec
    PkgConfig::PORTAUDIO
)

set_project_warnings(audio_loopback)