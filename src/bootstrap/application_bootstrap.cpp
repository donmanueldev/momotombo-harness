#include "bootstrap/application_bootstrap.h"

#include "infrastructure/connections/hiredis_connection_gateway.h"
#include "infrastructure/connections/in_memory_connection_profile_repository.h"
#include "infrastructure/keys_explorer/hiredis_key_explorer_gateway.h"
#include "infrastructure/logging/qt_structured_logger.h"
#include "infrastructure/value_inspector/hiredis_value_inspector_gateway.h"

namespace redisforge::bootstrap
{
InfrastructureServices buildInfrastructureServices()
{
    return InfrastructureServices{
        std::make_shared<redisforge::infrastructure::logging::QtStructuredLogger>(),
        std::make_shared<redisforge::app::connections::ActiveConnectionStore>(),
        std::make_shared<redisforge::infrastructure::connections::InMemoryConnectionProfileRepository>(),
        std::make_shared<redisforge::infrastructure::connections::HiredisConnectionGateway>(),
        std::make_shared<redisforge::infrastructure::keys_explorer::HiredisKeyExplorerGateway>(),
        std::make_shared<redisforge::infrastructure::value_inspector::HiredisValueInspectorGateway>(),
    };
}

ApplicationServices buildApplicationServices(const InfrastructureServices& infrastructureServices)
{
    return ApplicationServices{
        redisforge::app::connections::ListConnectionProfilesUseCase{
            *infrastructureServices.profileRepository,
            *infrastructureServices.logger,
        },
        redisforge::app::connections::SaveConnectionProfileUseCase{
            *infrastructureServices.profileRepository,
            *infrastructureServices.logger,
        },
        redisforge::app::connections::TestAndActivateConnectionUseCase{
            *infrastructureServices.connectionGateway,
            *infrastructureServices.activeConnectionStore,
            *infrastructureServices.logger,
        },
        redisforge::app::keys_explorer::ScanKeysUseCase{
            *infrastructureServices.activeConnectionStore,
            *infrastructureServices.keyExplorerGateway,
            *infrastructureServices.logger,
        },
        redisforge::app::value_inspector::InspectKeyUseCase{
            *infrastructureServices.activeConnectionStore,
            *infrastructureServices.valueInspectorGateway,
            *infrastructureServices.logger,
        },
    };
}

PresentationServices buildPresentationServices(
    ApplicationServices& applicationServices,
    const InfrastructureServices& infrastructureServices)
{
    return PresentationServices{
        std::make_unique<redisforge::presentation::connections::ConnectionManagementViewModel>(
            applicationServices.listProfilesUseCase,
            applicationServices.saveProfileUseCase,
            applicationServices.testAndActivateConnectionUseCase,
            *infrastructureServices.logger),
        std::make_unique<redisforge::presentation::keys_explorer::KeysExplorerViewModel>(
            applicationServices.scanKeysUseCase),
        std::make_unique<redisforge::presentation::value_inspector::ValueInspectorViewModel>(
            applicationServices.inspectKeyUseCase),
    };
}
} // namespace redisforge::bootstrap
