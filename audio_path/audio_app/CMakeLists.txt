find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET portaudio-2.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/audio_path/bin)

# audio_tx_app
add_executable(audio_tx_app
    audio_tx_app.cpp
)

target_include_directories(audio_tx_app
    PRIVATE
    ${CMAKE_SOURCE_DIR}/audio_path
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(audio_tx_app
    PRIVATE
    sdr_audio_codec
    ipc
    PkgConfig::PORTAUDIO
)

set_project_warnings(audio_tx_app)


# audio_rx_app
add_executable(audio_rx_app
    audio_rx_app.cpp
)

target_include_directories(audio_rx_app
    PRIVATE
    ${CMAKE_SOURCE_DIR}/audio_path
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(audio_rx_app
    PRIVATE
    sdr_audio_codec
    ipc
    PkgConfig::PORTAUDIO
)

set_project_warnings(audio_rx_app)
