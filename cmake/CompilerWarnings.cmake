function(set_project_warnings target)

    if(NOT ENABLE_WARNINGS)
        return()
    endif()

    set(MSVC_WARNINGS /W4 /WX)
    set(CLANG_GCC_WARNINGS
        -Wall                      # Enables broad set of common warnings.
        -Wextra                    # Enables additional warnings that -Wall does not.
        -Wpedantic                 # Warns when you use compiler extensions that are outside the official C/C++ standard.     
        -Wshadow                   # Warns when a variable hides another variable with the same name.
        -Wnon-virtual-dtor         # Warns when a class is intended to be used polymorphically, but its destructor is not virtual.
        -Wcast-align               # Warns about casts that may create an improperly aligned pointer.
        -Wunused                   # Enables warnings for unused things.
        -Woverloaded-virtual       # Catches cases where a derived-class function accidentally hides a virtual function from the base class instead of overriding it
        -Wnull-dereference         # Attempts to detect cases where you may dereference a null pointer.
    )

    if(ENABLE_WARNINGS_AS_ERRORS)
        list(APPEND CLANG_GCC_WARNINGS -Werror) 
    endif()

    if(MSVC)
        target_compile_options(${target} PRIVATE ${MSVC_WARNINGS})
    else()
        target_compile_options(${target} PRIVATE ${CLANG_GCC_WARNINGS})
    endif()
    
endfunction()