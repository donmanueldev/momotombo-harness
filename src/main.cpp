#include <QApplication>

#include <QObject>
#include <QStyleFactory>

#include "bootstrap/application_bootstrap.h"
#include "presentation/main_window.h"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setStyle(QStyleFactory::create("Fusion"));
    application.setApplicationName("RedisForge");
    application.setOrganizationName("RedisForge");

    auto infrastructureServices = redisforge::bootstrap::buildInfrastructureServices();
    auto applicationServices = redisforge::bootstrap::buildApplicationServices(infrastructureServices);
    auto presentationServices =
        redisforge::bootstrap::buildPresentationServices(applicationServices, infrastructureServices);

    QObject::connect(
        presentationServices.connectionViewModel.get(),
        &redisforge::presentation::connections::ConnectionManagementViewModel::activeConnectionChanged,
        presentationServices.keysExplorerViewModel.get(),
        &redisforge::presentation::keys_explorer::KeysExplorerViewModel::onActiveConnectionChanged);
    QObject::connect(
        presentationServices.connectionViewModel.get(),
        &redisforge::presentation::connections::ConnectionManagementViewModel::activeConnectionChanged,
        presentationServices.valueInspectorViewModel.get(),
        &redisforge::presentation::value_inspector::ValueInspectorViewModel::onActiveConnectionChanged);

    auto mainWindow = redisforge::presentation::MainWindow(
        *presentationServices.connectionViewModel,
        *presentationServices.keysExplorerViewModel,
        *presentationServices.valueInspectorViewModel);
    mainWindow.resize(1320, 840);
    mainWindow.show();

    infrastructureServices.logger->info("redisforge.startup.completed", {{"qtVersion", QT_VERSION_STR}});

    return application.exec();
}
