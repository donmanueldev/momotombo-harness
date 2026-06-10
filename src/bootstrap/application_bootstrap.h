#pragma once

#include <memory>

#include "app/connections/services/active_connection_store.h"
#include "app/connections/use_cases/list_connection_profiles_use_case.h"
#include "app/connections/use_cases/save_connection_profile_use_case.h"
#include "app/connections/use_cases/test_and_activate_connection_use_case.h"
#include "app/keys_explorer/use_cases/scan_keys_use_case.h"
#include "app/value_inspector/use_cases/inspect_key_use_case.h"
#include "app/value_inspector/use_cases/save_hash_field_use_case.h"
#include "app/value_inspector/use_cases/save_string_value_use_case.h"
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
        std::shared_ptr<shared::logging::ILogger> logger;
        std::shared_ptr<app::connections::ActiveConnectionStore> activeConnectionStore;
        std::shared_ptr<domain::connections::IConnectionProfileRepository> profileRepository;
        std::shared_ptr<domain::connections::IRedisConnectionGateway> connectionGateway;
        std::shared_ptr<domain::keys_explorer::IKeyExplorerGateway> keyExplorerGateway;
        std::shared_ptr<domain::value_inspector::IValueInspectorGateway> valueInspectorGateway;
    };

    struct ApplicationServices
    {
        app::connections::ListConnectionProfilesUseCase listProfilesUseCase;
        app::connections::SaveConnectionProfileUseCase saveProfileUseCase;
        app::connections::TestAndActivateConnectionUseCase testAndActivateConnectionUseCase;
        app::keys_explorer::ScanKeysUseCase scanKeysUseCase;
        app::value_inspector::InspectKeyUseCase inspectKeyUseCase;
        app::value_inspector::SaveStringValueUseCase saveStringValueUseCase;
        app::value_inspector::SaveHashFieldUseCase saveHashFieldUseCase;
    };

    struct PresentationServices
    {
        std::unique_ptr<presentation::connections::ConnectionManagementViewModel> connectionViewModel;
        std::unique_ptr<presentation::keys_explorer::KeysExplorerViewModel> keysExplorerViewModel;
        std::unique_ptr<presentation::value_inspector::ValueInspectorViewModel> valueInspectorViewModel;
    };

    [[nodiscard]] InfrastructureServices buildInfrastructureServices();
    [[nodiscard]] ApplicationServices buildApplicationServices(const InfrastructureServices& infrastructureServices);
    [[nodiscard]] PresentationServices buildPresentationServices(
        ApplicationServices& applicationServices,
        const InfrastructureServices& infrastructureServices);
} // namespace redisforge::bootstrap
