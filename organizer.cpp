// Smart File Organizer v2.0.0
// Cross-platform (Linux + Windows) automatic file organizer
// Monitors a directory and moves new files into categorized subdirectories.

#ifdef _WIN32
#define UNICODE
#define _UNICODE
#include <windows.h>
#else
#include <sys/inotify.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#endif

#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cstdlib>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────
//  Should we skip this file?
// ─────────────────────────────────────────────
// Skip hidden files, browser temp files, and system files.
// Chromium creates temp files like ".org.chromium.Chromium.XXXXXX" during
// downloads — moving these breaks active downloads.
bool shouldSkipFile(const std::string& filename) {
    // Skip hidden files (dotfiles) — these are system/temp files on Linux
    // This catches Chromium temp files like .org.chromium.Chromium.XXXXXX
    if (!filename.empty() && filename[0] == '.') return true;

    // Convert to lowercase for reliable matching
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // Browser temp file patterns
    if (lower.find(".crdownload") != std::string::npos) return true; // Chrome
    if (lower.find(".part")       != std::string::npos) return true; // Firefox
    if (lower.find(".tmp")        != std::string::npos) return true; // General temp
    if (lower.find(".download")   != std::string::npos) return true; // Safari / misc

    return false;
}

// ─────────────────────────────────────────────
//  File readiness check
// ─────────────────────────────────────────────
// Ensures the file is fully written before we try to move it.
bool isFileReady(const fs::path& filePath) {
#ifdef _WIN32
    // Windows: try to open file with exclusive read access.
    // If it's still being written to, this will fail.
    HANDLE file = CreateFileW(
        filePath.wstring().c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) {
        return false; // File still locked by another process
    }

    CloseHandle(file);
    return true;
#else
    // Linux: check if file size is stable across two checks.
    // If the file is still being written, its size will keep changing.
    try {
        if (!fs::exists(filePath)) return false;

        auto size1 = fs::file_size(filePath);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (!fs::exists(filePath)) return false;

        auto size2 = fs::file_size(filePath);
        return (size1 == size2) && (size1 > 0);
    } catch (...) {
        return false;
    }
#endif
}

// ─────────────────────────────────────────────
//  Logging
// ─────────────────────────────────────────────
void logAction(const std::string& message, const fs::path& logPath) {
    std::ofstream logFile(logPath, std::ios::app);
    if (!logFile.is_open()) return;

    std::time_t now = std::time(nullptr);
    char timeBuf[100];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&now));

    logFile << "[" << timeBuf << "] " << message << std::endl;
}

// ─────────────────────────────────────────────
//  File categorization
// ─────────────────────────────────────────────
std::string getCategory(const std::string& ext) {
    // Normalize to lowercase for case-insensitive matching
    std::string e = ext;
    std::transform(e.begin(), e.end(), e.begin(), ::tolower);

    // Images
    if (e == ".jpg"  || e == ".jpeg" || e == ".png"  || e == ".gif"  ||
        e == ".bmp"  || e == ".svg"  || e == ".webp" || e == ".ico"  ||
        e == ".tiff" || e == ".tif"  || e == ".heic" || e == ".heif" ||
        e == ".raw"  || e == ".cr2"  || e == ".nef"  || e == ".avif")
        return "Images";

    // Documents
    if (e == ".pdf"  || e == ".docx" || e == ".doc"  || e == ".txt"  ||
        e == ".pptx" || e == ".ppt"  || e == ".xlsx" || e == ".xls"  ||
        e == ".csv"  || e == ".odt"  || e == ".ods"  || e == ".odp"  ||
        e == ".rtf"  || e == ".md"   || e == ".epub" || e == ".pages"||
        e == ".key"  || e == ".numbers" || e == ".tex" || e == ".log")
        return "Documents";

    // Videos (including subtitles)
    if (e == ".mp4"  || e == ".mkv"  || e == ".avi"  || e == ".mov"  ||
        e == ".wmv"  || e == ".flv"  || e == ".webm" || e == ".m4v"  ||
        e == ".mpg"  || e == ".mpeg" || e == ".3gp"  || e == ".ts"  ||
        e == ".srt"  || e == ".sub"  || e == ".ass"  || e == ".vtt")
        return "Videos";

    // Music / Audio
    if (e == ".mp3"  || e == ".wav"  || e == ".aac"  || e == ".flac" ||
        e == ".ogg"  || e == ".wma"  || e == ".m4a"  || e == ".opus" ||
        e == ".aiff" || e == ".mid"  || e == ".midi")
        return "Music";

    // Archives
    if (e == ".zip"  || e == ".rar"  || e == ".7z"   || e == ".tar"  ||
        e == ".gz"   || e == ".bz2"  || e == ".xz"   || e == ".zst"  ||
        e == ".tgz"  || e == ".lz4"  || e == ".cab"  || e == ".z")
        return "Archives";

    // Programs / Installers
    if (e == ".exe"  || e == ".msi"  || e == ".deb"  || e == ".rpm"  ||
        e == ".appimage"             || e == ".flatpakref"            ||
        e == ".snap" || e == ".dmg"  || e == ".pkg"  || e == ".run")
        return "Programs";

    // Code / Scripts
    if (e == ".cpp"  || e == ".c"    || e == ".h"    || e == ".hpp"  ||
        e == ".py"   || e == ".java" || e == ".js"   || e == ".ts"   ||
        e == ".jsx"  || e == ".tsx"  || e == ".go"   || e == ".rs"   ||
        e == ".rb"   || e == ".php"  || e == ".swift"|| e == ".kt"   ||
        e == ".cs"   || e == ".html" || e == ".css"  || e == ".scss" ||
        e == ".json" || e == ".xml"  || e == ".yaml" || e == ".yml"  ||
        e == ".sql"  || e == ".sh"   || e == ".bash" || e == ".lua"  ||
        e == ".r"    || e == ".toml" || e == ".ini"  || e == ".cfg"  ||
        e == ".vue"  || e == ".svelte" || e == ".dart" || e == ".zig")
        return "Code";

    // Disk Images
    if (e == ".iso"  || e == ".img"  || e == ".bin"  || e == ".vmdk" ||
        e == ".vdi"  || e == ".vhd"  || e == ".qcow2")
        return "Disk_Images";

    // Fonts
    if (e == ".ttf"  || e == ".otf"  || e == ".woff" || e == ".woff2"||
        e == ".eot")
        return "Fonts";

    // Design / 3D
    if (e == ".psd"  || e == ".ai"   || e == ".fig"  || e == ".sketch"||
        e == ".xd"   || e == ".blend"|| e == ".obj"  || e == ".fbx"  ||
        e == ".stl"  || e == ".3ds")
        return "Design";

    // Torrents
    if (e == ".torrent" || e == ".magnet")
        return "Torrents";

    // Databases
    if (e == ".db"   || e == ".sqlite" || e == ".sqlite3" || e == ".mdb")
        return "Databases";

    return "Others";
}

// ─────────────────────────────────────────────
//  Unique path generation
// ─────────────────────────────────────────────
// If a file with the same name already exists in the target directory,
// append (1), (2), etc. — just like Chrome does.
fs::path getUniquePath(const fs::path& targetPath) {
    if (!fs::exists(targetPath)) {
        return targetPath;
    }

    fs::path parent = targetPath.parent_path();
    std::string stem = targetPath.stem().string();
    std::string ext  = targetPath.extension().string();

    int count = 1;
    fs::path newPath;

    do {
        newPath = parent / (stem + " (" + std::to_string(count) + ")" + ext);
        count++;
    } while (fs::exists(newPath));

    return newPath;
}

// ─────────────────────────────────────────────
//  Process a single file
// ─────────────────────────────────────────────
void processFile(const fs::path watchDir, const std::string filename,
                 const fs::path logPath) {
    // Skip hidden files, browser temp files, etc.
    if (shouldSkipFile(filename)) return;

    fs::path fullPath = watchDir / filename;

    // Wait until file is ready (max ~60 seconds, then give up)
    int attempts = 0;
    while (!isFileReady(fullPath) && attempts < 60) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        attempts++;
    }

    if (!fs::exists(fullPath)) return; // File disappeared (was temp, got renamed, etc.)

    // Skip directories
    if (fs::is_directory(fullPath)) return;

    std::string extension = fullPath.extension().string();
    if (extension.empty()) return; // No extension, skip (binaries, temp files, etc.)

    std::string category = getCategory(extension);

    fs::path targetDir  = watchDir / category;
    fs::create_directories(targetDir);

    fs::path targetPath = getUniquePath(targetDir / fullPath.filename());

    try {
        fs::rename(fullPath, targetPath);

        std::string logMsg = "Moved " + filename + " -> " + category;
        std::cout << "  ✓ " << logMsg << std::endl;
        logAction(logMsg, logPath);

    } catch (const std::exception& e) {
        std::string errMsg = "Failed to move " + filename + ": " + e.what();
        std::cerr << "  ✗ " << errMsg << std::endl;
        logAction(errMsg, logPath);
    }
}

// ─────────────────────────────────────────────
//  Get default Downloads directory
// ─────────────────────────────────────────────
fs::path getDefaultDownloadsDir() {
#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
#else
    const char* home = std::getenv("HOME");
#endif

    if (!home) {
        std::cerr << "Error: Could not determine home directory." << std::endl;
        std::exit(1);
    }

    return fs::path(home) / "Downloads";
}

// ─────────────────────────────────────────────
//  Usage / Help
// ─────────────────────────────────────────────
void printUsage(const char* prog) {
    std::cout
        << "\n"
        << "  Smart File Organizer v2.0.0\n"
        << "  Automatically organizes files into categorized subdirectories.\n\n"
        << "  USAGE:\n"
        << "    " << prog << " [OPTIONS] [DIRECTORY]\n\n"
        << "  OPTIONS:\n"
        << "    -h, --help       Show this help message\n"
        << "    -v, --version    Show version number\n\n"
        << "  ARGUMENTS:\n"
        << "    DIRECTORY        Path to watch (default: ~/Downloads)\n\n"
        << "  EXAMPLES:\n"
        << "    " << prog << "                     # Watch ~/Downloads\n"
        << "    " << prog << " /home/user/Desktop  # Watch custom directory\n"
        << "    " << prog << " .                   # Watch current directory\n\n"
        << "  CATEGORIES:\n"
        << "    Images, Documents, Videos, Music, Archives, Programs,\n"
        << "    Code, Disk_Images, Fonts, Design, Torrents, Databases, Others\n"
        << std::endl;
}

// ─────────────────────────────────────────────
//  Graceful shutdown (Linux)
// ─────────────────────────────────────────────
#ifndef _WIN32
volatile sig_atomic_t g_running = 1;

void signalHandler(int /*signum*/) {
    g_running = 0;
}
#endif

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {
    fs::path watchDir;

    // ── Parse CLI arguments ──
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        if (arg == "-v" || arg == "--version") {
            std::cout << "Smart File Organizer v2.0.0" << std::endl;
            return 0;
        }
        // Anything else is treated as the directory path
        watchDir = fs::absolute(arg);
    }

    // Default to ~/Downloads if no directory given
    if (watchDir.empty()) {
        watchDir = getDefaultDownloadsDir();
    }

    // Validate directory
    if (!fs::exists(watchDir) || !fs::is_directory(watchDir)) {
        std::cerr << "Error: Not a valid directory: " << watchDir << std::endl;
        return 1;
    }

    fs::path logPath = watchDir / "organizer.log";

    // ── Startup banner ──
    std::cout << "\n"
              << "  ╔══════════════════════════════════════╗\n"
              << "  ║     Smart File Organizer v2.0.0      ║\n"
              << "  ╚══════════════════════════════════════╝\n"
              << "\n"
              << "  Watching : " << watchDir << "\n"
              << "  Log file : " << logPath << "\n"
              << "  Press Ctrl+C to stop.\n"
              << std::endl;

    logAction("Started watching: " + watchDir.string(), logPath);

    // ══════════════════════════════════════════
    //  Platform-specific directory monitoring
    // ══════════════════════════════════════════

#ifdef _WIN32
    // ── Windows: ReadDirectoryChangesW ──
    HANDLE dirHandle = CreateFileW(
        watchDir.wstring().c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (dirHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Failed to open directory handle." << std::endl;
        return 1;
    }

    char buffer[4096];
    DWORD bytesReturned;

    while (true) {
        if (ReadDirectoryChangesW(
                dirHandle,
                buffer,
                sizeof(buffer),
                FALSE,
                FILE_NOTIFY_CHANGE_FILE_NAME,
                &bytesReturned,
                NULL,
                NULL
            )) {

            FILE_NOTIFY_INFORMATION* info =
                reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

            do {
                if (info->Action == FILE_ACTION_ADDED) {
                    std::wstring wfilename(
                        info->FileName,
                        info->FileNameLength / sizeof(WCHAR)
                    );

                    // Convert wide string to UTF-8 string
                    int size_needed = WideCharToMultiByte(
                        CP_UTF8, 0, wfilename.c_str(),
                        static_cast<int>(wfilename.size()),
                        nullptr, 0, nullptr, nullptr);
                    std::string filename(size_needed, 0);
                    WideCharToMultiByte(
                        CP_UTF8, 0, wfilename.c_str(),
                        static_cast<int>(wfilename.size()),
                        &filename[0], size_needed, nullptr, nullptr);

                    std::thread(processFile, watchDir, filename, logPath)
                        .detach();
                }

                if (info->NextEntryOffset == 0) break;
                info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                    reinterpret_cast<char*>(info) + info->NextEntryOffset
                );
            } while (true);
        }
    }

    CloseHandle(dirHandle);

#else
    // ── Linux: inotify ──
    // Use sigaction() instead of signal() so that blocking read() calls
    // are NOT automatically restarted after a signal (no SA_RESTART).
    // This ensures Ctrl+C actually interrupts the read() and lets us exit.
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // No SA_RESTART — read() must return EINTR
    sigaction(SIGINT,  &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    int inotifyFd = inotify_init();
    if (inotifyFd < 0) {
        std::cerr << "Error: Failed to initialize inotify." << std::endl;
        return 1;
    }

    // Watch for new files created or moved into the directory
    int watchDesc = inotify_add_watch(
        inotifyFd, watchDir.c_str(),
        IN_CREATE | IN_MOVED_TO
    );

    if (watchDesc < 0) {
        std::cerr << "Error: Failed to watch directory: " << watchDir
                  << std::endl;
        close(inotifyFd);
        return 1;
    }

    const size_t BUF_LEN = 4096;
    char buffer[BUF_LEN]
        __attribute__((aligned(__alignof__(struct inotify_event))));

    while (g_running) {
        ssize_t length = read(inotifyFd, buffer, BUF_LEN);

        if (length < 0) {
            if (errno == EINTR) continue; // Interrupted by signal, check g_running
            std::cerr << "Error reading inotify events." << std::endl;
            break;
        }

        ssize_t i = 0;
        while (i < length) {
            struct inotify_event* event =
                reinterpret_cast<struct inotify_event*>(&buffer[i]);

            if (event->len > 0 && !(event->mask & IN_ISDIR)) {
                std::string filename(event->name);

                // Brief delay to let the file finish being created/written
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                std::thread(processFile, watchDir, filename, logPath).detach();
            }

            i += sizeof(struct inotify_event) + event->len;
        }
    }

    inotify_rm_watch(inotifyFd, watchDesc);
    close(inotifyFd);
    logAction("Stopped watching: " + watchDir.string(), logPath);
    std::cout << "  Stopped." << std::endl;

#endif

    return 0;
}
