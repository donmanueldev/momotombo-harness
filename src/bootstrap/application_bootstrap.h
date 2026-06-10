#pragma once

#include <memory>

#include "app/connections/services/active_connection_store.h"
#include "app/connections/use_cases/list_connection_profiles_use_case.h"
#include "app/connections/use_cases/save_connection_profile_use_case.h"
#include "app/connections/use_cases/test_and_activate_connection_use_case.h"
#include "app/keys_explorer/use_cases/scan_keys_use_case.h"
#include "app/value_inspector/use_cases/inspect_key_use_case.h"
#include "domain/connections/repositories/i_connection_profile_repository.h"
#include "domain/connections/services/i_redis_connection_gateway.h"
#include "domain/keys_explorer/services/i_key_explorer_gateway.h"
#include "domain/value_inspector/services/i_value_inspector_gateway.h"
#include "presentation/connections/viewmodels/connection_management_view_model.h"
#include "presentation/keys_explorer/viewmodels/keys_explorer_view_model.h"
#include "presentation/value_inspector/viewmodels/value_inspector_view_model.h"
#include "shared/logging/logger.h"

namespace redisforge::bootstrap
{
struct InfrastructureServices
{
    std::shared_ptr<redisforge::shared::logging::ILogger> logger;
    std::shared_ptr<redisforge::app::connections::ActiveConnectionStore> activeConnectionStore;
    std::shared_ptr<redisforge::domain::connections::IConnectionProfileRepository> profileRepository;
    std::shared_ptr<redisforge::domain::connections::IRedisConnectionGateway> connectionGateway;
    std::shared_ptr<redisforge::domain::keys_explorer::IKeyExplorerGateway> keyExplorerGateway;
    std::shared_ptr<redisforge::domain::value_inspector::IValueInspectorGateway> valueInspectorGateway;
};

struct ApplicationServices
{
    redisforge::app::connections::ListConnectionProfilesUseCase listProfilesUseCase;
    redisforge::app::connections::SaveConnectionProfileUseCase saveProfileUseCase;
    redisforge::app::connections::TestAndActivateConnectionUseCase testAndActivateConnectionUseCase;
    redisforge::app::keys_explorer::ScanKeysUseCase scanKeysUseCase;
    redisforge::app::value_inspector::InspectKeyUseCase inspectKeyUseCase;
};

struct PresentationServices
{
    std::unique_ptr<redisforge::presentation::connections::ConnectionManagementViewModel> connectionViewModel;
    std::unique_ptr<redisforge::presentation::keys_explorer::KeysExplorerViewModel> keysExplorerViewModel;
    std::unique_ptr<redisforge::presentation::value_inspector::ValueInspectorViewModel> valueInspectorViewModel;
};

[[nodiscard]] InfrastructureServices buildInfrastructureServices();
[[nodiscard]] ApplicationServices buildApplicationServices(const InfrastructureServices& infrastructureServices);
[[nodiscard]] PresentationServices buildPresentationServices(
    ApplicationServices& applicationServices,
    const InfrastructureServices& infrastructureServices);
} // namespace redisforge::bootstrap
