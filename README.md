# QtCameraDemo (Qt 6, Widgets)

A minimal, portable camera preview + still capture demo using **Qt Multimedia**.

## Features
- Detects first available camera
- Live preview inside a `QVideoWidget`
- Capture **JPEG/PNG** to disk on button press
- Pure Qt (no external libs)

## Requirements
- Qt **6.6+** (tested with **6.7.x**)
- CMake 3.21+

## Build

### Linux
```bash
mkdir -p build
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64
cmake --build build -j
./build/QtCameraDemo
````

### Windows (MSVC)

```bat
cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH=C:\Qt\6.x\msvc2022_64
cmake --build build
build\QtCameraDemo.exe
```

### macOS

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build -j
open build/QtCameraDemo.app
```

> For redistribution on macOS:
>
> ```
> /opt/homebrew/opt/qt/bin/macdeployqt build/QtCameraDemo.app -verbose=2
> ```

## Runtime permissions

* **macOS**: Info.plist declares `NSCameraUsageDescription`. First run will prompt for access.
  If you denied earlier: `tccutil reset Camera com.ialexpobad.qtcamerademo`
* **Windows / Linux**: Normally no per-app prompt; ensure your OS privacy and device permissions allow camera use.

## Code structure

* `CameraWidget` sets up `QMediaCaptureSession` → `QCamera` → `QVideoWidget` + `QImageCapture`.
* `PlatformPermissions_mac.mm` requests camera permission (macOS only).
* `StaticPlugins.cpp` imports the darwin camera-permission plugin for Homebrew Qt.
* `MainWindow` hosts the widget; `main.cpp` creates the app.

## Extend

* Add device selection (already in toolbar).
* Add video recording via `QMediaRecorder`.
* Port to QML by replacing the widget UI with `VideoOutput` + `MediaCaptureSession`.
