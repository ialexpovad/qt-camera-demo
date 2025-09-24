#include "CameraWidget.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>

#include "PlatformPermissions.h"

CameraWidget::CameraWidget(QWidget* parent)
    : QWidget(parent)
{
    // --- UI ---
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setMinimumSize(640, 360);

    m_deviceCombo  = new QComboBox(this);
    m_startBtn     = new QPushButton(QStringLiteral("Start"), this);
    m_stopBtn      = new QPushButton(QStringLiteral("Stop"), this);
    m_captureBtn   = new QPushButton(QStringLiteral("Capture"), this);
    m_statusLabel  = new QLabel(QStringLiteral("Idle"), this);
    m_statusLabel->setObjectName("statusLabel");

    auto* topBar = new QHBoxLayout;
    topBar->addWidget(new QLabel(QStringLiteral("Camera:"), this));
    topBar->addWidget(m_deviceCombo, /*stretch*/1);
    topBar->addSpacing(12);
    topBar->addWidget(m_startBtn);
    topBar->addWidget(m_stopBtn);
    topBar->addWidget(m_captureBtn);

    auto* root = new QVBoxLayout(this);
    root->addLayout(topBar);
    root->addWidget(m_videoWidget, /*stretch*/1);
    root->addWidget(m_statusLabel);

    setLayout(root);

    // --- Media wiring ---
    m_imageCapture = new QImageCapture(this);
    m_session.setVideoOutput(m_videoWidget);
    m_session.setImageCapture(m_imageCapture);

    // Signals
    connect(m_startBtn,   &QPushButton::clicked, this, &CameraWidget::startCamera);
    connect(m_stopBtn,    &QPushButton::clicked, this, &CameraWidget::stopCamera);
    connect(m_captureBtn, &QPushButton::clicked, this, &CameraWidget::captureImage);
    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraWidget::onDeviceSelectionChanged);

    connect(m_imageCapture, &QImageCapture::imageSaved,
            this, &CameraWidget::onImageSaved);
    connect(m_imageCapture, &QImageCapture::errorOccurred,
            this, &CameraWidget::onImageError);

    // Refresh device list now and when devices change
    connect(&m_devices, &QMediaDevices::videoInputsChanged,
            this, &CameraWidget::refreshDevices);

    refreshDevices();
    updateUiState();

}

CameraWidget::~CameraWidget() {
    // Detach in safe order before destruction
    m_session.setImageCapture(nullptr);
    m_session.setVideoOutput(nullptr);
    m_session.setCamera(nullptr);
    if (m_camera) m_camera->stop();
    m_camera.reset();
}


void CameraWidget::refreshDevices() {
    m_videoInputs = m_devices.videoInputs();
    m_deviceCombo->clear();

    for (const auto& dev : m_videoInputs) {
        m_deviceCombo->addItem(dev.description());
    }

    if (m_videoInputs.isEmpty()) {
        m_statusLabel->setText(QStringLiteral("No cameras detected"));
        updateUiState();
        return;
    }

    // Select the first available device by default
    if (m_deviceCombo->currentIndex() < 0 && !m_videoInputs.isEmpty())
        m_deviceCombo->setCurrentIndex(0);

    // Prepare camera for the selected device (but don't auto-start)
    setCameraDevice(m_videoInputs[m_deviceCombo->currentIndex()]);
    updateUiState();
}

void CameraWidget::setCameraDevice(const QCameraDevice& dev) {
    // Stop & replace camera if switching
    if (m_camera) {
        m_camera->stop();
        m_session.setCamera(nullptr);
        m_camera.reset();
    }

    m_camera = std::make_unique<QCamera>(dev);

    connect(m_camera.get(), &QCamera::errorOccurred,
            this, &CameraWidget::handleCameraError);

    m_session.setCamera(m_camera.get());
    m_statusLabel->setText(QStringLiteral("Selected: %1").arg(dev.description()));
}

void CameraWidget::startCamera() {
#if defined(Q_OS_MACOS)
    if (!ensureCameraPermission(this)) return;
#endif
    if (!m_camera) {
        if (m_videoInputs.isEmpty()) {
            QMessageBox::warning(this, "Camera", "No camera devices available.");
            return;
        }
        setCameraDevice(m_videoInputs[m_deviceCombo->currentIndex()]);
    }
    m_camera->start();
    m_statusLabel->setText("Preview running…");
    updateUiState();
}

void CameraWidget::stopCamera() {
    if (m_camera) {
        m_camera->stop();
    }
    m_statusLabel->setText(QStringLiteral("Stopped"));
    updateUiState();
}

void CameraWidget::captureImage() {
    if (!m_camera || m_camera->isActive() == false) {
        QMessageBox::information(this, QStringLiteral("Capture"),
                                 QStringLiteral("Camera is not running."));
        return;
    }

    // Choose save path; default to Pictures/QtCameraDemo with timestamp
    QString suggested = nextDefaultPath("jpg");
    QString filter = QStringLiteral("JPEG (*.jpg *.jpeg);;PNG (*.png)");
    QString file = QFileDialog::getSaveFileName(this, QStringLiteral("Save snapshot"),
                                                suggested, filter);
    if (file.isEmpty())
        return;

    // Hint format from extension
    if (file.endsWith(".png", Qt::CaseInsensitive))
        m_imageCapture->setFileFormat(QImageCapture::FileFormat::PNG);
    else
        m_imageCapture->setFileFormat(QImageCapture::FileFormat::JPEG);

    #if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        m_imageCapture->setQuality(QImageCapture::VeryHighQuality);
    #else
        m_imageCapture->setQuality(QImageCapture::HighQuality);
    #endif

    const int id = m_imageCapture->captureToFile(file);
    if (id == -1) {
        QMessageBox::warning(this, QStringLiteral("Capture"),
                             QStringLiteral("Failed to initiate capture."));
    } else {
        m_statusLabel->setText(QStringLiteral("Capturing…"));
    }
}

void CameraWidget::onDeviceSelectionChanged(int index) {
    if (index < 0 || index >= m_videoInputs.size())
        return;
    setCameraDevice(m_videoInputs[index]);
    // keep running state; if camera was active, restart for new device
    if (m_camera && m_camera->isActive())
        m_camera->start();
    updateUiState();
}

void CameraWidget::onImageSaved(int /*id*/, const QString& fileName) {
    m_statusLabel->setText(QStringLiteral("Saved: %1").arg(fileName));
}

void CameraWidget::onImageError(int /*id*/, QImageCapture::Error error, const QString& errorString) {
    Q_UNUSED(error);
    QMessageBox::warning(this, QStringLiteral("Capture error"), errorString);
    m_statusLabel->setText(QStringLiteral("Error: %1").arg(errorString));
}

void CameraWidget::handleCameraError(QCamera::Error error, const QString& errorString) {
    if (error == QCamera::NoError)
        return;
    QMessageBox::warning(this, QStringLiteral("Camera error"), errorString);
    m_statusLabel->setText(QStringLiteral("Camera error: %1").arg(errorString));
}

QString CameraWidget::nextDefaultPath(const QString& extension) const {
    const QString pictures = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QDir dir(pictures.isEmpty() ? QDir::homePath() : pictures);
    if (!dir.exists("QtCameraDemo"))
        dir.mkpath("QtCameraDemo");
    dir.cd("QtCameraDemo");

    const QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    return dir.filePath(QStringLiteral("shot_%1.%2").arg(ts, extension));
}

void CameraWidget::updateUiState() {
    const bool hasCam = (m_camera != nullptr);
    const bool running = hasCam && m_camera->isActive();

    m_startBtn->setEnabled(hasCam && !running);
    m_stopBtn->setEnabled(hasCam && running);
    m_captureBtn->setEnabled(hasCam && running);
    m_deviceCombo->setEnabled(!running);
}
