# smart-file-organizer

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![C++17](https://img.shields.io/badge/C%2B%2B-17-brightgreen.svg)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey.svg)
![Version](https://img.shields.io/badge/Version-2.0.0-orange.svg)

A cross-platform background utility written in C++ that watches a directory and automatically sorts new files into categorized subdirectories. I built this because my Downloads folder was always a mess — now it stays organized without me doing anything.

Works on **Linux** and **Windows**. It runs in the background, waits for downloads to finish before touching files, handles multiple downloads at once, avoids overwriting anything, and logs every action.

---

## Download

**No compiler needed.** Just grab a pre-built binary from the [Releases page](https://github.com/Regentflame11/smart-file-organizer/releases/latest):

| Platform | File |
|----------|------|
| Linux (x86_64) | `smart-file-organizer-linux-x86_64.tar.gz` |
| Windows (x86_64) | `smart-file-organizer-windows-x86_64.zip` |

**On Linux:**
```bash
tar -xzf smart-file-organizer-linux-x86_64.tar.gz
./organizer
```

**On Windows:** Extract the zip and run `organizer.exe`.

---

## Features

- **Real-time directory monitoring** — watches any folder (defaults to `~/Downloads`)
- **Background service** — runs as a systemd service on Linux, or a background process on Windows
- **Download-safe** — waits for `.crdownload`, `.part`, and `.tmp` files to finish before moving them
- **Skips hidden/temp files** — Chromium temp files (`.org.chromium.*`) and dotfiles are left alone
- **Concurrent** — handles multiple files at once using threads
- **13 categories, 100+ extensions** — images, documents, videos, code, fonts, and more
- **No overwrites** — duplicate files get Chrome-style `(1)`, `(2)` naming
- **Logging** — every move is recorded in `organizer.log`
- **Custom directory** — point it at any folder, not just Downloads
- **Autostart** — one-command systemd setup on Linux

---

## File Categories

| Category     | Extensions |
|-------------|------------|
| Images      | `.jpg`, `.jpeg`, `.png`, `.gif`, `.bmp`, `.svg`, `.webp`, `.ico`, `.tiff`, `.heic`, `.avif`, `.raw`, `.cr2`, `.nef` |
| Documents   | `.pdf`, `.docx`, `.doc`, `.txt`, `.pptx`, `.ppt`, `.xlsx`, `.xls`, `.csv`, `.odt`, `.rtf`, `.md`, `.epub`, `.tex` |
| Videos      | `.mp4`, `.mkv`, `.avi`, `.mov`, `.wmv`, `.flv`, `.webm`, `.m4v`, `.mpg`, `.mpeg`, `.3gp`, `.srt`, `.sub`, `.ass`, `.vtt` |
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

Files with no extension and hidden files (starting with `.`) are intentionally left alone.

---

## Tech Stack

- **C++17** — `std::filesystem`, `std::thread`
- **Linux** — `inotify` for filesystem events
- **Windows** — `ReadDirectoryChangesW` (WinAPI)
- **UTF-8** strings throughout for cross-platform compatibility

---

## Linux

### Option 1: Download pre-built binary (recommended)

See the [Download](#download) section above. No compiler needed.

### Option 2: Build from source

```bash
# Ubuntu/Debian
sudo apt install g++ make

# Fedora
sudo dnf install gcc-c++ make

# Arch
sudo pacman -S gcc make
```

Build it:

```bash
make
```

### Usage

```bash
# Watch ~/Downloads (default)
./organizer

# Watch a custom directory
./organizer /path/to/your/folder

# Watch current directory
./organizer .
```

### Autostart with systemd

```bash
# Install and start watching ~/Downloads
./install.sh

# Or watch a custom directory
./install.sh /path/to/your/folder
```

Managing the service:

```bash
systemctl --user status  smart-file-organizer   # Check status
systemctl --user stop    smart-file-organizer   # Stop
systemctl --user restart smart-file-organizer   # Restart
journalctl --user -u     smart-file-organizer   # View logs
```

To remove:

```bash
./uninstall.sh
```

---

## Windows

### Prerequisites

Install [MSYS2](https://www.msys2.org/) or [MinGW-w64](https://www.mingw-w64.org/) to get `g++`.

### Build

```bash
g++ -std=c++17 -O2 organizer.cpp -o organizer.exe -mwindows
```

Or just use `make`.

### Usage

```cmd
:: Watch default Downloads folder
organizer.exe

:: Watch a custom directory
organizer.exe C:\Users\YourName\Desktop
```

---

## Stopping

**Linux:** Press `Ctrl+C`, or run `systemctl --user stop smart-file-organizer`

**Windows:** End the process from Task Manager, or run `taskkill /IM organizer.exe`

---

## Log File

All operations are logged to `organizer.log` in the watched directory.

```
[2026-05-31 14:32:01] Started watching: /home/surya/Downloads
[2026-05-31 14:32:15] Moved notes.pdf -> Documents
[2026-05-31 14:33:02] Moved vacation.mp4 -> Videos
[2026-05-31 14:33:45] Moved setup.deb -> Programs
```

---

## Project Structure

```
smart-file-organizer/
├── organizer.cpp                    # Main source (cross-platform)
├── Makefile                         # Build system (auto-detects OS)
├── install.sh                       # Linux: install + systemd service
├── uninstall.sh                     # Linux: clean removal
├── .github/workflows/release.yml    # CI: auto-builds binaries on release
├── LICENSE                          # MIT License
└── README.md
```

## License

MIT — see [LICENSE](LICENSE) for details.

## Author

**Surya Pranav** — [@Regentflame11](https://github.com/Regentflame11)
