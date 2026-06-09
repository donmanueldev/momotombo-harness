include(FetchContent)

function(redisforge_configure_dependencies)
    find_package(Threads REQUIRED)

    if(BUILD_TESTING)
        find_package(GTest QUIET)

        if(NOT GTest_FOUND)
            if(NOT REDISFORGE_FETCH_MISSING_DEPENDENCIES)
                message(FATAL_ERROR "GoogleTest not found. Enable REDISFORGE_FETCH_MISSING_DEPENDENCIES or install GTest.")
            endif()

            FetchContent_Declare(
                googletest
                URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            )
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(googletest)
        endif()
    endif()

    find_path(HIREDIS_INCLUDE_DIR NAMES hiredis/hiredis.h)
    find_library(HIREDIS_LIBRARY NAMES hiredis)

    if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
        if(NOT TARGET hiredis::hiredis)
            add_library(hiredis::hiredis UNKNOWN IMPORTED)
            set_target_properties(hiredis::hiredis PROPERTIES
                IMPORTED_LOCATION "${HIREDIS_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${HIREDIS_INCLUDE_DIR}"
            )
        endif()
        return()
    endif()

    if(NOT REDISFORGE_FETCH_MISSING_DEPENDENCIES)
        message(FATAL_ERROR "hiredis not found. Enable REDISFORGE_FETCH_MISSING_DEPENDENCIES or install hiredis.")
    endif()

    FetchContent_Declare(
        hiredis
        GIT_REPOSITORY https://github.com/redis/hiredis.git
        GIT_TAG v1.2.0
    )
    FetchContent_MakeAvailable(hiredis)

    if(TARGET hiredis AND NOT TARGET hiredis::hiredis)
        add_library(hiredis::hiredis ALIAS hiredis)
    endif()

    if(NOT TARGET hiredis::hiredis)
        message(FATAL_ERROR "Failed to configure hiredis target.")
    endif()
endfunction()
