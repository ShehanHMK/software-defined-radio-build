set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${AUDIO_PATH_CODEC_LIBRARY_OUTPUT_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${AUDIO_PATH_CODEC_LIBRARY_OUTPUT_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AUDIO_PATH_CODEC_LIBRARY_OUTPUT_DIR}/bin)   

add_library(sdr_audio_codec STATIC
    ima_adpcm_codec.cpp
)

target_include_directories(sdr_audio_codec
    PUBLIC
        ${CMAKE_SOURCE_DIR}/audio_path/libs/codec
        ${CMAKE_BINARY_DIR}/generated
)

set_project_warnings(sdr_audio_codec)