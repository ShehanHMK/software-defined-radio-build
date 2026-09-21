# Find cpplint and cppcheck binaries on systempath

# -----------------------------------------
# cpplint
# -----------------------------------------
if(ENABLE_CPPLINT)
    find_program(CPPLINT_BIN NAMES cpplint)

    if(CPPLINT_BIN)
        message(STATUS "cpplint enabled: ${CPPLINT_BIN}")

        set(CMAKE_CXX_CPPLINT
            ${CPPLINT_BIN}
        )
    else()
        message(WARNING "Cpplint enabled!, but cpplint was not found")
    endif()
endif()


# -----------------------------------------
# cppcheck
# -----------------------------------------
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_BIN NAMES cppcheck)

    if(CPPCHECK_BIN)
        message(STATUS "cppcheck enabled: ${CPPCHECK_BIN}")

        set(CMAKE_CXX_CPPCHECK
            ${CPPCHECK_BIN}
        )
    else()
        message(WARNING "Cppcheck enabled!, but cppcheck was not found")
    endif()
endif()


# -----------------------------------------
# clang-tidy
# -----------------------------------------
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_BIN NAMES clang-tidy)

    if(CLANG_TIDY_BIN)
        message(STATUS "clang-tidy enabled: ${CLANG_TIDY_BIN}")

        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY_BIN}
        )
    else()
        message(WARNING "clange-tidy enabled!, but clang-tidy was not found")
    endif()
endif()
