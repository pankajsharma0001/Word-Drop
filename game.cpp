#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") // Link the Windows Multimedia library
#include <iostream>
#include <graphics.h>
#include <conio.h>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <algorithm>
#include "game.h"

using namespace std;

int allowedColors[] = {
    GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY,
    DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, WHITE};

struct Word
{
    string text;
    int x, y;
    bool active;
    int color;
};

struct HighScore
{
    string name;
    int score;
};

vector<HighScore> highScores;
vector<string> dictionary;
vector<Word> words;

string typed = "";

int currentPage = 0;

int score = 0, health = 5, fallSpeed = 1, frameCount = 0, lastMilestone = 0;
int screenWidth = 800, screenHeight = 600;

bool showCursor = true;
bool paused = false;
bool gameOver = false;
bool restartRequested = false;
int totalKeystrokes = 0;
int correctKeystrokes = 0;
int longestStrick = 0;
int currentStrick = 0;
int powerUpStreak = 0; // Count consecutive power-ups
bool soundEnabled = true;
int previousSpeed = fallSpeed;
int glowTimer = 0;
int originalFallSpeed = 1;

bool slowTimeActive = false;
bool scoreBoostActive = false;
int slowTimeDuration = 5000;  // milliseconds
int scoreBoostDuration = 5000;
unsigned long powerUpStartTime = 0;

bool showPowerUpMessage = false;
char currentPowerUpMessage[50];
unsigned long messageStartTime = 0;
int messageDuration = 1200;  // in milliseconds (1.2 seconds)

int messageX = 0;  // X position of message
int messageTargetX = 350; // Center position
int messageStartX = -300; // Start from off-screen
int messageEndX = 800;    // Slide out to the right

void loadWordsFromFile(const string &filename)
{
    ifstream fin(filename);
    string word;
    dictionary.clear();
    while (getline(fin, word))
    {
        if (!word.empty())
        {
            dictionary.push_back(word);
        }
    }
    fin.close();
}

void loadHighScores()
{
    ifstream fin("file/highscore.txt");
    highScores.clear();
    string name;
    int score;
    while (fin >> name >> score)
    {
        highScores.push_back({name, score});
    }
    fin.close();
}

void saveHighScores()
{
    ofstream fout("file/highscore.txt");
    for (auto &entry : highScores)
    {
        fout << entry.name << " " << entry.score << endl;
    }
    fout.close();
}

void insertHighScore(const string &name, int score)
{
    highScores.push_back({name, score});
    sort(highScores.begin(), highScores.end(), [](HighScore a, HighScore b)
         { return a.score > b.score; });
    if (highScores.size() > 5)
    {
        highScores.resize(5); // Keep only top 5
    }
    saveHighScores();
}

void playSound(const char *filename)
{
    PlaySoundA(filename, NULL, SND_FILENAME | SND_ASYNC);
}

void stopSound()
{
    PlaySoundA(NULL, 0, 0);
}

string getRandomWord()
{
    return dictionary[rand() % dictionary.size()];
}

int countMatchingPrefix(const std::string &typed, const std::string &actual)
{
    int len = std::min(typed.size(), actual.size());
    for (int i = 0; i < len; ++i)
    {
        if (tolower(typed[i]) != tolower(actual[i]))
        {
            return i; // mismatch happened here
        }
    }
    return len; // all typed chars match
}

void drawSpeedBar(int speed, int glowTimer)
{
    int barX = 30;
    int barY = 550;
    int blockWidth = 30;
    int blockHeight = 20;
    int spacing = 5;

    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    setcolor(WHITE);
    outtextxy(barX, barY - 30, (char *)"Speed:");

    for (int i = 1; i <= 7; i++)
    {
        int x = barX + (i - 1) * (blockWidth + spacing);
        int y = barY;

        if (i <= speed)
        {
            if (glowTimer > 0 && i == speed)
                setfillstyle(SOLID_FILL, YELLOW); // glowing block
            else
                setfillstyle(SOLID_FILL, LIGHTGREEN); // normal filled
        }
        else
        {
            setfillstyle(SOLID_FILL, DARKGRAY); // unfilled
        }

        bar(x, y, x + blockWidth, y + blockHeight);
        rectangle(x, y, x + blockWidth, y + blockHeight); // outline
    }
}

void drawHUD()
{
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2); // larger font

    char buffer[100];

    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(30, 5, 250, 90); // Score area
    setcolor(WHITE);
    rectangle(30, 5, 250, 90);

    sprintf(buffer, "Score: %d", score);
    outtextxy(35, 10, buffer);

    sprintf(buffer, "Health: %d", health);
    outtextxy(35, 40, buffer);

    sprintf(buffer, "Typed: ");
    outtextxy(35, 70, buffer);

    drawSpeedBar(fallSpeed, glowTimer);

    setcolor(LIGHTCYAN);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(650, 15, (char*)(soundEnabled ? "Sound: ON" : "Sound: OFF"));

    string displayText = typed;
    if (showCursor)
    {
        displayText += "|";
    }

    char typedBuffer[100];
    strcpy(typedBuffer, displayText.c_str()); // convert string to char*
    outtextxy(125, 70, typedBuffer);          // display typed text
}

void drawWords()
{
    settextstyle(BOLD_FONT, HORIZ_DIR, 3);
    for (int i = words.size() - 1; i >= 0; --i)
    {
        if (!words[i].active)
            continue;
        words[i].y += fallSpeed;

        string full = words[i].text;
        int matchLen = countMatchingPrefix(typed, full);

        int x = words[i].x;
        int y = words[i].y;

        // Shadow
        setcolor(DARKGRAY);
        outtextxy(x + 2, y + 2, const_cast<char *>(full.c_str()));

        // Draw matching prefix in highlight (YELLOW)
        if (matchLen > 0)
        {
            string matched = full.substr(0, matchLen);
            setcolor(YELLOW);
            outtextxy(x, y, const_cast<char *>(matched.c_str()));

            // Measure the width of matched text
            int prefixWidth = textwidth(const_cast<char *>(matched.c_str()));

            // Draw the rest in the word's original color
            string rest = full.substr(matchLen);
            setcolor(words[i].color);
            outtextxy(x + prefixWidth, y, const_cast<char *>(rest.c_str()));
        }
        else
        {
            // No match at all, draw full word in original color
            setcolor(words[i].color);
            outtextxy(x, y, const_cast<char *>(full.c_str()));
        }

        // Missed word
        if (words[i].y >= screenHeight - 20)
        {
            if (soundEnabled)
                playSound("resources/wrong.wav");
            health--;
            words.erase(words.begin() + i);
        }
    }
}

void displayPowerUpMessage(const char* message) {
    strcpy(currentPowerUpMessage, message);
    messageStartTime = clock();
    showPowerUpMessage = true;
    messageX = messageStartX;  // Start off-screen
}

void checkForPowerUp() {
    if (powerUpStreak == 10) {
        powerUpStreak = 0;

        int randPower = rand() % 2;
        if(fallSpeed == 1)
            randPower = 1; // Ensure at least one power-up is available

        if (randPower == 0) {
            slowTimeActive = true;
            powerUpStartTime = clock();
            originalFallSpeed = fallSpeed;
            fallSpeed /= 2;  // slow down the falling speed
            displayPowerUpMessage("Slow Time Activated!");
        } else {
            scoreBoostActive = true;
            powerUpStartTime = clock();
            displayPowerUpMessage("Score Boost Activated!");
        }
    }
}

// Returns true if `typed` is a prefix of `w`
bool isPrefix(const string &typed, const string &w)
{
    return w.size() >= typed.size() &&
           equal(typed.begin(), typed.end(), w.begin());
}

// Returns the index of a word that exactly matches `typed`, –1 if none
int findExactMatch(const string &typed)
{
    for (int i = 0; i < (int)words.size(); ++i)
        if (words[i].active && words[i].text == typed)
            return i;
    return -1;
}

void acceptWord(int index)
{
    if (soundEnabled)
        playSound("resources/correct.wav");
    
    int baseScore = 1;
    if (scoreBoostActive)
        baseScore *= 2;

    score += baseScore;
    currentStrick++;
    powerUpStreak++; // Increment power-up streak
    checkForPowerUp();
    if (health < 5)
        health++; // reward a life
    words.erase(words.begin() + index);
    typed.clear();
}

Word createWord()
{
    Word newWord;
    newWord.text = getRandomWord();
    newWord.x = 100 + rand() % (screenWidth - 190);
    int color = allowedColors[rand() % (sizeof(allowedColors) / sizeof(allowedColors[0]))];
    newWord.color = color;
    newWord.y = 0;
    newWord.active = true;
    return newWord;
}

void spawnNewWords()
{
    int x = score >= 100 ? 25 : 50;
    if (frameCount % x == 0 || words.empty())
    { // spawn a new word every 50 frames
        if (words.size() >= 10)
            return; // limit to 10 active
        Word newWord = createWord();
        words.push_back(newWord);
    }
}

void updateSpeed()
{
    if (score % 10 == 0 && score != 0 && score != lastMilestone)
    {
        if (fallSpeed < 7) // Limit fall speed to a maximum value
            fallSpeed++;
        lastMilestone = score;
    }
    if (fallSpeed > previousSpeed) {
        glowTimer = 15; // how many frames to glow
    }
    previousSpeed = fallSpeed;
}

void resetGame()
{
    words.clear();
    typed = "";
    score = 0;
    health = 5;
    fallSpeed = 1;
    frameCount = 0;
    lastMilestone = 0;
    paused = false;
    gameOver = false;
    restartRequested = false;
    correctKeystrokes = 0;
    totalKeystrokes = 0;
    longestStrick = 0;
}

void showPauseScreen()
{
    setcolor(LIGHTBLUE);
    settextstyle(BOLD_FONT, HORIZ_DIR, 3);
    outtextxy(screenWidth / 2 - 80, screenHeight / 2, (char *)"PAUSED");
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(screenWidth / 2 - 140, screenHeight / 2 + 40, (char *)"Press 'Space' to Resume");
}

int showHighScores()
{
    setcolor(YELLOW);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);

    if(highScores.empty())
    {
        outtextxy(screenWidth / 2 - 200, 160, (char *)"No High Scores Yet!");
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        return 200; // Return a default y position
    }

    outtextxy(screenWidth / 2 - 150, 160, (char *)"High Scores:");

    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    int y = 190;
    for (auto &entry : highScores)
    {
        char line[100];
        sprintf(line, "%-10s  %3d", entry.name.c_str(), entry.score);
        outtextxy(screenWidth / 2 - 150, y, line);
        y += 30;
    }
    return y;
}

void showGameOverScreen(void *gameOverBg)
{
    // Load and show background image
    putimage(0, 0, gameOverBg, COPY_PUT);

    setcolor(RED);
    settextstyle(BOLD_FONT, HORIZ_DIR, 5);

    int y = showHighScores(); // displays top scores

    // Score
    char buffer[100];
    sprintf(buffer, "Your Score: %d", score);
    outtextxy(screenWidth / 2 - 150, y + 20, buffer);

    // Accuracy
    float accuracy = 0;
    if (totalKeystrokes > 0)
        accuracy = (correctKeystrokes * 100.0f) / totalKeystrokes;

    if (accuracy > 90)
        setcolor(GREEN);
    else if (accuracy > 70)
        setcolor(YELLOW);
    else
        setcolor(RED);

    sprintf(buffer, "Accuracy: %.2f%%", accuracy);
    outtextxy(screenWidth / 2 - 150, y + 60, buffer);

    // Longest Streak
    setcolor(YELLOW);
    sprintf(buffer, "Longest Streak: %d words", longestStrick);
    outtextxy(screenWidth / 2 - 150, y + 100, buffer);

    // Restart message
    outtextxy(screenWidth / 2 - 150, y + 140, (char *)"Press 'R' to Restart");
    outtextxy(screenWidth / 2 - 150, y + 180, (char *)"Press 'Esc' to Quit");
}

void handleTyping()
{
    while (kbhit()) // read *all* queued keys
    {
        char ch = getch();
        // global hot‑keys first
        if (ch == ' ')
        {
            paused = !paused;
            continue;
        }
        if (ch == 27)
        {
            exit(0);
        } // ESC
        if ((ch == 'r' || ch == 'R') && gameOver)
        {
            restartRequested = true;
            continue;
        }

        if (paused || gameOver)
            continue; // ignore other keys

        /* --- text input --- */
        if (ch == '\b')
        { // back‑space
            if (!typed.empty())
                typed.pop_back();
        }
        else if (isalpha(ch))
        {
            totalKeystrokes++; // count every keystroke
            typed += ch;
        }

        /* --- real‑time checking --- */

        // 3‑a) exact word finished?
        int idx = findExactMatch(typed);
        if (idx != -1)
        {                                        // typed matches a word
            correctKeystrokes += typed.length(); // count correct keystrokes
            acceptWord(idx);
            continue;
        }

        // 3‑b) still a prefix of at least one word?
        bool stillValid = false;
        for (auto &w : words)
            if (w.active && isPrefix(typed, w.text))
            {
                stillValid = true;
                break;
            }

        if (!stillValid && !typed.empty())
        { // typed can never match
            for (auto &w : words)
            {
                if (w.active)
                {
                    int matchLen = countMatchingPrefix(typed, w.text);
                    correctKeystrokes += matchLen;
                    break; // only apply to first matching word
                }
            }

            if (soundEnabled)
                playSound("resources/wrong.wav");
            longestStrick = currentStrick > longestStrick ? currentStrick : longestStrick;
            currentStrick = 0; // reset current streak
            powerUpStreak = 0;
            health--;
            typed.clear();
        }
    }
}

void playOpeningTransition()
{
    // Prepare background buffer
    void *bgBuffer = malloc(imagesize(0, 0, screenWidth, screenHeight));
    readimagefile("resources/background.bmp", 0, 0, screenWidth, screenHeight);
    getimage(0, 0, screenWidth, screenHeight, bgBuffer);

    // Start with black screen
    setbkcolor(BLACK);
    cleardevice();

    const int barHeight = 20;

    for (int i = screenHeight; i >= 0; i -= barHeight)
    {
        setactivepage(currentPage);
        setvisualpage(1 - currentPage);

        putimage(0, 0, bgBuffer, COPY_PUT); // draw full background

        // draw black bars from bottom upwards
        setfillstyle(SOLID_FILL, BLACK);
        bar(0, i, screenWidth, screenHeight);

        delay(20);
        currentPage = 1 - currentPage;
    }

    // Show final full frame
    setactivepage(0);
    setvisualpage(0);
    putimage(0, 0, bgBuffer, COPY_PUT);

    free(bgBuffer);
}

void runGame()
{
    cleardevice();

    // Load once — this buffer is reused and freed after all sessions
    void *gameOverBg = malloc(imagesize(0, 0, screenWidth, screenHeight));
    readimagefile("resources/game_over.bmp", 0, 0, screenWidth, screenHeight);
    getimage(0, 0, screenWidth, screenHeight, gameOverBg);

    const int totalFrames = 30;
    std::vector<void *> bgFrames;

    for (int i = 0; i < totalFrames; ++i)
    {
        char filename[100];
        sprintf(filename, "resources/frames/frame_%03d.bmp", i);
        readimagefile(filename, 0, 0, screenWidth, screenHeight);
        void *frame = malloc(imagesize(0, 0, screenWidth, screenHeight));
        getimage(0, 0, screenWidth, screenHeight, frame);
        bgFrames.push_back(frame);
    }

    srand(time(0));
    setbkcolor(BLACK);
    setcolor(WHITE);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);

    do
    {
        resetGame();
        playOpeningTransition();

        while (!restartRequested)
        {
            setactivepage(currentPage);
            setvisualpage(1 - currentPage);
            int currentFrame = (frameCount / 2) % totalFrames;
            putimage(0, 0, bgFrames[currentFrame], COPY_PUT);

            handleTyping();

            // Toggle sound with F1
            static bool f1Pressed = false;
            if (GetAsyncKeyState(VK_F1) & 0x8000)
            {
                if (!f1Pressed)
                {
                    soundEnabled = !soundEnabled;
                    if (!soundEnabled)
                        PlaySound(NULL, 0, 0);
                    f1Pressed = true;
                }
            }
            else
            {
                f1Pressed = false;
            }

            // Power-up duration check
            if (slowTimeActive || scoreBoostActive)
            {
                if ((clock() - powerUpStartTime) > (slowTimeActive ? slowTimeDuration : scoreBoostDuration))
                {
                    if (slowTimeActive)
                    {
                        fallSpeed = originalFallSpeed;
                        slowTimeActive = false;
                    }
                    if (scoreBoostActive)
                        scoreBoostActive = false;
                }
            }

            // Game logic
            if (!paused && !gameOver)
            {
                drawHUD();
                drawWords();
                spawnNewWords();
                updateSpeed();
                frameCount++;
                if (frameCount % 10 == 0)
                    showCursor = !showCursor;
            }
            else
            {
                drawHUD();

                if (paused)
                {
                    showPauseScreen();
                }
                else if (gameOver)
                {
                    showGameOverScreen(gameOverBg);
                }
            }

            // Power-up message slide-in
            if (showPowerUpMessage)
            {
                unsigned long elapsed = clock() - messageStartTime;

                if (elapsed < messageDuration)
                {
                    if (elapsed < 500)
                        messageX = messageStartX + (messageTargetX - messageStartX) * (elapsed / 500.0) * (elapsed / 500.0);
                    else if (elapsed < 2500)
                        messageX = messageTargetX;
                    else
                        messageX = messageEndX;

                    setcolor(YELLOW);
                    settextstyle(BOLD_FONT, HORIZ_DIR, 2);
                    outtextxy(messageX, 50, currentPowerUpMessage);
                }
                else
                {
                    showPowerUpMessage = false;
                }
            }

            // Game Over check
            if (health <= 0 && !gameOver)
            {
                if (soundEnabled)
                    playSound("resources/game_over.wav");

                playOpeningTransition();
                gameOver = true;

                loadHighScores();

                if (highScores.size() < 5 || score > highScores.back().score)
                {
                    char name[20] = "";
                    int i = 0;
                    cleardevice();

                    while (true)
                    {
                        setactivepage(currentPage);
                        setvisualpage(1 - currentPage);
                        cleardevice();
                        setcolor(WHITE);
                        outtextxy(screenWidth / 2 - 150, screenHeight / 2 - 20, (char *)"New High Score!");
                        outtextxy(screenWidth / 2 - 150, screenHeight / 2 + 20, (char *)"Enter Name: ");
                        outtextxy(screenWidth / 2, screenHeight / 2 + 20, name);

                        if (kbhit())
                        {
                            char ch = getch();
                            if (ch == '\r') break;
                            if (ch == '\b' && i > 0)
                            {
                                i--;
                                name[i] = '\0';
                            }
                            else if (isalpha(ch) && i < 19)
                            {
                                name[i++] = ch;
                                name[i] = '\0';
                            }
                        }

                        delay(30);
                        currentPage = 1 - currentPage;
                    }

                    insertHighScore(name, score);
                }
            }

            delay(50);
            currentPage = 1 - currentPage;
        }

    } while (restartRequested);

    // 🔴 Free allocated images here to prevent memory leaks
    for (auto frame : bgFrames)
        free(frame);

    free(gameOverBg);

    closegraph();
}


void userInput()
{
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(50, 100, (char *)"Enter your custom sentence (press Enter to finish):");

    char input[1000];
    int index = 0;
    input[0] = '\0';

    // Layout
    int startX = 100, startY = 150;
    int maxWidth = 650;
    int lineHeight = textheight((char *)"A") + 5;
    int maxLines = 10;

    int lineCount = 1;
    int prevIndex = -1; // To track changes
    bool showCursor = true;
    int cursorBlinkTimer = 0;

    int lastLineX = startX;
    int lastLineY = startY;

    while (true)
    {
        // Handle input
        if (kbhit())
        {
            char ch = getch();
            if (ch == 13)
            {
                input[index] = '\0';
                break;
            }
            else if (ch == 8 && index > 0)
            {
                index--;
                input[index] = '\0';
            }
            else if (index < 999 && ch >= 32 && ch <= 126)
            {
                input[index++] = ch;
                input[index] = '\0';
            }
        }

        // Check if input changed
        if (index != prevIndex)
        {
            prevIndex = index;

            // Clear full region
            setfillstyle(SOLID_FILL, BLACK);
            bar(startX - 12, startY - 12, startX + maxWidth + 12, startY + maxLines * lineHeight + 20);

            // Multiline render
            int x = startX, y = startY;
            lineCount = 1;
            char line[1000] = "";
            int lineLength = 0;

            for (int i = 0; i <= index; ++i)
            {
                line[lineLength] = input[i];
                line[lineLength + 1] = '\0';
                int textWidth = textwidth(line);

                if (textWidth > maxWidth || input[i] == '\0')
                {
                    if (textWidth > maxWidth)
                    {
                        line[lineLength] = '\0';
                        outtextxy(x, y, line);
                        y += lineHeight;
                        line[0] = input[i];
                        line[1] = '\0';
                        lineLength = 1;
                        lineCount++;
                    }
                    else
                    {
                        outtextxy(x, y, line);
                        lastLineX = x + textwidth(line);
                        lastLineY = y;
                    }

                    if (input[i] == '\0') break;

                    if (textWidth <= maxWidth)
                    {
                        y += lineHeight;
                        line[0] = '\0';
                        lineLength = 0;
                        lineCount++;
                    }
                }
                else
                {
                    lineLength++;
                    lastLineX = x + textwidth(line);
                    lastLineY = y;
                }
            }

            // Draw rectangle
            int rectTop = startY - 10;
            int rectBottom = startY + lineCount * lineHeight + 5;
            setcolor(WHITE);
            rectangle(startX - 10, rectTop, startX + maxWidth + 10, rectBottom);
        }

        // Blinking cursor logic
        cursorBlinkTimer++;
        if (cursorBlinkTimer >= 10)
        {
            showCursor = !showCursor;
            cursorBlinkTimer = 0;

            // Draw or erase cursor without touching text
            setfillstyle(SOLID_FILL, BLACK);
            bar(lastLineX, lastLineY, lastLineX + 10, lastLineY + lineHeight);

            if (showCursor)
            {
                setcolor(LIGHTCYAN);
                outtextxy(lastLineX, lastLineY, (char *)"|");
                setcolor(WHITE);
            }
        }

        delay(50);
    }

    // Save to file
    FILE *fp = fopen("file/custom.txt", "w");
    if (fp != NULL)
    {
        char *token = strtok(input, " ");
        while (token != NULL)
        {
            fprintf(fp, "%s\n", token);
            token = strtok(NULL, " ");
        }
        fclose(fp);
    }

    // Clear cursor
    setfillstyle(SOLID_FILL, BLACK);
    bar(lastLineX, lastLineY, lastLineX + 10, lastLineY + lineHeight);

    // Final message
    int messageY = startY + lineCount * lineHeight + 30;
    setcolor(WHITE);
    outtextxy(200, messageY, (char *)"Custom words loaded!");
    delay(1000);
}

void drawSelectionMenu(int hoverIndex = -1)
{
    setcolor(GREEN);
    settextstyle(BOLD_FONT, HORIZ_DIR, 4);
    outtextxy(220, 100, (char *)"TYPING SPEED GAME");

    int x = 250, width = 270, height = 40;
    int y1 = 180, gap = 50;

    const char *labels[] = {
        "1. Random Words",
        "2. Custom Words",
        "3. Back",
        "4. Exit"};

    for (int i = 0; i < 4; ++i)
    {
        int y = y1 + i * gap;

        // Highlight hovered option
        if (i == hoverIndex)
            setcolor(RED);
        else
            setcolor(WHITE);

        setfillstyle(SOLID_FILL, BLACK);
        bar(x, y, x + width, y + height);

        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        outtextxy(x + 10, y + 10, (char *)labels[i]);
    }
}

void showSelectionMenu()
{
    SelectionState state = MAINMENU;
    int hover = -1;
    bool redraw = true; 

    void* bgBuffer = malloc(imagesize(0, 0, 800, 600));
    readimagefile("resources/user_menu_background.bmp", 0, 0, 800, 600);
    getimage(0, 0, 800, 600, bgBuffer);

    while (state != QUIT)
    {
        if (redraw)
        {
            putimage(0, 0, bgBuffer, COPY_PUT);
            drawSelectionMenu(hover);
            redraw = false;
        }

        // Mouse movement detection
        if (ismouseclick(WM_MOUSEMOVE))
        {
            int mx, my;
            getmouseclick(WM_MOUSEMOVE, mx, my);

            int newHover = -1;
            int x = 250, width = 270, height = 40, y1 = 180, gap = 50;
            for (int i = 0; i < 4; ++i)
            {
                int y = y1 + i * gap;
                if (mx >= x && mx <= x + width && my >= y && my <= y + height)
                {
                    newHover = i;
                    break;
                }
            }

            if (newHover != hover)
            {
                hover = newHover;
                redraw = true;
            }
        }

        // Redraw on hover change
        if (redraw)
        {
            drawSelectionMenu(hover);
        }

        // Mouse click
        if (ismouseclick(WM_LBUTTONDOWN))
        {
            int mx, my;
            getmouseclick(WM_LBUTTONDOWN, mx, my);

            int x = 250, width = 270, height = 40, y1 = 180, gap = 50;
            for (int i = 0; i < 4; ++i)
            {
                int y = y1 + i * gap;
                if (mx >= x && mx <= x + width && my >= y && my <= y + height)
                {
                    state = static_cast<SelectionState>(i + 1);
                    break;
                }
            }
        }

        // Keyboard fallback
        if (kbhit())
        {
            char ch = getch();
            if (ch == '1')
                state = RANDOM;
            else if (ch == '2')
                state = CUSTOM;
            else if (ch == '3')
                state = BACK;
            else if (ch == '4' || ch == 27)
                state = QUIT;
        }

        // Handle selected state
        if (state == RANDOM)
        {
            cleardevice();
            loadWordsFromFile("file/words.txt");
            runGame();
            state = MAINMENU;
        }
        else if (state == CUSTOM)
        {
            cleardevice();
            userInput();
            loadWordsFromFile("file/custom.txt");
            runGame();
            state = MAINMENU;
        }
        else if (state == BACK)
        {
            free(bgBuffer);
            return;
        }
        else if (state == QUIT)
        {
            exit(0);
        }

        delay(30);
    }
}
