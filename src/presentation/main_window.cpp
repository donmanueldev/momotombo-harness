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
    background: #f4f1ea;
}
QWidget {
    color: #1f2933;
    font-size: 13px;
}
QFrame#appHeader,
QGroupBox,
QWidget#panelCard {
    background: #fbfaf7;
    border: 1px solid #ddd6c8;
    border-radius: 16px;
}
QFrame#appHeader {
    background: #1f4d3a;
    border: 0;
}
QLabel#appTitle {
    color: #f7f4ed;
    font-size: 26px;
    font-weight: 700;
}
QLabel#appSubtitle {
    color: #d8e7de;
    font-size: 13px;
}
QLabel#sectionEyebrow {
    color: #7a6f5f;
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 0.08em;
    text-transform: uppercase;
}
QLabel#sectionTitle {
    color: #18212b;
    font-size: 20px;
    font-weight: 700;
}
QLabel#sectionBody {
    color: #5f6b76;
    font-size: 13px;
}
QLabel#statusCard {
    background: #fffdf8;
    border: 1px solid #ddd6c8;
    border-radius: 12px;
    padding: 12px 14px;
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
    border: 1px solid #d7cfbf;
    border-radius: 10px;
    padding: 8px 10px;
    selection-background-color: #d7eadf;
    selection-color: #173325;
}
QLineEdit:focus,
QPlainTextEdit:focus,
QSpinBox:focus,
QComboBox:focus,
QListWidget:focus,
QTableView:focus {
    border: 1px solid #2f7d57;
}
QPushButton {
    background: #ebe4d8;
    border: 1px solid #d2c8b7;
    border-radius: 10px;
    padding: 9px 14px;
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
    border-top-left-radius: 12px;
    border-top-right-radius: 12px;
    padding: 10px 18px;
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
    padding: 9px 10px;
    font-weight: 700;
}
QGroupBox {
    margin-top: 16px;
    padding: 18px 16px 16px 16px;
    font-weight: 700;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 14px;
    top: -10px;
    padding: 2px 8px;
    background: #fbfaf7;
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
        headerLayout->setContentsMargins(24, 22, 24, 22);
        headerLayout->setSpacing(6);

        auto* titleLabel = new QLabel("RedisForge", header);
        titleLabel->setObjectName("appTitle");
        auto* subtitleLabel = new QLabel(
            "Connect to local or hosted Redis instances, browse keys, inspect values, and edit supported records.",
            header);
        subtitleLabel->setObjectName("appSubtitle");
        subtitleLabel->setWordWrap(true);

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
