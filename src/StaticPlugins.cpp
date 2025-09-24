#include <QtPlugin>

#ifdef Q_OS_MACOS
// Import the darwin camera permission plugin ONLY when we linked it statically.
// (If Qt6::Permissions is present, we load it dynamically; do NOT import here.)
#  if defined(QT_STATICPLUGIN)
Q_IMPORT_PLUGIN(QDarwinCameraPermissionPlugin)
#  endif
#endif
