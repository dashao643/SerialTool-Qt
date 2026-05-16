# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
qmake SerialTool2.pro
make          # or mingw32-make / nmake
```

Target environment: Qt 6.10.1, MSVC2022 or MinGW, C++17, Windows.

The `.pro` file defines `QT_NO_DEBUG_OUTPUT` (suppresses `qDebug` in release) and `APP_VERSION` (used for the window title). Version is in `VERSION = 1.2.0`.

## Architecture

The app is a multi-mode serial/network/Bluetooth debugging tool with a tabbed send-item system. Mode switching is driven by `ComModel` enum (SERIAL/NETWORK/BLUETOOTH) via `cbBox_Model` combobox, which swaps `stackedWidget` pages to show per-mode UI controls.

### Core classes

- **`MainWindow`** (QMainWindow) — owns everything. Creates `SerialManager`, `NetworkManager`, `AppConfig` in `dataInit()`. All signal/slot wiring happens in `slotsInit()`. UI state updates via `do_UiUpdate(bool isOpen)`. Send logic and data display live here. Three timers: `infoTimer_` (single-shot, clears status label after 1s), `rxTimer_` (single-shot, batches incoming data for display), `sendTimer_` (periodic, for timed-repeat send). On close, `closeEvent` saves all config and tab pages via `AppConfig`.
- **`SerialManager`** (QObject) — wraps `QSerialPort`. Responsible for port enumeration, open/close, baud rate changes, and read/write. Emits `sgn_readyRead(QByteArray)` for incoming data and `sgn_serialStateChanged(bool)` for UI updates. Does NOT depend on MainWindow or any UI class. Serial port config (data bits, stop bits, parity, flow control) lives in `serialConfig_` and is configured via `SerialSettingDialog`.
- **`NetworkManager`** (QObject) — supports three modes via `CurNetworkModel` enum (TcpServer, TcpClient, UDP). Wraps `QTcpServer` + `QTcpSocket` (server/client) and `QUdpSocket`. Emits `sgn_readyRead`, `sgn_stateChange`, `sgn_btnStateChanged`. Send path is wired: `MainWindow::sendData()` calls `networkManager_->sendData()` when in NETWORK mode.
- **`AppConfig`** (QObject) — persists all settings to `setting.ini` via `QSettings`. Loads/saves window config, serial params, tab pages with their send items, IAP file transfer config, and network settings (local port, remote IP/port).
- **`TabPage`** (QWidget) — one tab in the main tab widget. Contains a `QListWidget` of send items with context menu (insert/delete). Emits `insertItem(int)` which MainWindow handles to insert a `CustomItem` at the given row.
- **`CustomItem`** (QWidget) — a single send item: remark label, text input, HEX checkbox, and send button. Emits `sendDataRequest(content, model)` which MainWindow connects to `sendData()`.
- **`SerialSettingDialog`** (QDialog) — config dialog for serial port parameters: data bits, stop bits, parity, flow control. Values are read back by MainWindow and written to `SerialManager::serialConfig_`.
- **`SendFileDialog`** (QDialog) — IAP configuration dialog. Collects file path, packet size, handshake command, expected ACK, and timeout. Emits `download(SendFile_t)` to trigger `MainWindow::do_fileDownload()`.

### Data structures (`dataStructure.h`)

All shared enums and structs: `ComModel` (SERIAL/NETWORK/BLUETOOTH), `SendModel` (HEX/ASCII), `CheckDataIndex` (NONE_CHECK/MODBUS_CRC16/ADD8), `SerialConfigStruct`, `ItemConfig`, `TabPageConfig`, `SendFile_t`, `Config_t`.

### Signals vs. direct calls pattern

`SerialManager` and `NetworkManager` have no knowledge of MainWindow. They emit signals (`sgn_readyRead`, `sgn_serialStateChanged`, etc.); MainWindow connects to them in `slotsInit()`. The send direction goes the other way: MainWindow calls `serialManager_->sendData()` / `networkManager_->sendData()` directly.

### IAP (bin file transfer)

Orchestrated in `MainWindow::do_fileDownload()`. Sends a handshake command, waits for ACK via `waitAck()` (which pumps the event loop with `processEvents()`), then sends file in chunks with ACK after each. The last chunk is padded with `0xFF` if undersized. During transfer, `isFileDownload` is true, causing incoming data to also accumulate in `fileReceiveBuffer_` for ACK checking.

### Data display with batching

Incoming data from serial/network accumulates in `receiveBuffer_`. A single-shot `rxTimer_` fires after `spBox_Timeout` ms of silence, then `do_showReceivedData()` formats (HEX or ASCII) and displays the accumulated data at once, auto-scrolling to the bottom.

### Check data (HEX mode only)

When sending in HEX mode, `appendCheckData()` appends a checksum based on `cbBox_DataCheck` selection: Modbus CRC16 (polynomial 0xA001, little-endian) or ADD8 (simple byte sum). No checksum is appended in ASCII mode.
