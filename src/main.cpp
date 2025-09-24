#include <QApplication>
#include "MainWindow.h"

#ifdef Q_OS_MACOS
  // No extra addLibraryPath() needed once we link the permission plugin statically.
  // (Avoids duplicate plugin discovery/unload edge cases)
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
