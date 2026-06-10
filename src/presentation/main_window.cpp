#include "presentation/main_window.h"

#include <QFrame>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "presentation/connections/viewmodels/connection_management_view_model.h"
#include "presentation/connections/widgets/connection_management_widget.h"
#include "presentation/keys_explorer/widgets/keys_explorer_widget.h"

namespace
{
QString applicationStyleSheet()
{
    return QString::fromUtf8(R"(
QMainWindow {
    background: #f3f0ea;
}
QWidget {
    color: #1d2730;
    font-size: 13px;
}
QFrame#appHeader,
QGroupBox,
QWidget#panelCard {
    background: #fcfbf8;
    border: 1px solid #ddd5c7;
    border-radius: 14px;
}
QFrame#appHeader {
    background: #183d2f;
    border: 0;
}
QLabel#appTitle {
    color: #f8f5ee;
    font-size: 24px;
    font-weight: 700;
}
QLabel#appSubtitle {
    color: #d7e3db;
    font-size: 12px;
}
QLabel#sectionTitle {
    color: #18212b;
    font-size: 18px;
    font-weight: 700;
}
QLabel#statusCard {
    background: #fffdf8;
    border: 1px solid #ddd6c8;
    border-radius: 12px;
    padding: 10px 12px;
}
QLabel#statusCard[success="true"] {
    background: #eef8f1;
    border-color: #8ac49a;
    color: #1c5e35;
}
QLabel#statusCard[success="false"] {
    background: #fff4ef;
    border-color: #e8a08d;
    color: #8a2f1d;
}
QLineEdit,
QPlainTextEdit,
QSpinBox,
QComboBox,
QListWidget,
QTableView {
    background: #fffdf8;
    border: 1px solid #d3ccbf;
    border-radius: 9px;
    padding: 6px 9px;
    selection-background-color: #d7eadf;
    selection-color: #173325;
}
QLineEdit,
QSpinBox,
QComboBox {
    min-height: 20px;
}
QLineEdit[echoMode="2"] {
    letter-spacing: 0.15em;
}
QLineEdit:focus,
QPlainTextEdit:focus,
QSpinBox:focus,
QComboBox:focus,
QListWidget:focus,
QTableView:focus {
    border: 1px solid #2f7d57;
}
QComboBox {
    padding-right: 28px;
}
QComboBox::drop-down,
QSpinBox::down-button,
QSpinBox::up-button {
    border: 0;
    width: 22px;
    background: transparent;
}
QComboBox QAbstractItemView {
    background: #fffdf8;
    border: 1px solid #d3ccbf;
    selection-background-color: #d7eadf;
    padding: 4px;
}
QPushButton {
    background: #ebe4d8;
    border: 1px solid #d2c8b7;
    border-radius: 9px;
    padding: 8px 12px;
    font-weight: 600;
}
QPushButton:hover {
    background: #e3dbc9;
}
QPushButton:disabled {
    color: #9aa5af;
    background: #f1ede3;
    border-color: #e6dfd0;
}
QPushButton#primaryButton {
    background: #1f6a49;
    color: #f7f4ed;
    border-color: #1f6a49;
}
QPushButton#primaryButton:hover {
    background: #18583c;
}
QPushButton#ghostButton {
    background: transparent;
}
QTabWidget::pane {
    border: 0;
    margin-top: 12px;
}
QTabBar::tab {
    background: #e8e0d2;
    border: 1px solid #d4cab8;
    border-bottom: 0;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
    padding: 8px 16px;
    margin-right: 6px;
    font-weight: 600;
}
QTabBar::tab:selected {
    background: #fbfaf7;
    color: #173325;
}
QHeaderView::section {
    background: #efe8dc;
    color: #5e5140;
    border: 0;
    border-bottom: 1px solid #d7cfbf;
    padding: 8px 10px;
    font-weight: 700;
}
QGroupBox {
    margin-top: 14px;
    padding: 16px 14px 14px 14px;
    font-weight: 700;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    top: -10px;
    padding: 2px 8px;
    background: #fcfbf8;
    color: #42505c;
}
QSplitter::handle {
    background: #ddd6c8;
    width: 2px;
}
)");
}
}

namespace redisforge::presentation
{
    MainWindow::MainWindow(
        connections::ConnectionManagementViewModel& connectionViewModel,
        keys_explorer::KeysExplorerViewModel& keysExplorerViewModel,
        value_inspector::ValueInspectorViewModel& valueInspectorViewModel,
        QWidget* parent)
        : QMainWindow(parent)
    {
        setWindowTitle("RedisForge Data Browser");
        setStyleSheet(applicationStyleSheet());

        auto* root = new QWidget(this);
        auto* rootLayout = new QVBoxLayout(root);
        rootLayout->setContentsMargins(24, 20, 24, 24);
        rootLayout->setSpacing(18);

        auto* header = new QFrame(root);
        header->setObjectName("appHeader");
        auto* headerLayout = new QVBoxLayout(header);
        headerLayout->setContentsMargins(22, 18, 22, 18);
        headerLayout->setSpacing(4);

        auto* titleLabel = new QLabel("RedisForge", header);
        titleLabel->setObjectName("appTitle");
        auto* subtitleLabel = new QLabel("Redis browser", header);
        subtitleLabel->setObjectName("appSubtitle");

        headerLayout->addWidget(titleLabel);
        headerLayout->addWidget(subtitleLabel);

        auto* tabs = new QTabWidget(root);
        tabs->setDocumentMode(true);
        tabs->addTab(new connections::ConnectionManagementWidget(connectionViewModel, tabs), "Connections");
        tabs->addTab(
            new keys_explorer::KeysExplorerWidget(keysExplorerViewModel, valueInspectorViewModel, tabs),
            "Data Browser");

        rootLayout->addWidget(header);
        rootLayout->addWidget(tabs, 1);

        setCentralWidget(root);
    }
} // namespace redisforge::presentation
