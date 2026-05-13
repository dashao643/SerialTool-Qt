# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
qmake SerialTool2.pro
make          # or mingw32-make / nmake
```

Target environment: Qt 6.10.1, MSVC2022 or MinGW, C++17, Windows.

## Architecture

The app is a multi-mode serial/network/Bluetooth debugging tool with a tabbed send-item system.

### Core classes

- **`MainWindow`** (QMainWindow) — owns everything. Creates `SerialManager`, `NetworkManager`, `AppConfig` in `dataInit()`. All signal/slot wiring happens in `slotsInit()`. UI state updates via `do_UiUpdate(bool isSerialOpen)`. Send logic and data display live here.
- **`SerialManager`** (QObject) — wraps `QSerialPort`. Responsible for port enumeration, open/close, baud rate changes, and read/write. Emits `sgn_readyRead(QByteArray)` for incoming data and `serialStateChanged(bool)` for UI updates. Does NOT depend on MainWindow or any UI class.
- **`NetworkManager`** (QObject) — wraps `QTcpServer` + `QTcpSocket`. Currently TCP server mode (accepts incoming connections). Emits `sgn_readyRead` and `sgn_stateChange`. Network send path is not yet wired to MainWindow.
- **`AppConfig`** (QObject) — persists all settings to `setting.ini` via `QSettings`. Loads/saves window config, serial params, tab pages with their send items, and IAP file transfer config.
- **`TabPage`** (QWidget) — one tab in the main tab widget. Contains a `QListWidget` of send items with context menu (insert/delete).
- **`CustomItem`** (QWidget) — a single send item: text input + HEX checkbox + send button. Emits `sendDataRequest(content, model)` which MainWindow connects to `sendData()`.

### Data structures (`dataStructure.h`)

All shared enums and structs: `ComModel`, `SendModel`, `CheckDataIndex`, `SerialConfigStruct`, `ItemConfig`, `TabPageConfig`, `SendFile_t`, `Config_t`.

### Signals vs. direct calls pattern

`SerialManager` and `NetworkManager` have no knowledge of MainWindow. They emit signals (`sgn_readyRead`, `serialStateChanged`, etc.); MainWindow connects to them in `slotsInit()`. The send direction goes the other way: MainWindow calls `serialManager_->sendData()` directly.

### IAP (bin file transfer)

Orchestrated in `MainWindow::do_fileDownload()`. Sends a handshake command, waits for ACK via `waitAck()` (which pumps the event loop with `processEvents()`), then sends file in chunks with ACK after each. Uses `SendFileDialog` for configuration.

### Data display with batching

Incoming data from serial/network accumulates in `receiveBuffer_`. A single-shot `rxTimer_` fires after `spBox_Timeout` ms of silence, then `do_showReceivedData()` formats and displays the accumulated data at once.
