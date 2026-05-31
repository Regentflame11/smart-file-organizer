# 📂 smart-file-organizer

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![C++17](https://img.shields.io/badge/C%2B%2B-17-brightgreen.svg)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey.svg)
![Version](https://img.shields.io/badge/Version-2.0.0-orange.svg)

A cross-platform background utility written in C++ that automatically monitors a directory and organizes files into categorized subdirectories in real time.

Works on **Linux** and **Windows**. Runs silently in the background, waits for downloads to finish before moving files, handles multiple downloads concurrently, avoids overwriting files, and logs all operations.

---

## ⬇️ Download

**No compiler needed.** Grab a pre-built binary from the [Releases page](https://github.com/Regentflame11/smart-file-organizer/releases/latest):

| Platform | Download |
|----------|----------|
| 🐧 Linux (x86_64) | `smart-file-organizer-linux-x86_64.tar.gz` |
| 🪟 Windows (x86_64) | `smart-file-organizer-windows-x86_64.zip` |

**Linux — quick install:**
```bash
# Download and extract
tar -xzf smart-file-organizer-linux-x86_64.tar.gz

# Run it
./organizer

# Or install with autostart
./install.sh
```

**Windows:** Extract the zip and run `organizer.exe`.

---

## ✨ Features

- 📁 **Real-time monitoring** of any directory (defaults to `~/Downloads`)
- 🕶️ **Runs in the background** as a systemd service (Linux) or background process (Windows)
- ⏳ **Waits for downloads to finish** before moving files (detects `.crdownload`, `.part`, `.tmp`)
- ⚡ **Concurrent processing** — handles multiple downloads at once with multithreading
- 🗂️ **Automatic categorization** of 100+ file extensions into 13 categories
- 🔁 **No overwrites** — duplicates get Chrome-style `(1)`, `(2)` naming
- 📝 **Logging** — all operations recorded to `organizer.log`
- 🎯 **Custom directory support** — watch any folder, not just Downloads
- 🚀 **Auto-start on boot** via systemd (Linux)

---

## 🗂️ File Categories

| Category     | Extensions                                                          |
|-------------|----------------------------------------------------------------------|
| Images      | `.jpg`, `.jpeg`, `.png`, `.gif`, `.bmp`, `.svg`, `.webp`, `.ico`, `.tiff`, `.heic`, `.avif`, `.raw`, `.cr2`, `.nef` |
| Documents   | `.pdf`, `.docx`, `.doc`, `.txt`, `.pptx`, `.ppt`, `.xlsx`, `.xls`, `.csv`, `.odt`, `.rtf`, `.md`, `.epub`, `.tex` |
| Videos      | `.mp4`, `.mkv`, `.avi`, `.mov`, `.wmv`, `.flv`, `.webm`, `.m4v`, `.mpg`, `.mpeg`, `.3gp` |
| Music       | `.mp3`, `.wav`, `.aac`, `.flac`, `.ogg`, `.wma`, `.m4a`, `.opus`, `.aiff`, `.mid` |
| Archives    | `.zip`, `.rar`, `.7z`, `.tar`, `.gz`, `.bz2`, `.xz`, `.zst`, `.tgz` |
| Programs    | `.exe`, `.msi`, `.deb`, `.rpm`, `.appimage`, `.flatpakref`, `.snap`, `.dmg`, `.run` |
| Code        | `.cpp`, `.c`, `.h`, `.py`, `.java`, `.js`, `.ts`, `.go`, `.rs`, `.rb`, `.php`, `.html`, `.css`, `.json`, `.yaml`, `.sh`, `.sql`, `.vue`, `.dart`, `.zig`, and more |
| Disk_Images | `.iso`, `.img`, `.bin`, `.vmdk`, `.vdi`, `.vhd`, `.qcow2` |
| Fonts       | `.ttf`, `.otf`, `.woff`, `.woff2`, `.eot` |
| Design      | `.psd`, `.ai`, `.fig`, `.sketch`, `.xd`, `.blend`, `.obj`, `.fbx`, `.stl` |
| Torrents    | `.torrent` |
| Databases   | `.db`, `.sqlite`, `.sqlite3`, `.mdb` |
| Others      | Everything else |

---

## 🖥️ Technologies Used

- **C++17** with `std::filesystem`
- **Linux**: `inotify` for filesystem monitoring
- **Windows**: `ReadDirectoryChangesW` (WinAPI)
- **Multithreading**: `std::thread` for concurrent file processing
- **UTF-8** strings (cross-platform compatible)

---

## 🐧 Linux — Quick Start

### Option 1: Download pre-built binary (recommended)

See the [Download](#️-download) section above. No compiler needed.

### Option 2: Build from source

```bash
# Install build tools (Ubuntu/Debian)
sudo apt install g++ make

# Fedora: sudo dnf install gcc-c++ make
# Arch:   sudo pacman -S gcc make
```

#### Build

```bash
make
```

### Run

```bash
# Watch ~/Downloads (default)
./organizer

# Watch a custom directory
./organizer /path/to/your/folder

# Watch current directory
./organizer .
```

### Install with Autostart (systemd)

```bash
# Install and auto-start watching ~/Downloads
./install.sh

# Or specify a custom directory
./install.sh /path/to/your/folder
```

### Manage the Service

```bash
systemctl --user status  smart-file-organizer   # Check status
systemctl --user stop    smart-file-organizer   # Stop
systemctl --user restart smart-file-organizer   # Restart
journalctl --user -u     smart-file-organizer   # View logs
```

### Uninstall

```bash
./uninstall.sh
```

---

## 🪟 Windows — Quick Start

### Prerequisites

Install [MSYS2](https://www.msys2.org/) or [MinGW-w64](https://www.mingw-w64.org/) to get `g++`.

### Build (MinGW / MSYS2)

```bash
g++ -std=c++17 -O2 organizer.cpp -o organizer.exe -mwindows
```

Or use the Makefile:

```bash
make
```

### Run

```cmd
:: Watch default Downloads folder
organizer.exe

:: Watch a custom directory
organizer.exe C:\Users\YourName\Desktop
```

---

## 🛑 Stopping the Application

**Linux:**
- Press `Ctrl+C` if running in terminal
- Or: `systemctl --user stop smart-file-organizer`

**Windows:**
- End the process from Task Manager
- Or: `taskkill /IM organizer.exe`

---

## 📝 Log File

All file operations are logged to `organizer.log` inside the watched directory.

Example:
```
[2026-05-31 14:32:01] Started watching: /home/surya/Downloads
[2026-05-31 14:32:15] Moved notes.pdf -> Documents
[2026-05-31 14:33:02] Moved vacation.mp4 -> Videos
[2026-05-31 14:33:45] Moved setup.deb -> Programs
```

---

## 📁 Project Structure

```
smart-file-organizer/
├── organizer.cpp                    # Main source (cross-platform)
├── Makefile                         # Build system (auto-detects OS)
├── install.sh                       # Linux: install binary + systemd service
├── uninstall.sh                     # Linux: clean removal
├── .github/workflows/release.yml    # CI: auto-builds binaries on release
├── LICENSE                          # MIT License
└── README.md
```

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).

## 👤 Author

**Surya Pranav** — [@Regentflame11](https://github.com/Regentflame11)
