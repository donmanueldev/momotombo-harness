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
            std::make_shared<infrastructure::logging::QtStructuredLogger>(),
            std::make_shared<app::connections::ActiveConnectionStore>(),
            std::make_shared<infrastructure::connections::InMemoryConnectionProfileRepository>(),
            std::make_shared<infrastructure::connections::HiredisConnectionGateway>(),
            std::make_shared<infrastructure::keys_explorer::HiredisKeyExplorerGateway>(),
            std::make_shared<infrastructure::value_inspector::HiredisValueInspectorGateway>(),
        };
    }

    ApplicationServices buildApplicationServices(const InfrastructureServices& infrastructureServices)
    {
        return ApplicationServices{
            app::connections::ListConnectionProfilesUseCase{
                *infrastructureServices.profileRepository,
                *infrastructureServices.logger,
            },
            app::connections::SaveConnectionProfileUseCase{
                *infrastructureServices.profileRepository,
                *infrastructureServices.logger,
            },
            app::connections::TestAndActivateConnectionUseCase{
                *infrastructureServices.connectionGateway,
                *infrastructureServices.activeConnectionStore,
                *infrastructureServices.logger,
            },
            app::keys_explorer::ScanKeysUseCase{
                *infrastructureServices.activeConnectionStore,
                *infrastructureServices.keyExplorerGateway,
                *infrastructureServices.logger,
            },
            app::value_inspector::InspectKeyUseCase{
                *infrastructureServices.activeConnectionStore,
                *infrastructureServices.valueInspectorGateway,
                *infrastructureServices.logger,
            },
            app::value_inspector::SaveStringValueUseCase{
                *infrastructureServices.activeConnectionStore,
                *infrastructureServices.valueInspectorGateway,
                *infrastructureServices.logger,
            },
            app::value_inspector::SaveHashFieldUseCase{
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
            std::make_unique<presentation::connections::ConnectionManagementViewModel>(
                applicationServices.listProfilesUseCase,
                applicationServices.saveProfileUseCase,
                applicationServices.testAndActivateConnectionUseCase,
                *infrastructureServices.logger),
            std::make_unique<presentation::keys_explorer::KeysExplorerViewModel>(
                applicationServices.scanKeysUseCase),
            std::make_unique<presentation::value_inspector::ValueInspectorViewModel>(
                applicationServices.inspectKeyUseCase,
                applicationServices.saveStringValueUseCase,
                applicationServices.saveHashFieldUseCase),
        };
    }
} // namespace redisforge::bootstrap
