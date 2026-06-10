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
include(GNUInstallDirs)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Concurrent)

redisforge_configure_dependencies()
qt_standard_project_setup()

set(REDISFORGE_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(redisforge_shared
        src/shared/logging/logger.h
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
        src/domain/value_inspector/entities/key_inspection_result.h
        src/domain/value_inspector/services/i_value_inspector_gateway.h
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
        src/app/connections/use_cases/test_and_activate_connection_use_case.h
        src/app/connections/use_cases/test_and_activate_connection_use_case.cpp
        src/app/keys_explorer/use_cases/scan_keys_use_case.h
        src/app/keys_explorer/use_cases/scan_keys_use_case.cpp
        src/app/value_inspector/use_cases/inspect_key_use_case.h
        src/app/value_inspector/use_cases/inspect_key_use_case.cpp
        src/app/value_inspector/use_cases/save_string_value_use_case.h
        src/app/value_inspector/use_cases/save_string_value_use_case.cpp
        src/app/value_inspector/use_cases/save_hash_field_use_case.h
        src/app/value_inspector/use_cases/save_hash_field_use_case.cpp
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
        src/infrastructure/value_inspector/hiredis_value_inspector_gateway.h
        src/infrastructure/value_inspector/hiredis_value_inspector_gateway.cpp
)
target_include_directories(redisforge_infrastructure PUBLIC ${REDISFORGE_SRC_ROOT})
target_link_libraries(redisforge_infrastructure
        PUBLIC redisforge_domain redisforge_shared
        PRIVATE hiredis::hiredis Qt6::Core
)

add_library(redisforge_presentation
        src/bootstrap/application_bootstrap.h
        src/bootstrap/application_bootstrap.cpp
        src/presentation/main_window.h
        src/presentation/main_window.cpp
        src/presentation/connections/viewmodels/connection_management_view_model.h
        src/presentation/connections/viewmodels/connection_management_view_model.cpp
        src/presentation/connections/widgets/connection_management_widget.h
        src/presentation/connections/widgets/connection_management_widget.cpp
        src/presentation/keys_explorer/models/keys_table_model.h
        src/presentation/keys_explorer/models/keys_table_model.cpp
        src/presentation/keys_explorer/viewmodels/keys_explorer_view_model.h
        src/presentation/keys_explorer/viewmodels/keys_explorer_view_model.cpp
        src/presentation/keys_explorer/widgets/keys_explorer_widget.h
        src/presentation/keys_explorer/widgets/keys_explorer_widget.cpp
        src/presentation/value_inspector/models/value_entries_table_model.h
        src/presentation/value_inspector/models/value_entries_table_model.cpp
        src/presentation/value_inspector/viewmodels/value_inspection_view_data.h
        src/presentation/value_inspector/viewmodels/value_inspection_view_data_mapper.h
        src/presentation/value_inspector/viewmodels/value_inspection_view_data_mapper.cpp
        src/presentation/value_inspector/viewmodels/value_inspector_view_model.h
        src/presentation/value_inspector/viewmodels/value_inspector_view_model.cpp
        src/presentation/value_inspector/widgets/value_inspection_details_widget.h
        src/presentation/value_inspector/widgets/value_inspection_details_widget.cpp
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
set_target_properties(RedisForge PROPERTIES
        OUTPUT_NAME "RedisForge"
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "RedisForge"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.redisforge.desktop"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)

install(TARGETS RedisForge
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
        TARGET RedisForge
        OUTPUT_SCRIPT redisforge_deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${redisforge_deploy_script})

if (BUILD_TESTING)
    include(GoogleTest)

    add_executable(redisforge_tests
            tests/domain/connections/connection_profile_validator_tests.cpp
            tests/app/connections/save_connection_profile_use_case_tests.cpp
            tests/app/connections/test_and_activate_connection_use_case_tests.cpp
            tests/app/keys_explorer/scan_keys_use_case_tests.cpp
            tests/presentation/keys_explorer/keys_table_model_tests.cpp
            tests/app/value_inspector/inspect_key_use_case_tests.cpp
            tests/app/value_inspector/save_string_value_use_case_tests.cpp
            tests/app/value_inspector/save_hash_field_use_case_tests.cpp
            tests/presentation/value_inspector/value_inspection_view_data_mapper_tests.cpp
            tests/integration/hiredis_connection_gateway_integration_tests.cpp
    )
    target_include_directories(redisforge_tests PRIVATE ${REDISFORGE_SRC_ROOT})
    target_link_libraries(redisforge_tests
            PRIVATE
            redisforge_app
            redisforge_presentation
            redisforge_infrastructure
            GTest::gtest_main
    )

    gtest_discover_tests(redisforge_tests)
endif ()

set(CPACK_PACKAGE_NAME "RedisForge")
set(CPACK_PACKAGE_VENDOR "RedisForge")
set(CPACK_PACKAGE_CONTACT "support@redisforge.local")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desktop Redis browser for connections, keys, and values")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "RedisForge")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif (WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "RedisForge")
    set(CPACK_NSIS_PACKAGE_NAME "RedisForge")
endif ()

include(CPack)
