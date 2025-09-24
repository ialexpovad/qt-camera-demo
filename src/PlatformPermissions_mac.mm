#include "PlatformPermissions.h"
#ifdef Q_OS_MACOS
#import <AVFoundation/AVFoundation.h>
#include <QMessageBox>

bool ensureCameraPermission(QWidget* parent) {
    AVAuthorizationStatus st = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    if (st == AVAuthorizationStatusAuthorized) return true;
    if (st == AVAuthorizationStatusDenied || st == AVAuthorizationStatusRestricted) {
        QMessageBox::warning(parent, "Camera",
            "Camera access is denied.\nEnable it in System Settings → Privacy & Security → Camera for QtCameraDemo.");
        return false;
    }
    __block BOOL granted = NO;
    dispatch_semaphore_t sem = dispatch_semaphore_create(0);
    [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL g){
        granted = g; dispatch_semaphore_signal(sem);
    }];
    dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
    if (!granted) {
        QMessageBox::warning(parent, "Camera", "Camera permission was not granted.");
    }
    return granted;
}
#endif
