#pragma once
#include <QMainWindow>

class CameraWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    CameraWidget* m_cameraWidget = nullptr;
};
