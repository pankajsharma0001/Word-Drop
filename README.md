# 🎮 Word Drop

**Word Drop** is a fast-paced reflex and typing game built using `graphics.h` in C++. Words fall from the top of the screen, and your goal is to type them before they hit the bottom. If you miss, you lose health — but if you succeed, you score points and recover health. The longer you survive, the faster the words fall!

---

## 📚 Table of Contents

- [🎮 Word Drop](#-word-drop)
- [🧠 Features](#-features)
- [🎮 Controls](#-controls)
- [🧾 Menu Options](#-menu-options)
- [🛠 Requirements](#-requirements)
- [🧰 Setup](#-setup)
- [🚀 How to Run](#-how-to-run)
  - [🔧 Using Makefile](#-using-makefile-recommended)
  - [🧰 Manual Compilation](#-manual-compilation-if-makefile-not-used)
- [📁 File Structure](#-file-structure)
- [🧑‍💻 Contributing](#-contributing)
- [📄 License](#-license)

---

## 🧠 Features

- ⌨️ Type falling words to eliminate them in real time  
- 💥 Words fall like meteors — increasing in speed as you score more  
- 🧠 Streak system: Type 10 perfect words in a row to earn a power-up
- ⚡ Power-Ups:
    • 🐢 Slow Time — slows down falling speed for a few seconds  
    • 💰 Score Boost — doubles your score temporarily
- ❤️ Health system: gain on success, lose on failure  
- 🔊 Sound effects for correct, incorrect, and game over events  
- 🧠 Custom word mode: enter your own words or sentences  
- 📈 High Score leaderboard saved between sessions  
- 🎯 Real-time HUD: score, health, accuracy, streaks  
- 🎵 Background music in menu (looping)  
- 🎬 Animated splash screen and game transition effects  
- 🛑 Pause (`Space`) and Restart (`R`) functionality  
- 📋 Instructions and UI navigation with both **mouse and keyboard**

---

## 🎮 Controls

| Key                  | Action                          |
|----------------------|---------------------------------|
| Type letters         | Build and match current word    |
| `Backspace`          | Delete last typed character     |
| `Space`              | Pause / Resume the game         |
| `R`                  | Restart after Game Over         |
| `F1`                 | Enable / Disable sound          |
| `Esc`                | Quit the game                   |
| `1` / `2` / Mouse    | Navigate menus and selections   |

---

## 🧾 Menu Options

- **Start Game** → Choose between:
  - Random Word Mode
  - Custom Word Mode (enter your own sentence!)
- **High Scores** → View top 5 player scores
- **Instructions** → Learn how to play
- **Exit** → Close the game

---

## 🛠 Requirements

- C++ compiler (e.g., g++, MinGW)  
- `graphics.h` library (WinBGIm)  
- Windows OS (uses `PlaySound()` from `winmm.lib`)  
- `make` utility (optional but recommended)

---

## 🧰 Setup

1. Install MinGW
   
   https://sourceforge.net/projects/mingw/
2. Add the Environment Variable path
     ```bash
     C:\MinGW\lib
3. Download graphics header file
   
   [Graphics Header Files.zip](https://github.com/user-attachments/files/21310839/Graphics.Header.Files.zip)
   
4. Open MinGW folder
     ```bash
     C:\MinGW
5. Put graphics.h and winbgim.h in include folder
     ```bash
     C:\MinGW\include
6. Put libbgi.a in lib folder
     ```bash
     C:\MinGW\lib
     
---

## 🚀 How to Run

### 🔧 Using Makefile (recommended)

1. Clone this repo  
2. Open terminal in the project folder  
3. Compile with:
   
   ```bash
   make
   
### 🧰 Manual Compilation (if Makefile not used)

  ```bash
    g++ main.cpp menu.cpp game.cpp -o WordDrop -lgraphics -lwinmm
  ```
---

## 📁 File Structure

<details>
<summary><code>.</code> (Project Root)</summary>
.  <br>
├── game.cpp # Game logic, scoring, rendering  <br>
├── menu.cpp # Menu, instructions, high score UI  <br>
├── main.cpp # Entry point  <br>
├── game.h # Game header  <br>
├── menu.h # Menu header  <br>
├── Makefile # Build script  <br>
├── README.md # This file  <br>
├── file/ # Game data files  <br>
├── resources/ # Images, sounds, animations  

</details>

---

<details>
<summary><code>file/</code></summary>

file/  
├── words.txt # Default word list  
├── custom.txt # Stores user's custom input sentence  
└── highscore.txt # Top 5 scores saved

</details>

---

<details>
<summary><code>resources/</code></summary>

resources/  
├── menu_background.bmp  
├── game_over.bmp  
├── background.bmp  
├── icon.ico  
├── correct.wav  
├── wrong.wav  
├── game_over.wav  
├── keyboard_typing.wav  
├── menu_music.wav  
└── frames/  
├── frame_000.bmp  
├── frame_001.bmp  
├── ...  
└── frame_029.bmp  
*NOTE:* `frame_000.bmp` to `frame_029.bmp` are used for background animation.
</details>

---

## 🧑‍💻 Contributing
Feel free to fork, modify, and contribute improvements! Bug fixes, new features, or even just better animations are welcome.

---

## 📄 License
This project is open-source and free to use for educational or personal use.
