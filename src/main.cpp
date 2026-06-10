#include <QApplication>

#include <memory>

#include <QObject>

#include "app/connections/services/active_connection_store.h"
#include "app/connections/use_cases/list_connection_profiles_use_case.h"
#include "app/connections/use_cases/save_connection_profile_use_case.h"
#include "app/connections/use_cases/test_connection_use_case.h"
#include "app/keys_explorer/use_cases/scan_keys_use_case.h"
#include "app/value_inspector/use_cases/inspect_key_use_case.h"
#include "domain/connections/repositories/i_connection_profile_repository.h"
#include "domain/connections/services/i_redis_connection_gateway.h"
#include "domain/keys_explorer/services/i_key_explorer_gateway.h"
#include "domain/value_inspector/services/i_value_inspector_gateway.h"
#include "infrastructure/connections/hiredis_connection_gateway.h"
#include "infrastructure/connections/in_memory_connection_profile_repository.h"
#include "infrastructure/keys_explorer/hiredis_key_explorer_gateway.h"
#include "infrastructure/logging/qt_structured_logger.h"
#include "infrastructure/value_inspector/hiredis_value_inspector_gateway.h"
#include "presentation/connections/viewmodels/connection_management_view_model.h"
#include "presentation/keys_explorer/viewmodels/keys_explorer_view_model.h"
#include "presentation/main_window.h"
#include "shared/di/service_container.h"
#include "shared/logging/logger.h"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("RedisForge");
    application.setOrganizationName("RedisForge");

    auto serviceContainer = redisforge::shared::di::ServiceContainer{};

    serviceContainer.registerInstance<redisforge::app::connections::ActiveConnectionStore>(
        std::make_shared<redisforge::app::connections::ActiveConnectionStore>());
    serviceContainer.registerInstance<redisforge::shared::logging::ILogger>(
        std::make_shared<redisforge::infrastructure::logging::QtStructuredLogger>());
    serviceContainer.registerInstance<redisforge::domain::connections::IConnectionProfileRepository>(
        std::make_shared<redisforge::infrastructure::connections::InMemoryConnectionProfileRepository>());
    serviceContainer.registerInstance<redisforge::domain::connections::IRedisConnectionGateway>(
        std::make_shared<redisforge::infrastructure::connections::HiredisConnectionGateway>());
    serviceContainer.registerInstance<redisforge::domain::keys_explorer::IKeyExplorerGateway>(
        std::make_shared<redisforge::infrastructure::keys_explorer::HiredisKeyExplorerGateway>());
    serviceContainer.registerInstance<redisforge::domain::value_inspector::IValueInspectorGateway>(
        std::make_shared<redisforge::infrastructure::value_inspector::HiredisValueInspectorGateway>());

    auto activeConnectionStore = serviceContainer.resolve<redisforge::app::connections::ActiveConnectionStore>();
    auto logger = serviceContainer.resolve<redisforge::shared::logging::ILogger>();
    auto profileRepository = serviceContainer.resolve<redisforge::domain::connections::IConnectionProfileRepository>();
    auto connectionGateway = serviceContainer.resolve<redisforge::domain::connections::IRedisConnectionGateway>();
    auto keyExplorerGateway = serviceContainer.resolve<redisforge::domain::keys_explorer::IKeyExplorerGateway>();
    auto valueInspectorGateway = serviceContainer.resolve<redisforge::domain::value_inspector::IValueInspectorGateway>();

    auto listProfilesUseCase =
        redisforge::app::connections::ListConnectionProfilesUseCase{*profileRepository, *logger};
    auto saveProfileUseCase =
        redisforge::app::connections::SaveConnectionProfileUseCase{*profileRepository, *logger};
    auto testConnectionUseCase =
        redisforge::app::connections::TestConnectionUseCase{*connectionGateway, *logger};
    auto scanKeysUseCase =
        redisforge::app::keys_explorer::ScanKeysUseCase{*activeConnectionStore, *keyExplorerGateway, *logger};
    auto inspectKeyUseCase =
        redisforge::app::value_inspector::InspectKeyUseCase{*activeConnectionStore, *valueInspectorGateway, *logger};

    redisforge::presentation::connections::ConnectionManagementViewModel connectionViewModel(
        listProfilesUseCase,
        saveProfileUseCase,
        testConnectionUseCase,
        *activeConnectionStore,
        *logger
    );
    redisforge::presentation::keys_explorer::KeysExplorerViewModel keysExplorerViewModel(
        scanKeysUseCase,
        inspectKeyUseCase);

    QObject::connect(
        &connectionViewModel,
        &redisforge::presentation::connections::ConnectionManagementViewModel::activeConnectionChanged,
        &keysExplorerViewModel,
        &redisforge::presentation::keys_explorer::KeysExplorerViewModel::onActiveConnectionChanged);

    redisforge::presentation::MainWindow mainWindow(connectionViewModel, keysExplorerViewModel);
    mainWindow.resize(1320, 840);
    mainWindow.show();

    logger->info("redisforge.startup.completed", {{"qtVersion", QT_VERSION_STR}});

    return application.exec();
}
