#pragma once

#include <QMainWindow>

namespace redisforge::presentation::connections
{
class ConnectionManagementViewModel;
}

namespace redisforge::presentation::keys_explorer
{
class KeysExplorerViewModel;
}

namespace redisforge::presentation::value_inspector
{
class ValueInspectorViewModel;
}

namespace redisforge::presentation
{
class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        connections::ConnectionManagementViewModel& connectionViewModel,
        value_inspector::ValueInspectorViewModel& valueInspectorViewModel,
        keys_explorer::KeysExplorerViewModel& keysExplorerViewModel,
        QWidget* parent = nullptr);
};
} // namespace redisforge::presentation
