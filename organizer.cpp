#define UNICODE
#define _UNICODE //not mandatory incase of non-english characters in file name
#include <windows.h>  //gives access to win system APIs
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <ctime>
#include <thread>



namespace fs = std::filesystem;


//checks if files are temp and if it is waits until full download
bool isTempFile(const std::wstring& filename) {
    return filename.find(L".crdownload") != std::wstring::npos ||
           filename.find(L".tmp") != std::wstring::npos;
}

//make the code wait until download finishes
bool isFileReady(const std::filesystem::path& filePath) {
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
        return false; // File still locked
    }

    CloseHandle(file);
    return true;
}

//adds the actions into logs
void logAction(const std::wstring& message) {
    std::wofstream logFile(L"organizer.log", std::ios::app);
    if (!logFile.is_open()) return;

    std::time_t now = std::time(nullptr);
    wchar_t timeBuf[100];
    wcsftime(timeBuf, 100, L"%Y-%m-%d %H:%M:%S", std::localtime(&now));

    logFile << L"[" << timeBuf << L"] " << message << std::endl;
}

//returns the extension of the file
std::wstring getCategory(const std::wstring& ext) {
    // Images
    if (ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".gif")
        return L"Images";

    // Documents
    if (ext == L".pdf" || ext == L".docx" || ext == L".txt" || ext == L".pptx")
        return L"Documents";

    // Videos
    if (ext == L".mp4" || ext == L".mkv" || ext == L".avi")
        return L"Videos";

    // Music
    if (ext == L".mp3" || ext == L".wav" || ext == L".aac")
        return L"Music";

    // Archives
    if (ext == L".zip" || ext == L".rar" || ext == L".7z")
        return L"Archives";

    // Programs
    if (ext == L".exe" || ext == L".msi")
        return L"Programs";

    // Code files
    if (ext == L".cpp" || ext == L".c" || ext == L".py" ||
        ext == L".java" || ext == L".js")
        return L"Code";

    return L"Others";
}
//if a file exists with same name it adds numbers at the end instead of replacing them
std::filesystem::path getUniquePath(const std::filesystem::path& targetPath) {
    if (!std::filesystem::exists(targetPath)) {
        return targetPath;
    }

    std::filesystem::path parent = targetPath.parent_path();
    std::wstring stem = targetPath.stem().wstring();
    std::wstring ext = targetPath.extension().wstring();

    int count = 1;
    std::filesystem::path newPath;

    do {
        newPath = parent / (stem + L" (" + std::to_wstring(count) + L")" + ext);
        count++;
    } while (std::filesystem::exists(newPath));

    return newPath;
}

void processFile(const std::filesystem::path& watchDir,
                 const std::wstring& filename) {

    if (isTempFile(filename)) return;

    std::filesystem::path fullPath = watchDir / filename;//adds the filename with file addrerss

   // std::wcout << L"[THREAD] New file: " << filename << L" (waiting...)" << std::endl; 

    // Wait until file is ready
    while (!isFileReady(fullPath)) {
        Sleep(1000);//tries to read every 1 sec
    }

    std::wstring extension = fullPath.extension().wstring();
    std::wstring category = getCategory(extension);

    std::filesystem::path targetDir = watchDir / category;
    std::filesystem::create_directory(targetDir); //if target directory doesnt exits it creates one

    std::filesystem::path targetPath =
        getUniquePath(targetDir / fullPath.filename());

    try {
        std::filesystem::rename(fullPath, targetPath);//moves the file to respective place

        std::wstring logMsg =
            L"Moved " + filename + L" -> " + category;

       // std::wcout << logMsg << std::endl;
        logAction(logMsg);

    } catch (...) {
       // std::wcout << L"Failed to move " << filename << std::endl;
    }
}


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdLine, int) {
    
    wchar_t* userProfile = _wgetenv(L"USERPROFILE");

std::filesystem::path watchDir =
    std::filesystem::path(userProfile) / L"Downloads";

    if (!fs::exists(watchDir) || !fs::is_directory(watchDir)) { //checks if download folder exists
        std::cout << "Invalid directory.\n";
        return 1;
    }

    std::wcout << L"Watching directory: " << watchDir.wstring() << std::endl;
//these handle gives permission to monitor the directory
    HANDLE dirHandle = CreateFileW(
        watchDir.wstring().c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, //this ensures that no other application is being blocked by this
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (dirHandle == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to open directory handle.\n";
        return 1;
    }
    //NOTIFICATION BUFFER
    char buffer[1024];
    DWORD bytesReturned;
    //Makes it a persistent background process
    while (true) {
        //returns when there is change in files
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
                //if a new file appears extracts its filename
            if (info->Action == FILE_ACTION_ADDED) {
    std::wstring filename(
        info->FileName,
        info->FileNameLength / sizeof(WCHAR)
    );

    // Launch a detached thread for each file
    std::thread(processFile, watchDir, filename).detach();
}


        }
    }

    CloseHandle(dirHandle);
    return 0;
}
