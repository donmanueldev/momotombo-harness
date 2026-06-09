cmake_minimum_required(VERSION 3.24)

project(RedisForge VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTING "Build tests" ON)
option(REDISFORGE_FETCH_MISSING_DEPENDENCIES "Fetch missing dependencies with FetchContent" ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(CTest)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Concurrent)

redisforge_configure_dependencies()
qt_standard_project_setup()

set(REDISFORGE_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(redisforge_shared
        src/shared/logging/logger.h
        src/shared/di/service_container.h
        src/shared/pagination/page.h
        src/shared/result/operation_result.h
)
target_include_directories(redisforge_shared PUBLIC ${REDISFORGE_SRC_ROOT})
target_link_libraries(redisforge_shared PUBLIC Qt6::Core)

add_library(redisforge_domain
        src/domain/connections/entities/connection_profile.h
        src/domain/connections/entities/connection_test_result.h
        src/domain/connections/repositories/i_connection_profile_repository.h
        src/domain/connections/services/i_redis_connection_gateway.h
        src/domain/connections/services/connection_profile_validator.h
        src/domain/connections/services/connection_profile_validator.cpp
)
target_include_directories(redisforge_domain PUBLIC ${REDISFORGE_SRC_ROOT})
target_link_libraries(redisforge_domain PUBLIC redisforge_shared)

add_library(redisforge_app
        src/app/connections/services/active_connection_store.h
        src/app/connections/services/active_connection_store.cpp
        src/app/connections/use_cases/list_connection_profiles_use_case.h
        src/app/connections/use_cases/list_connection_profiles_use_case.cpp
        src/app/connections/use_cases/save_connection_profile_use_case.h
        src/app/connections/use_cases/save_connection_profile_use_case.cpp
        src/app/connections/use_cases/test_connection_use_case.h
        src/app/connections/use_cases/test_connection_use_case.cpp
        src/app/keys_explorer/use_cases/scan_keys_use_case.h
        src/app/keys_explorer/use_cases/scan_keys_use_case.cpp
)
target_include_directories(redisforge_app PUBLIC ${REDISFORGE_SRC_ROOT})
target_link_libraries(redisforge_app PUBLIC redisforge_domain redisforge_shared)

add_library(redisforge_infrastructure
        src/infrastructure/connections/hiredis_connection_gateway.h
        src/infrastructure/connections/hiredis_connection_gateway.cpp
        src/infrastructure/connections/in_memory_connection_profile_repository.h
        src/infrastructure/connections/in_memory_connection_profile_repository.cpp
        src/infrastructure/connections/redis_connection_session.h
        src/infrastructure/connections/redis_connection_session.cpp
        src/infrastructure/keys_explorer/hiredis_key_explorer_gateway.h
        src/infrastructure/keys_explorer/hiredis_key_explorer_gateway.cpp
        src/infrastructure/logging/qt_structured_logger.h
        src/infrastructure/logging/qt_structured_logger.cpp
)
target_include_directories(redisforge_infrastructure PUBLIC ${REDISFORGE_SRC_ROOT})
target_link_libraries(redisforge_infrastructure
        PUBLIC redisforge_domain redisforge_shared
        PRIVATE hiredis::hiredis Qt6::Core
)

add_library(redisforge_presentation
        src/presentation/main_window.h
        src/presentation/main_window.cpp
        src/presentation/connections/viewmodels/connection_management_view_model.h
        src/presentation/connections/viewmodels/connection_management_view_model.cpp
        src/presentation/connections/widgets/connection_management_widget.h
        src/presentation/connections/widgets/connection_management_widget.cpp
        src/presentation/keys_explorer/viewmodels/keys_explorer_view_model.h
        src/presentation/keys_explorer/viewmodels/keys_explorer_view_model.cpp
        src/presentation/keys_explorer/widgets/keys_explorer_widget.h
        src/presentation/keys_explorer/widgets/keys_explorer_widget.cpp
)
target_include_directories(redisforge_presentation PUBLIC ${REDISFORGE_SRC_ROOT})
target_link_libraries(redisforge_presentation
        PUBLIC redisforge_app redisforge_shared
        PRIVATE Qt6::Core Qt6::Widgets Qt6::Concurrent
)

qt_add_executable(RedisForge
        src/main.cpp
)
target_link_libraries(RedisForge
        PRIVATE
        redisforge_shared
        redisforge_domain
        redisforge_app
        redisforge_infrastructure
        redisforge_presentation
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
)

if (BUILD_TESTING)
    include(GoogleTest)

    add_executable(redisforge_tests
            tests/domain/connections/connection_profile_validator_tests.cpp
            tests/app/connections/save_connection_profile_use_case_tests.cpp
            tests/app/keys_explorer/scan_keys_use_case_tests.cpp
            tests/integration/hiredis_connection_gateway_integration_tests.cpp
    )
    target_include_directories(redisforge_tests PRIVATE ${REDISFORGE_SRC_ROOT})
    target_link_libraries(redisforge_tests
            PRIVATE
            redisforge_app
            redisforge_infrastructure
            GTest::gtest_main
    )

    gtest_discover_tests(redisforge_tests)
endif ()
