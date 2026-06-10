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
    background: #f3f1eb;
}
QWidget {
    color: #24313a;
    font-size: 13px;
}
QWidget#appShell {
    background: #f3f1eb;
}
QFrame#appHeader,
QWidget#panelCard,
QFrame#infoBanner {
    background: #fbf8f2;
    border: 1px solid #d8d0c2;
    border-radius: 16px;
}
QFrame#appHeader {
    background: #1f4d3a;
    border-color: #1f4d3a;
}
QLabel#heroEyebrow,
QLabel#panelEyebrow {
    color: #6c776f;
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 0.12em;
    text-transform: uppercase;
}
QLabel#heroEyebrow {
    color: #cfded5;
}
QLabel#heroTitle {
    color: #f7f3ec;
    font-size: 30px;
    font-weight: 700;
}
QLabel#heroSubtitle {
    color: #dce8e1;
    font-size: 15px;
    line-height: 1.35;
}
QLabel#sectionTitle,
QLabel#panelTitle {
    color: #1c2a33;
    font-size: 18px;
    font-weight: 700;
}
QLabel#panelDescription,
QLabel#metaLabel,
QLabel#emptyStateBody,
QLabel#supportingText {
    color: #61707b;
    font-size: 13px;
}
QLabel#metaLabel {
    font-size: 12px;
}
QLabel#emptyStateTitle {
    color: #2b3740;
    font-size: 20px;
    font-weight: 700;
}
QLabel#emptyStateBody {
    line-height: 1.35;
}
QFrame#infoBanner {
    background: #fcfaf5;
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
    background: #fffdf9;
    border: 1px solid #d4cbbb;
    border-radius: 10px;
    padding: 7px 10px;
    selection-background-color: #dceadf;
    selection-color: #1d4330;
}
QLineEdit,
QSpinBox,
QComboBox {
    min-height: 22px;
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
    border: 1px solid #2f7a59;
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
    background: #fffdf9;
    border: 1px solid #d4cbbb;
    selection-background-color: #dceadf;
    padding: 4px;
}
QListWidget {
    outline: 0;
    padding: 8px;
}
QListWidget::item {
    padding: 10px 12px;
    margin-bottom: 6px;
    border-radius: 10px;
}
QListWidget::item:selected {
    background: #edf3ee;
    color: #1d4330;
}
QPushButton {
    background: #ece5d8;
    border: 1px solid #d5cab7;
    border-radius: 10px;
    padding: 9px 14px;
    font-weight: 600;
}
QPushButton:hover {
    background: #e3dac8;
}
QPushButton:disabled {
    color: #9aa5af;
    background: #f2ede4;
    border-color: #e4dccf;
}
QPushButton#primaryButton {
    background: #266a4a;
    color: #f8f5ee;
    border-color: #266a4a;
}
QPushButton#primaryButton:hover {
    background: #1f5a3f;
}
QPushButton#ghostButton {
    background: #f7f3ec;
}
QTabWidget::pane {
    border: 0;
    margin-top: 10px;
}
QTabBar::tab {
    background: #eee6d9;
    border: 1px solid #d5cbb9;
    border-bottom: 0;
    border-top-left-radius: 12px;
    border-top-right-radius: 12px;
    padding: 10px 18px;
    margin-right: 6px;
    font-weight: 600;
    color: #4d5d67;
}
QTabBar::tab:selected {
    background: #fbf8f2;
    color: #1e4d39;
}
QHeaderView::section {
    background: #f1ebdf;
    color: #5d645f;
    border: 0;
    border-bottom: 1px solid #d8cfbf;
    padding: 9px 10px;
    font-weight: 700;
}
QTableView {
    gridline-color: transparent;
    alternate-background-color: #f7f3eb;
}
QTableView::item {
    padding: 6px 8px;
}
QSplitter::handle {
    background: #ddd4c4;
    width: 3px;
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
        rootLayout->setContentsMargins(24, 20, 24, 24);
        rootLayout->setSpacing(18);

        auto* header = new QFrame(root);
        header->setObjectName("appHeader");
        auto* headerLayout = new QVBoxLayout(header);
        headerLayout->setContentsMargins(32, 26, 32, 28);
        headerLayout->setSpacing(8);

        auto* eyebrowLabel = new QLabel("Desktop Redis client", header);
        eyebrowLabel->setObjectName("heroEyebrow");
        auto* titleLabel = new QLabel("RedisForge", header);
        titleLabel->setObjectName("heroTitle");
        auto* subtitleLabel = new QLabel("Connections, keys, and values.", header);
        subtitleLabel->setObjectName("heroSubtitle");
        subtitleLabel->setWordWrap(true);

        headerLayout->addWidget(eyebrowLabel);
        headerLayout->addWidget(titleLabel);
        headerLayout->addWidget(subtitleLabel);

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
