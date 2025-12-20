📂 Smart File Organizer (Windows)

A background Windows utility written in C++ that automatically monitors the Downloads folder and organizes files into categorized directories in real time.

The application runs silently in the background, waits for downloads to finish before moving files, handles multiple downloads concurrently, avoids overwriting files, and logs all operations.




✨ Features :

📁 Real-time monitoring of the Downloads folder

🕶️ Runs silently in the background (no console window)

⏳ Waits for downloads to finish before moving files

⚡ Handles multiple downloads concurrently using multithreading

🗂️ Automatically categorizes files by type

🔁 Prevents overwriting using Chrome-style (1), (2) naming

📝 Logs all operations to organizer.log

🚀 Automatically starts with Windows (on first launch)



🗂️ File Categories :
Category	        Extensions
Images	        .jpg, .jpeg, .png, .gif
Documents	    .pdf, .docx, .txt, .pptx
Videos	        .mp4, .mkv, .avi
Music	        .mp3, .wav, .aac
Archives	    .zip, .rar, .7z
Programs	    .exe, .msi
Code	        .cpp, .c, .py, .java, .js
Others	        All remaining files



🖥️ Technologies Used:

C++

Windows API (WinAPI)

ReadDirectoryChangesW

std::filesystem

Multithreading (std::thread)

Unicode (UTF-16)

▶️ Build (MinGW / MSYS2):
```bash

g++ -std=gnu++17 organizer.cpp -o organizer -mwindows
```
▶️ Run:

organizer.exe


The application automatically monitors:

C:\Users\<username>\Downloads


and organizes files as soon as downloads complete.

🛑 Stop the Application:

You can stop the application using any standard Windows method:

End the process from Task Manager

Or use:
```bash
taskkill /IM organizer.exe
```

📝 Logging:

All file operations are logged to:

organizer.log


Example:

[2025-01-10 14:32:01] Moved notes.pdf -> Documents


👤 Author

Surya Pranav
C++ | Windows Systems Programming
