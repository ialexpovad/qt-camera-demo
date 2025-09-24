#include "MainWindow.h"
#include "CameraWidget.h"
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_cameraWidget = new CameraWidget(this);
    setCentralWidget(m_cameraWidget);
    setWindowTitle(QStringLiteral("Qt Camera Demo (Qt 6, Widgets)"));

    // Reasonable starting size
    resize(960, 640);

    // Center on primary screen
    const auto g = screen()->geometry();
    move(g.center() - rect().center());
}

MainWindow::~MainWindow() = default;
