#include "presentation/main_window.h"

#include <QLabel>
#include <QTabWidget>

#include "presentation/connections/viewmodels/connection_management_view_model.h"
#include "presentation/connections/widgets/connection_management_widget.h"
#include "presentation/keys_explorer/widgets/keys_explorer_widget.h"
#include "presentation/value_inspector/widgets/value_inspector_widget.h"

namespace
{
    QWidget* makePlaceholderTab(const QString& title)
    {
        auto* label = new QLabel(title);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        return label;
    }
} // namespace

namespace redisforge::presentation
{
    MainWindow::MainWindow(
        connections::ConnectionManagementViewModel& connectionViewModel,
        value_inspector::ValueInspectorViewModel& valueInspectorViewModel,
        keys_explorer::KeysExplorerViewModel& keysExplorerViewModel,
        QWidget* parent)
        : QMainWindow(parent)
    {
        setWindowTitle("RedisForge");

        auto* tabs = new QTabWidget(this);
        tabs->addTab(new connections::ConnectionManagementWidget(connectionViewModel, tabs), "Connections");
        tabs->addTab(
            new keys_explorer::KeysExplorerWidget(keysExplorerViewModel, valueInspectorViewModel, tabs),
            "Keys Explorer");
        tabs->addTab(new value_inspector::ValueInspectorWidget(valueInspectorViewModel, tabs), "Value Inspector");
        tabs->addTab(makePlaceholderTab("TTL Manager scaffold ready."), "TTL Manager");
        tabs->addTab(makePlaceholderTab("Pub/Sub scaffold ready."), "Pub/Sub");
        tabs->addTab(makePlaceholderTab("Streams scaffold ready."), "Streams");
        tabs->addTab(makePlaceholderTab("Monitoring scaffold ready."), "Monitoring");
        tabs->addTab(makePlaceholderTab("Command Console scaffold ready."), "Command Console");

        setCentralWidget(tabs);
    }
} // namespace redisforge::presentation
