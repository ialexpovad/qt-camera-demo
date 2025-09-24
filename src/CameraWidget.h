#pragma once

#include <QWidget>
#include <QMediaCaptureSession>
#include <QCamera>
#include <QImageCapture>
#include <QVideoWidget>
#include <QMediaDevices>
#include <QCameraDevice>

class QPushButton;
class QComboBox;
class QLabel;

class CameraWidget : public QWidget {
    Q_OBJECT
public:
    explicit CameraWidget(QWidget* parent = nullptr);
    ~CameraWidget() override;

private slots:
    void startCamera();
    void stopCamera();
    void captureImage();
    void onDeviceSelectionChanged(int index);
    void onImageSaved(int id, const QString& fileName);
    void onImageError(int id, QImageCapture::Error error, const QString& errorString);
    void handleCameraError(QCamera::Error error, const QString& errorString);

private:
    void refreshDevices();
    void setCameraDevice(const QCameraDevice& dev);
    QString nextDefaultPath(const QString& extension) const;
    void updateUiState();

private:
    // Media pipeline
    QMediaCaptureSession m_session;
    std::unique_ptr<QCamera> m_camera;
    QImageCapture* m_imageCapture = nullptr;

    // UI
    QVideoWidget* m_videoWidget = nullptr;
    QPushButton* m_startBtn = nullptr;
    QPushButton* m_stopBtn = nullptr;
    QPushButton* m_captureBtn = nullptr;
    QComboBox* m_deviceCombo = nullptr;
    QLabel* m_statusLabel = nullptr;

    // Device listing
    QMediaDevices m_devices;                // emits videoInputsChanged()
    QList<QCameraDevice> m_videoInputs;
};
