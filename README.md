# Browser Navigation & Session Snapshot Extension

Welcome to the **Browser Navigation** project! This repository contains a unique two-part project centered around browser session management and navigation mechanics. It features both a simulated C++ terminal application and a fully functional Chrome extension.

## 🚀 Project Overview

The project is divided into two distinct components:

1. **Browser Navigation (C++ Terminal App)**
   A C++ application that simulates the core functionalities of a modern web browser's navigation system. It models history, bookmarks, session management, and tab handling using fundamental data structures.

2. **Session Snapshot (Chrome Extension)**
   A practical, real-world browser extension for Google Chrome that allows users to capture, save, and restore their browser sessions (tabs and windows) with a single click.

---

## 💻 1. Browser Navigation (C++ App)

### Features
- **Tab Management:** Simulate opening, closing, and switching between tabs.
- **History Tracking:** Records visited pages and navigation sequences.
- **Bookmarks:** Save and retrieve favorite URLs.
- **Session History:** Persist and load browser sessions from local text files (`history.txt`, `bookmarks.txt`, `tabs.txt`, etc.).
- **Data Structures:** Heavily utilizes custom data structures (Stacks, Queues, Linked Lists) to emulate browser mechanics efficiently.

### File Structure
- `main.cpp` - The entry point of the terminal application.
- `browser.h` - Contains the core logic for the browser simulation.
- `fileManager.h` - Handles reading and writing data to local text files.
- `structures.h` - Definitions for the custom data structures used.
- `*.txt` - Local storage files acting as a primitive database.

### Usage
Compile the C++ code using any standard C++ compiler (e.g., GCC, MSVC):
```bash
g++ main.cpp -o main
./main
```

---

## 🌐 2. Session Snapshot (Chrome Extension)

### Features
- **One-Click Save:** Instantly save all currently open tabs in your window as a "Snapshot".
- **Session Restoration:** Easily restore previously saved sessions at any time.
- **Persistent Storage:** Utilizes Chrome's Local Storage API to keep snapshot data safe across browser restarts.
- **Clean UI:** A simple, intuitive popup interface built with HTML, CSS, and Vanilla JavaScript.

### File Structure
- `manifest.json` - The extension configuration file (Manifest V3).
- `popup.html` & `popup.css` - The user interface for the extension popup.
- `popup.js` - Logic for handling user interactions in the popup.
- `background.js` - Service worker for background tasks and state management.
- `icon.png` - The extension icon.

### Installation (Developer Mode)
1. Open Google Chrome and navigate to `chrome://extensions/`.
2. Enable **Developer mode** using the toggle switch in the top right corner.
3. Click on the **Load unpacked** button.
4. Select the `SessionSnapshotExtension` directory.
5. The extension "Session Snapshot" will now appear in your browser toolbar!

---

## 🛠️ Technologies Used
- **C++:** For the core logic and simulation of the browser navigator.
- **HTML, CSS, JavaScript:** For the Session Snapshot Chrome extension.
- **Chrome Extension APIs:** For interacting with browser tabs and local storage.

## 🤝 Contributing
Contributions are welcome! If you'd like to improve the C++ simulation or add new features to the Chrome extension, feel free to fork this repository and submit a pull request.