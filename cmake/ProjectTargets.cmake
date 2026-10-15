include(CompilerWarnings)

# Helper function for building a custom library.
function(add_project_target_library TARGET_NAME TYPE)

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
  set(oneValueArgs)
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

  if(ARG_PUBLIC_INCLUDES)
    target_include_directories(${TARGET_NAME}
      PUBLIC
        ${ARG_PUBLIC_INCLUDES}
    )
  endif()

  if(ARG_PRIVATE_INCLUDES)
    target_include_directories(${TARGET_NAME}
      PRIVATE
        ${ARG_PRIVATE_INCLUDES}
    )
  endif()

  if(ARG_PUBLIC_LIBS)
    target_link_libraries(${TARGET_NAME}
      PUBLIC
        ${ARG_PUBLIC_LIBS}
    )
  endif()

  if(ARG_PRIVATE_LIBS)
    target_link_libraries(${TARGET_NAME}
      PRIVATE
        ${ARG_PRIVATE_LIBS}
    )
  endif()

  set_project_warnings(${TARGET_NAME})

endfunction()


# Helper function for building a custom executable.
function(add_project_target_executable TARGET_NAME)

  set(options)
  set(oneValueArgs)
  set(multiValueArgs
    SOURCES
    INCLUDES
    LIBS
  )

  cmake_parse_arguments(
    ARG
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
  )

  add_executable(${TARGET_NAME}
    ${ARG_SOURCES}
  )

  if(ARG_INCLUDES)
    target_include_directories(${TARGET_NAME}
      PRIVATE
        ${ARG_INCLUDES}
    )
  endif()

  if(ARG_LIBS)
    target_link_libraries(${TARGET_NAME}
      PRIVATE
        ${ARG_LIBS}
    )
  endif()

  set_project_warnings(${TARGET_NAME})

endfunction()
