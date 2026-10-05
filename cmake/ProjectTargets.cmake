include(CMakeParseArguments)
include(CompilerWarnings)


function(add_project_target_library TARGET_NAME)

  set(valid_library_types
    STATIC
    SHARED
    OBJECT
    MODULE
  )

  if(NOT TYPE IN_LIST valid_library_types)
    message(FATAL_ERROR
        "Invalid library type '${TYPE}' for target '${TARGET_NAME}'"
    )
  endif()

  set(options)
  set(oneValueArgs
    TYPE
  )
  set(multiValueArgs
    SOURCES
    PUBLIC_INCLUDES
    PRIVATE_INCLUDES
    PUBLIC_LIBS
    PRIVATE_LIBS
  )

  cmake_parse_arguments(
    ARG
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
  )

  add_library(${TARGET_NAME} ${TYPE}
    ${ARG_SOURCES}
  )

  target_include_directories(${TARGET_NAME}
    PUBLIC
      ${ARG_PUBLIC_INCLUDES}
    PRIVATE
      ${ARG_PRIVATE_INCLUDES}
  )

  set_project_warnings(${TARGET_NAME})

endfunction()


function()
