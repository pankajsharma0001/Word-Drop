#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <string>
#include "menu.h"
#include "game.h"

void splashScreen() {
    cleardevice();
    setbkcolor(BLACK);
    setcolor(LIGHTCYAN);
    settextstyle(BOLD_FONT, HORIZ_DIR, 5);

    const char* title = "Typing Speed Challenge";
    int x = 800 / 2 - 300;
    int y = 600 / 2 - 80;

    std::string typed = "";

    PlaySound(TEXT("resources/keyboard_typing.wav"), NULL, SND_FILENAME | SND_ASYNC);

    // Typing effect with fade glow
    for (int i = 0; title[i] != '\0'; i++) {
        typed += title[i];
        // cleardevice();
        setcolor(LIGHTCYAN);
        for (int glow = 0; glow < 3; glow++) {
            outtextxy(x - glow, y - glow, (char*)typed.c_str());
            outtextxy(x + glow, y + glow, (char*)typed.c_str());
        }
        delay(100);
    }

    delay(500);

    // Loading text
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    setcolor(WHITE);
    outtextxy(800 / 2 - 50, y + 80, (char*)"Loading...");

    // Loading bar with animation glow
    int barX = 800 / 2 - 150;
    int barY = y + 110;
    for (int i = 0; i <= 300; i += 10) {
        setfillstyle(SOLID_FILL, i % 30 < 15 ? LIGHTGREEN : GREEN);
        bar(barX, barY, barX + i, barY + 20);
        delay(40);
    }

    delay(500);
    cleardevice();
}

void drawMenu(int hoverIndex = -1) {
    setcolor(GREEN);
    settextstyle(BOLD_FONT, HORIZ_DIR, 4);
    outtextxy(220, 100, (char*)"TYPING SPEED GAME");

    int x = 250, width = 270, height = 40;
    int y1 = 180, gap = 50;

    const char* labels[] = {
        "1. Start Game",
        "2. High Scores",
        "3. Instructions",
        "4. Exit"
    };

    setfillstyle(SOLID_FILL, BLACK);
    for (int i = 0; i < 4; ++i) {
        int y = y1 + i * gap;

        // Change color if hovered
        if (i == hoverIndex){
            // setfillstyle(SOLID_FILL, LIGHTRED);
            setcolor(RED);
        }
        else{
            // setfillstyle(SOLID_FILL, BLUE);
            setcolor(WHITE);
        }

        bar(x, y, x + width, y + height);

        // setcolor(WHITE);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        outtextxy(x + 10, y + 10, (char*)labels[i]);
    }
}

void showInstructions() {
    setcolor(GREEN);
    settextstyle(BOLD_FONT, HORIZ_DIR, 4);
    outtextxy(270, 120, (char*)"Instructions");

    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    
    outtextxy(100, 200, (char*)" 1 / 2 / Mouse Click : select game mode");
    outtextxy(100, 230, (char*)" 'Backspace'    : delete the last character");
    outtextxy(100, 260, (char*)" 'Esc'          : exit the game");
    outtextxy(100, 290, (char*)" 'Space'        : pause/resume");
    outtextxy(100, 320, (char*)" 'R'            : restart the game");
    outtextxy(100, 350, (char*)" 'F1'           : toggle sound on/off");
    outtextxy(100, 380, (char*)" 10 word streak : earn a power-up");

    setcolor(YELLOW);
    outtextxy(150, 410, (char*)"Press any key to return to the menu...");
}

void showMenu() {
    loadHighScores();
    GameState state = MENU;
    initwindow(800, 600, (char*)"Typing Speed Game");

    void* bgBuffer = malloc(imagesize(0, 0, 800, 600));
    readimagefile("resources/menu_background.bmp", 0, 0, 800, 600);
    getimage(0, 0, 800, 600, bgBuffer);

    HWND hwnd = GetForegroundWindow();
    HICON hIcon = (HICON)LoadImage(NULL, "resources/icon.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    if (hIcon) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }

    splashScreen();
    PlaySound(TEXT("resources/menu_music.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC);

    while (state != EXIT) {
        if (state == MENU) {
            int hover = -1, lastHover = -2;

            putimage(0, 0, bgBuffer, COPY_PUT); // draw once
            drawMenu(hover);

            while (1) {
                bool redraw = false;

                // Mouse movement detection
                if (ismouseclick(WM_MOUSEMOVE)) {
                    int mx, my;
                    getmouseclick(WM_MOUSEMOVE, mx, my);

                    int newHover = -1;
                    int x = 250, width = 300, height = 40, y1 = 180, gap = 50;
                    for (int i = 0; i < 4; ++i) {
                        int y = y1 + i * gap;
                        if (mx >= x && mx <= x + width && my >= y && my <= y + height) {
                            newHover = i;
                            break;
                        }
                    }

                    if (newHover != hover) {
                        hover = newHover;
                        redraw = true;
                    }
                }

                // Redraw only on hover change
                if (redraw) {
                    putimage(0, 0, bgBuffer, COPY_PUT);
                    drawMenu(hover);
                }

                // Mouse click
                if (ismouseclick(WM_LBUTTONDOWN)) {
                    int mx, my;
                    getmouseclick(WM_LBUTTONDOWN, mx, my);

                    int x = 250, width = 300, height = 40, y1 = 180, gap = 50;
                    for (int i = 0; i < 4; ++i) {
                        int y = y1 + i * gap;
                        if (mx >= x && mx <= x + width && my >= y && my <= y + height) {
                            state = (GameState)(i + 1);
                            break;
                        }
                    }
                    if (state != MENU) break;
                }

                // Keyboard fallback
                if (kbhit()) {
                    char ch = getch();
                    if (ch == '1') { state = PLAYING; break; }
                    if (ch == '2') { state = HIGHSCORES; break; }
                    if (ch == '3') { state = INSTRUCTIONS; break; }
                    if (ch == '4' || ch == 27) { state = EXIT; break; }
                }

                delay(30);
            }
        }

        if (state == PLAYING) {
            cleardevice();
            stopSound();
            showSelectionMenu();
            PlaySound(TEXT("resources/menu_music.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC);
            state = MENU;
        } else if (state == HIGHSCORES) {
            putimage(0, 0, bgBuffer, COPY_PUT);
            int y = showHighScores();
            outtextxy(800 / 2 - 180, y + 40, (char*)"Press any key to return to menu.");
            outtextxy(800 / 2 - 180, y + 70, (char*)"Or click anywhere to go back.");

            while (true) {
                if (kbhit()) {
                    getch();
                    break;
                }
                if (ismouseclick(WM_LBUTTONDOWN)) {
                    clearmouseclick(WM_LBUTTONDOWN);  // clear the event
                    break;
                }
                delay(50);
            }
            state = MENU;
        } else if (state == INSTRUCTIONS) {
            putimage(0, 0, bgBuffer, COPY_PUT);
            showInstructions();

            while (true) {
                if (kbhit()) {
                    getch();
                    break;
                }
                if (ismouseclick(WM_LBUTTONDOWN)) {
                    clearmouseclick(WM_LBUTTONDOWN);  // clear the event
                    break;
                }
                delay(50);
            }
            state = MENU;
        }
    }

    free(bgBuffer);
    closegraph();
}

