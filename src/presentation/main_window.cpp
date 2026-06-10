#include "presentation/main_window.h"

#include <QAction>
#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
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
    background: #eef2f6;
}
QWidget {
    color: #1f2933;
    font-size: 12px;
}
QWidget#appShell {
    background: #eef2f6;
}
QFrame#appHeader,
QWidget#panelCard,
QFrame#infoBanner,
QWidget#toolbarCard {
    background: #fbfcfd;
    border: 1px solid #d7dee6;
    border-radius: 12px;
}
QFrame#appHeader {
    background: #fbfcfd;
    border-color: #d7dee6;
}
QLabel#heroEyebrow,
QLabel#panelEyebrow {
    color: #6b7785;
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 0.12em;
    text-transform: uppercase;
}
QLabel#heroTitle {
    color: #18212b;
    font-size: 18px;
    font-weight: 700;
}
QLabel#heroSubtitle {
    color: #607080;
    font-size: 12px;
}
QLabel#sectionTitle,
QLabel#panelTitle {
    color: #18212b;
    font-size: 15px;
    font-weight: 700;
}
QLabel#panelDescription,
QLabel#metaLabel,
QLabel#emptyStateBody,
QLabel#supportingText {
    color: #657482;
    font-size: 12px;
}
QLabel#metaLabel {
    font-size: 11px;
}
QLabel#emptyStateTitle {
    color: #2b3740;
    font-size: 18px;
    font-weight: 700;
}
QLabel#connectionBadge {
    color: #245b41;
    background: #ebf5ef;
    border: 1px solid #c9e1d1;
    border-radius: 999px;
    padding: 4px 10px;
    font-size: 11px;
    font-weight: 700;
}
QFrame#infoBanner {
    background: #f8fafc;
}
QLabel#statusCard {
    border: 0;
    padding: 0;
}
QLabel#statusCard[success="true"] {
    color: #216342;
}
QLabel#statusCard[success="false"] {
    color: #8c4330;
}
QLineEdit,
QPlainTextEdit,
QSpinBox,
QComboBox,
QListWidget,
QTableView {
    background: #ffffff;
    border: 1px solid #d4dde6;
    border-radius: 8px;
    padding: 5px 8px;
    selection-background-color: #d9ebdf;
    selection-color: #183926;
}
QLineEdit,
QSpinBox,
QComboBox {
    min-height: 18px;
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
    border: 1px solid #2d7d57;
}
QComboBox {
    padding-right: 24px;
}
QComboBox::drop-down,
QSpinBox::down-button,
QSpinBox::up-button {
    border: 0;
    width: 20px;
    background: transparent;
}
QComboBox QAbstractItemView {
    background: #ffffff;
    border: 1px solid #d4dde6;
    selection-background-color: #d9ebdf;
    padding: 4px;
}
QListWidget {
    outline: 0;
    padding: 4px;
}
QListWidget::item {
    padding: 8px 10px;
    margin-bottom: 4px;
    border-radius: 8px;
}
QListWidget::item:selected {
    background: #e8f1eb;
    color: #173826;
}
QPushButton {
    background: #f5f7fa;
    border: 1px solid #d4dde6;
    border-radius: 8px;
    padding: 7px 12px;
    font-weight: 600;
}
QPushButton:hover {
    background: #edf2f7;
}
QPushButton:disabled {
    color: #96a1ad;
    background: #f7f9fb;
    border-color: #e3e8ee;
}
QPushButton#primaryButton {
    background: #266a4a;
    color: #f8fafb;
    border-color: #266a4a;
}
QPushButton#primaryButton:hover {
    background: #1f5d41;
}
QPushButton#ghostButton {
    background: #ffffff;
}
QTabWidget::pane {
    border: 0;
    margin-top: 8px;
}
QTabBar::tab {
    background: #f4f7fa;
    border: 1px solid #d6dee7;
    border-bottom: 0;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
    padding: 8px 14px;
    margin-right: 4px;
    font-weight: 600;
    color: #576574;
}
QTabBar::tab:selected {
    background: #fbfcfd;
    color: #1b4d38;
}
QHeaderView::section {
    background: #f5f8fb;
    color: #65707d;
    border: 0;
    border-bottom: 1px solid #dbe2ea;
    padding: 6px 8px;
    font-weight: 700;
}
QTableView {
    gridline-color: transparent;
    alternate-background-color: #f8fafc;
}
QTableView::item {
    padding: 4px 6px;
}
QSplitter::handle {
    background: #d6dde5;
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
        setWindowTitle("RedisForge");
        setStyleSheet(applicationStyleSheet());
        setupMenuBar();

        auto* root = new QWidget(this);
        root->setObjectName("appShell");
        auto* rootLayout = new QVBoxLayout(root);
        rootLayout->setContentsMargins(16, 12, 16, 16);
        rootLayout->setSpacing(10);

        auto* header = new QFrame(root);
        header->setObjectName("appHeader");
        auto* headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(16, 12, 16, 12);
        headerLayout->setSpacing(12);

        auto* titleBlock = new QWidget(header);
        auto* titleLayout = new QVBoxLayout(titleBlock);
        titleLayout->setContentsMargins(0, 0, 0, 0);
        titleLayout->setSpacing(2);
        auto* titleLabel = new QLabel("RedisForge", header);
        titleLabel->setObjectName("heroTitle");
        auto* subtitleLabel = new QLabel("Local Redis browser", header);
        subtitleLabel->setObjectName("heroSubtitle");
        auto* connectionBadge = new QLabel("MVP desktop preview", header);
        connectionBadge->setObjectName("connectionBadge");

        titleLayout->addWidget(titleLabel);
        titleLayout->addWidget(subtitleLabel);
        headerLayout->addWidget(titleBlock, 1);
        headerLayout->addWidget(connectionBadge, 0, Qt::AlignVCenter);

        tabs_ = new QTabWidget(root);
        tabs_->setDocumentMode(true);
        tabs_->addTab(new connections::ConnectionManagementWidget(connectionViewModel, tabs_), "Connections");
        tabs_->addTab(
            new keys_explorer::KeysExplorerWidget(keysExplorerViewModel, valueInspectorViewModel, tabs_),
            "Data Browser");

        rootLayout->addWidget(header);
        rootLayout->addWidget(tabs_, 1);

        setCentralWidget(root);
    }

    void MainWindow::setupMenuBar()
    {
        menuBar()->setNativeMenuBar(true);

        auto* fileMenu = menuBar()->addMenu("&File");
        auto* newConnectionAction = fileMenu->addAction("New Connection");
        newConnectionAction->setShortcut(QKeySequence::New);
        connect(newConnectionAction, &QAction::triggered, this, [this]()
        {
            if (tabs_ != nullptr)
            {
                tabs_->setCurrentIndex(0);
            }
        });

        auto* openBrowserAction = fileMenu->addAction("Open Data Browser");
        openBrowserAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
        connect(openBrowserAction, &QAction::triggered, this, [this]()
        {
            if (tabs_ != nullptr)
            {
                tabs_->setCurrentIndex(1);
            }
        });
        fileMenu->addSeparator();
        auto* closeAction = fileMenu->addAction("Close Window");
        closeAction->setShortcut(QKeySequence::Close);
        connect(closeAction, &QAction::triggered, this, &QWidget::close);
        fileMenu->addSeparator();
        auto* quitAction = fileMenu->addAction("Quit RedisForge");
        quitAction->setShortcut(QKeySequence::Quit);
        connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

        auto* editMenu = menuBar()->addMenu("&Edit");
        editMenu->addAction("Cut")->setShortcut(QKeySequence::Cut);
        editMenu->addAction("Copy")->setShortcut(QKeySequence::Copy);
        editMenu->addAction("Paste")->setShortcut(QKeySequence::Paste);
        editMenu->addAction("Select All")->setShortcut(QKeySequence::SelectAll);

        auto* viewMenu = menuBar()->addMenu("&View");
        auto* showConnectionsAction = viewMenu->addAction("Connections");
        showConnectionsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
        connect(showConnectionsAction, &QAction::triggered, this, [this]()
        {
            if (tabs_ != nullptr)
            {
                tabs_->setCurrentIndex(0);
            }
        });
        auto* showBrowserAction = viewMenu->addAction("Data Browser");
        showBrowserAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
        connect(showBrowserAction, &QAction::triggered, this, [this]()
        {
            if (tabs_ != nullptr)
            {
                tabs_->setCurrentIndex(1);
            }
        });
        viewMenu->addSeparator();
        auto* fullScreenAction = viewMenu->addAction("Toggle Full Screen");
        fullScreenAction->setShortcut(QKeySequence::FullScreen);
        connect(fullScreenAction, &QAction::triggered, this, [this]()
        {
            isFullScreen() ? showNormal() : showFullScreen();
        });

        auto* windowMenu = menuBar()->addMenu("&Window");
        auto* minimizeAction = windowMenu->addAction("Minimize");
        minimizeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
        connect(minimizeAction, &QAction::triggered, this, &QWidget::showMinimized);
        auto* zoomAction = windowMenu->addAction("Zoom");
        connect(zoomAction, &QAction::triggered, this, [this]()
        {
            isMaximized() ? showNormal() : showMaximized();
        });
        windowMenu->addSeparator();
        auto* bringAllToFrontAction = windowMenu->addAction("Bring All to Front");
        connect(bringAllToFrontAction, &QAction::triggered, this, [this]()
        {
            raise();
            activateWindow();
        });

        auto* helpMenu = menuBar()->addMenu("&Help");
        auto* shortcutsAction = helpMenu->addAction("Keyboard Shortcuts");
        connect(shortcutsAction, &QAction::triggered, this, [this]()
        {
            QMessageBox::information(
                this,
                "Keyboard Shortcuts",
                "Cmd/Ctrl+N opens Connections.\nCmd/Ctrl+1 shows Connections.\nCmd/Ctrl+2 shows Data Browser.");
        });
        auto* aboutAction = helpMenu->addAction("About RedisForge");
        connect(aboutAction, &QAction::triggered, this, [this]()
        {
            QMessageBox::about(this, "About RedisForge", "RedisForge\nDesktop Redis browser for connections, keys, and values.");
        });
    }
} // namespace redisforge::presentation
