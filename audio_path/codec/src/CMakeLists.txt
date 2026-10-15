add_project_target_library(sdr-audiocodec STATIC
  SOURCES
    audio_codec_factory.cpp
    ima_adpcm_codec.cpp
  PUBLIC_INCLUDES
    ${CMAKE_SOURCE_DIR}/audio_path/codec
    ${CMAKE_BINARY_DIR}/generated
)
