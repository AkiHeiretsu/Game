#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <set>
#include <SDL_ttf.h>
#include <fstream>
#include <sstream>
#include <SDL_image.h>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int MAX_HIGH_SCORES = 5;

struct HighScore {
    int streak;
};

vector<HighScore> loadHighScores() {
    vector<HighScore> highScores;
    ifstream file("highscores.txt");
    if (file.is_open()) {
        string line;
        while (getline(file, line) && highScores.size() < 1) {  
            istringstream iss(line);
            HighScore hs;
            if (iss >> hs.streak) {
                highScores.push_back(hs);
            }
        }
    }
    return highScores;
}

void saveHighScores(const vector<HighScore>& highScores) {
    ofstream file("highscores.txt");
    if (file.is_open()) {
        for (const auto& hs : highScores) {
            file << hs.streak << "\n";
        }
    }
}

void updateHighScores(int streak) {
    vector<HighScore> highScores = loadHighScores();
    HighScore newScore{streak};
    
    if (highScores.empty() || streak > highScores[0].streak) {
        highScores.clear();
        highScores.push_back(newScore);
    }
    
    saveHighScores(highScores);
}

void SDL_RenderDrawEllipse(SDL_Renderer *renderer, int x0, int y0, int rx, int ry);

void drawHangman(SDL_Renderer *renderer, int wrongGuesses)
{
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawLine(renderer, 100, 500, 300, 500); 
    SDL_RenderDrawLine(renderer, 200, 500, 200, 100); 
    SDL_RenderDrawLine(renderer, 200, 100, 350, 100); 
    SDL_RenderDrawLine(renderer, 350, 100, 350, 150); 

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    if (wrongGuesses > 0)
    {
        SDL_Rect head = {325, 150, 50, 50};
        SDL_RenderDrawEllipse(renderer, head.x + 25, head.y + 25, 25, 25);
    }
    if (wrongGuesses > 1)
        SDL_RenderDrawLine(renderer, 350, 200, 350, 320);
    if (wrongGuesses > 2)
        SDL_RenderDrawLine(renderer, 350, 220, 310, 270);
    if (wrongGuesses > 3)
        SDL_RenderDrawLine(renderer, 350, 220, 390, 270);
    if (wrongGuesses > 4)
        SDL_RenderDrawLine(renderer, 350, 320, 310, 380);
    if (wrongGuesses > 5)
        SDL_RenderDrawLine(renderer, 350, 320, 390, 380);
}

void SDL_RenderDrawEllipse(SDL_Renderer *renderer, int x0, int y0, int rx, int ry)
{
    int x, y;
    int rx2 = rx * rx;
    int ry2 = ry * ry;
    int tworx2 = 2 * rx2;
    int twory2 = 2 * ry2;
    int px = 0;
    int py = tworx2 * ry;

    int p = round(ry2 - (rx2 * ry) + (0.25 * rx2));
    x = 0;
    y = ry;
    while (px < py)
    {
        SDL_RenderDrawPoint(renderer, x0 + x, y0 + y);
        SDL_RenderDrawPoint(renderer, x0 - x, y0 + y);
        SDL_RenderDrawPoint(renderer, x0 + x, y0 - y);
        SDL_RenderDrawPoint(renderer, x0 - x, y0 - y);
        x++;
        px += twory2;
        if (p < 0)
        {
            p += ry2 + px;
        }
        else
        {
            y--;
            py -= tworx2;
            p += ry2 + px - py;
        }
    }

    p = round(ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2);
    while (y >= 0)
    {
        SDL_RenderDrawPoint(renderer, x0 + x, y0 + y);
        SDL_RenderDrawPoint(renderer, x0 - x, y0 + y);
        SDL_RenderDrawPoint(renderer, x0 + x, y0 - y);
        SDL_RenderDrawPoint(renderer, x0 - x, y0 - y);
        y--;
        py -= tworx2;
        if (p > 0)
        {
            p += rx2 - py;
        }
        else
        {
            x++;
            px += twory2;
            p += rx2 - py + px;
        }
    }
}
int main(int argc, char *argv[])
{
    srand(static_cast<unsigned int>(time(nullptr)));
    vector<string> wordList = {"computer", "hangman", "sdl", "window", "programming"};
    string word = wordList[rand() % wordList.size()];
    set<char> guessed;
    int wrongGuesses = 0;
    const int maxWrong = 6;
    int currentStreak = 0;
    int totalScore = 0;
    int mouseX = 0, mouseY = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() < 0)
    {
        cerr << "TTF could not initialize! TTF_Error: " << TTF_GetError() << endl;
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    char* basePath = SDL_GetBasePath();
    if (!basePath) {
        cerr << "Could not get base path! SDL_Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    string musicPath = string(basePath) + "../assets/background.mp3";
    cout << "Current working directory: " << basePath << endl;
    cout << "Music path: " << musicPath << endl;
    Mix_Music *backgroundMusic = Mix_LoadMUS(musicPath.c_str());
    if (!backgroundMusic)
    {
        cerr << "Failed to load background music! Mix_Error: " << Mix_GetError() << endl;
        SDL_free(basePath);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("SDL2 Hangman", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    string fontPath = string(basePath) + "../assets/font.ttf";
    cout << "Font path: " << fontPath << endl;
    TTF_Font *font = TTF_OpenFont(fontPath.c_str(), 24);
    TTF_Font *largeFont = TTF_OpenFont(fontPath.c_str(), 48);
    TTF_Font *hugeFont = TTF_OpenFont(fontPath.c_str(), 64);
    TTF_Font *mediumFont = TTF_OpenFont(fontPath.c_str(), 36);

    if (!font || !largeFont || !hugeFont || !mediumFont)
    {
        cerr << "Font could not be loaded! TTF_Error: " << TTF_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_free(basePath);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_free(basePath);

    // Load background image
    SDL_Surface* backgroundSurface = IMG_Load("../assets/background.jpeg");
    if (!backgroundSurface)
    {
        cerr << "Failed to load background image! SDL_image Error: " << IMG_GetError() << endl;
        return 1;
    }
    SDL_Texture* backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);
    if (!backgroundTexture)
    {
        cerr << "Failed to create texture from background image! SDL Error: " << SDL_GetError() << endl;
        return 1;
    }

    bool quit = false;
    while (!quit)
    {
        bool startScreen = true;
        vector<HighScore> highScores = loadHighScores();
        currentStreak = 0;
        totalScore = 0;

        while (startScreen && !quit)
        {
    SDL_Event e;
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    startScreen = false;
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    Mix_CloseAudio();
                    TTF_Quit();
                    SDL_Quit();
                    return 0;
                }
                else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    SDL_GetMouseState(&mouseX, &mouseY);

                    SDL_Rect buttonRect = {
                        (WINDOW_WIDTH - 300) / 2,
                        WINDOW_HEIGHT - 150,
                        300,
                        100
                    };
                    if (mouseX >= buttonRect.x && mouseX <= buttonRect.x + buttonRect.w &&
                        mouseY >= buttonRect.y && mouseY <= buttonRect.y + buttonRect.h)
                    {
                        startScreen = false;
                        if (Mix_PlayMusic(backgroundMusic, -1) == -1)
                        {
                            cerr << "Failed to play background music! Mix_Error: " << Mix_GetError() << endl;
                        }
                    }
                }
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255);
            SDL_RenderClear(renderer);

            // Draw background
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

            SDL_GetMouseState(&mouseX, &mouseY);

            SDL_Color titleColor = {0, 0, 0, 255};
            string titleText = "HANGMAN";
            SDL_Surface *titleSurface = TTF_RenderText_Solid(hugeFont, titleText.c_str(), titleColor);
            SDL_Texture *titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
            SDL_Rect titleRect = {
                (WINDOW_WIDTH - titleSurface->w) / 2,
                50,
                titleSurface->w,
                titleSurface->h
            };
            SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
            SDL_FreeSurface(titleSurface);
            SDL_DestroyTexture(titleTexture);

            if (!highScores.empty()) {
                string highScoreText = "HIGHEST STREAK: " + to_string(highScores[0].streak);
                SDL_Surface *highScoreSurface = TTF_RenderText_Solid(mediumFont, highScoreText.c_str(), titleColor);
                SDL_Texture *highScoreTexture = SDL_CreateTextureFromSurface(renderer, highScoreSurface);
                SDL_Rect highScoreRect = {
                    (WINDOW_WIDTH - highScoreSurface->w) / 2,
                    250,
                    highScoreSurface->w,
                    highScoreSurface->h
                };
                SDL_RenderCopy(renderer, highScoreTexture, NULL, &highScoreRect);
                SDL_FreeSurface(highScoreSurface);
                SDL_DestroyTexture(highScoreTexture);
            }

            SDL_GetMouseState(&mouseX, &mouseY);
            SDL_Rect buttonRect = {
                (WINDOW_WIDTH - 300) / 2,
                WINDOW_HEIGHT - 150,
                300,
                100
            };
            bool isHovering = (mouseX >= buttonRect.x && mouseX <= buttonRect.x + buttonRect.w &&
                             mouseY >= buttonRect.y && mouseY <= buttonRect.y + buttonRect.h);

            SDL_SetRenderDrawColor(renderer, isHovering ? 0 : 0, isHovering ? 200 : 150, 0, 255);
            SDL_RenderFillRect(renderer, &buttonRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &buttonRect);

            string startText = "START GAME";
            SDL_Surface *startSurface = TTF_RenderText_Solid(largeFont, startText.c_str(), titleColor);
            SDL_Texture *startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
            SDL_Rect startRect = {
                (WINDOW_WIDTH - startSurface->w) / 2,
                WINDOW_HEIGHT - 130,
                startSurface->w,
                startSurface->h
            };
            SDL_RenderCopy(renderer, startTexture, NULL, &startRect);
            SDL_FreeSurface(startSurface);
            SDL_DestroyTexture(startTexture);

            SDL_Color instructColor = {180, 180, 180, 255};
            string instructText = "Click START GAME to begin";
            SDL_Surface *instructSurface = TTF_RenderText_Solid(font, instructText.c_str(), instructColor);
            SDL_Texture *instructTexture = SDL_CreateTextureFromSurface(renderer, instructSurface);
            SDL_Rect instructRect = {
                (WINDOW_WIDTH - instructSurface->w) / 2,
                WINDOW_HEIGHT - 50,
                instructSurface->w,
                instructSurface->h
            };
            SDL_RenderCopy(renderer, instructTexture, NULL, &instructRect);
            SDL_FreeSurface(instructSurface);
            SDL_DestroyTexture(instructTexture);

            string wrongGuessText = "Wrong guesses allowed: " + to_string(maxWrong);
            SDL_Surface *wrongSurface = TTF_RenderText_Solid(font, wrongGuessText.c_str(), titleColor);
            SDL_Texture *wrongTexture = SDL_CreateTextureFromSurface(renderer, wrongSurface);
            SDL_Rect wrongRect = {
                20,
                20,
                wrongSurface->w,
                wrongSurface->h
            };
            SDL_RenderCopy(renderer, wrongTexture, NULL, &wrongRect);
            SDL_FreeSurface(wrongSurface);
            SDL_DestroyTexture(wrongTexture);

            SDL_RenderPresent(renderer);
            SDL_Delay(50);
        }

        if (quit) break;

        word = wordList[rand() % wordList.size()];
        guessed.clear();
        wrongGuesses = 0;
        bool gameOver = false;

        while (!quit && !gameOver)
    {
            string displayWord;
            bool wordComplete = true;
        for (char c : word)
        {
            if (guessed.count(c))
            {
                displayWord += c;
            }
            else
            {
                displayWord += '_';
                    wordComplete = false;
            }
            displayWord += ' ';
        }
            cout << "\nWord: " << displayWord << endl;
            cout << "Guessed letters: ";
        for (char c : guessed)
                cout << c << ' ';
            cout << "\nWrong guesses: " << wrongGuesses << "/" << maxWrong << endl;

            if (wordComplete)
            {
                cout << "You win! The word was: " << word << endl;
                currentStreak++;
                int wordScore = (word.length() * 10) - (wrongGuesses * 5);
                totalScore += wordScore;
                SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
                SDL_RenderClear(renderer);
                SDL_Color textColor = {255, 255, 255, 255};
                string streakText = "Correct! Streak: " + to_string(currentStreak);
                SDL_Surface *streakSurface = TTF_RenderText_Solid(hugeFont, streakText.c_str(), textColor);
                SDL_Texture *streakTexture = SDL_CreateTextureFromSurface(renderer, streakSurface);
                SDL_Rect streakRect = {
                    (WINDOW_WIDTH - streakSurface->w) / 2,
                    WINDOW_HEIGHT / 2 - 50,
                    streakSurface->w,
                    streakSurface->h
                };
                SDL_RenderCopy(renderer, streakTexture, NULL, &streakRect);
                SDL_FreeSurface(streakSurface);
                SDL_DestroyTexture(streakTexture);

                string scoreText = "Total Score: " + to_string(totalScore);
                SDL_Surface *scoreSurface = TTF_RenderText_Solid(mediumFont, scoreText.c_str(), textColor);
                SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
                SDL_Rect scoreRect = {
                    (WINDOW_WIDTH - scoreSurface->w) / 2,
                    WINDOW_HEIGHT / 2 + 50,
                    scoreSurface->w,
                    scoreSurface->h
                };
                SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
                SDL_FreeSurface(scoreSurface);
                SDL_DestroyTexture(scoreTexture);

                SDL_RenderPresent(renderer);
                SDL_Delay(1500);

                string prevWord = word;
                do {
                    word = wordList[rand() % wordList.size()];
                } while (word == prevWord);
                guessed.clear();
                wrongGuesses = 0;
                continue;
            }

        if (wrongGuesses >= maxWrong)
        {
                cout << "Game Over! The word was: " << word << endl;
                gameOver = true;
                updateHighScores(currentStreak);           
                SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255);
                SDL_RenderClear(renderer);
                SDL_GetMouseState(&mouseX, &mouseY);
                SDL_Color gameOverColor = {255, 255, 255, 255};
                string gameOverText = "Game Over!";
                SDL_Surface *gameOverSurface = TTF_RenderText_Solid(hugeFont, gameOverText.c_str(), gameOverColor);
                SDL_Texture *gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
                SDL_Rect gameOverRect = {
                    (WINDOW_WIDTH - gameOverSurface->w) / 2,
                    WINDOW_HEIGHT / 2 - 150,
                    gameOverSurface->w,
                    gameOverSurface->h
                };
                SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
                SDL_FreeSurface(gameOverSurface);
                SDL_DestroyTexture(gameOverTexture);

                // Display final stats
                string statsText = "Words Guessed: " + to_string(currentStreak) + 
                                      " | Final Score: " + to_string(totalScore);
                SDL_Surface *statsSurface = TTF_RenderText_Solid(mediumFont, statsText.c_str(), gameOverColor);
                SDL_Texture *statsTexture = SDL_CreateTextureFromSurface(renderer, statsSurface);
                SDL_Rect statsRect = {
                    (WINDOW_WIDTH - statsSurface->w) / 2,
                    WINDOW_HEIGHT / 2 - 50,
                    statsSurface->w,
                    statsSurface->h
                };
                SDL_RenderCopy(renderer, statsTexture, NULL, &statsRect);
                SDL_FreeSurface(statsSurface);
                SDL_DestroyTexture(statsTexture);
                string wordText = "The word was: " + word;
                SDL_Surface *wordSurface = TTF_RenderText_Solid(mediumFont, wordText.c_str(), gameOverColor);
                SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                SDL_Rect wordRect = {
                    (WINDOW_WIDTH - wordSurface->w) / 2,
                    WINDOW_HEIGHT / 2 + 50,
                    wordSurface->w,
                    wordSurface->h
                };
                SDL_RenderCopy(renderer, wordTexture, NULL, &wordRect);
                SDL_FreeSurface(wordSurface);
                SDL_DestroyTexture(wordTexture);

                SDL_Rect mainMenuButton = {
                    (WINDOW_WIDTH - 300) / 2,
                    WINDOW_HEIGHT - 200,
                    300,
                    80
                };

                SDL_Rect playAgainButton = {
                    (WINDOW_WIDTH - 300) / 2,
                    WINDOW_HEIGHT - 100,
                    300,
                    80
                };

                SDL_GetMouseState(&mouseX, &mouseY);
                bool isHoveringMainMenu = (mouseX >= mainMenuButton.x && mouseX <= mainMenuButton.x + mainMenuButton.w &&
                                         mouseY >= mainMenuButton.y && mouseY <= mainMenuButton.y + mainMenuButton.h);
                bool isHoveringPlayAgain = (mouseX >= playAgainButton.x && mouseX <= playAgainButton.x + playAgainButton.w &&
                                          mouseY >= playAgainButton.y && mouseY <= playAgainButton.y + playAgainButton.h);

                SDL_SetRenderDrawColor(renderer, isHoveringMainMenu ? 0 : 0, isHoveringMainMenu ? 200 : 150, 0, 255);
                SDL_RenderFillRect(renderer, &mainMenuButton);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &mainMenuButton);

                SDL_SetRenderDrawColor(renderer, isHoveringPlayAgain ? 0 : 0, isHoveringPlayAgain ? 200 : 150, 0, 255);
                SDL_RenderFillRect(renderer, &playAgainButton);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &playAgainButton);

                string mainMenuText = "MAIN MENU";
                SDL_Surface *mainMenuSurface = TTF_RenderText_Solid(mediumFont, mainMenuText.c_str(), gameOverColor);
                SDL_Texture *mainMenuTexture = SDL_CreateTextureFromSurface(renderer, mainMenuSurface);
                SDL_Rect mainMenuTextRect = {
                    (WINDOW_WIDTH - mainMenuSurface->w) / 2,
                    WINDOW_HEIGHT - 180,
                    mainMenuSurface->w,
                    mainMenuSurface->h
                };
                SDL_RenderCopy(renderer, mainMenuTexture, NULL, &mainMenuTextRect);
                SDL_FreeSurface(mainMenuSurface);
                SDL_DestroyTexture(mainMenuTexture);

                string playAgainText = "PLAY AGAIN";
                SDL_Surface *playAgainSurface = TTF_RenderText_Solid(mediumFont, playAgainText.c_str(), gameOverColor);
                SDL_Texture *playAgainTexture = SDL_CreateTextureFromSurface(renderer, playAgainSurface);
                SDL_Rect playAgainTextRect = {
                    (WINDOW_WIDTH - playAgainSurface->w) / 2,
                    WINDOW_HEIGHT - 80,
                    playAgainSurface->w,
                    playAgainSurface->h
                };
                SDL_RenderCopy(renderer, playAgainTexture, NULL, &playAgainTextRect);
                SDL_FreeSurface(playAgainSurface);
                SDL_DestroyTexture(playAgainTexture);

                SDL_RenderPresent(renderer);

                bool deciding = true;
                while (deciding && !quit)
                {
                    SDL_Event e;
                    while (SDL_PollEvent(&e) != 0)
                    {
                        if (e.type == SDL_QUIT)
                        {
                            quit = true;
                            deciding = false;
                        }
                        else if (e.type == SDL_MOUSEBUTTONDOWN)
                        {
                            SDL_GetMouseState(&mouseX, &mouseY);
                            
                            if (mouseX >= mainMenuButton.x && mouseX <= mainMenuButton.x + mainMenuButton.w &&
                                mouseY >= mainMenuButton.y && mouseY <= mainMenuButton.y + mainMenuButton.h)
                            {
                                startScreen = true;
                                deciding = false;
                            }
                            else if (mouseX >= playAgainButton.x && mouseX <= playAgainButton.x + playAgainButton.w &&
                                     mouseY >= playAgainButton.y && mouseY <= playAgainButton.y + playAgainButton.h)
                            {
                                word = wordList[rand() % wordList.size()];
                                guessed.clear();
                                wrongGuesses = 0;
                                currentStreak = 0;
                                totalScore = 0;
                                deciding = false;
                            }
                        }
                    }
                    SDL_Delay(50);
                }
                continue;
        }

            SDL_Event e;
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
                else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    SDL_GetMouseState(&mouseX, &mouseY);
            }
            else if (e.type == SDL_KEYDOWN)
            {
                char guess = 0;
                if (e.key.keysym.sym >= SDLK_a && e.key.keysym.sym <= SDLK_z)
                {
                    guess = static_cast<char>(e.key.keysym.sym);
                }
                if (guess && !guessed.count(guess))
                {
                    guessed.insert(guess);
                    if (word.find(guess) == string::npos)
                    {
                        wrongGuesses++;
                    }
                }
            }
        }

            // Draw game screen
            SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
        SDL_RenderClear(renderer);
        drawHangman(renderer, wrongGuesses);

            // Get mouse position for hover effects
            SDL_GetMouseState(&mouseX, &mouseY);

            // Render word with larger font
            SDL_Color wordColor = {0, 0, 0, 255};
            string displayText = displayWord;
            SDL_Surface *textSurface = TTF_RenderText_Solid(largeFont, displayText.c_str(), wordColor);
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = {
                (WINDOW_WIDTH - textSurface->w) / 2,  // Center horizontally
                WINDOW_HEIGHT - 150,                   // Near bottom of screen
                textSurface->w,
                textSurface->h
            };
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);

            // Render guessed letters with blue color
            SDL_Color guessedColor = {100, 180, 255, 255};
            string guessedLetters = "Letters tried: ";
            for (char c : guessed) {
                guessedLetters += c;
                guessedLetters += ' ';
            }
            textSurface = TTF_RenderText_Solid(font, guessedLetters.c_str(), guessedColor);
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect guessRect = {50, 50, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, textTexture, NULL, &guessRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);

            SDL_Color wrongColor;
            if (wrongGuesses <= 2) {
                wrongColor = {100, 255, 100, 255}; 
            } else if (wrongGuesses <= 4) {
                wrongColor = {255, 255, 100, 255};
            } else {
                wrongColor = {255, 100, 100, 255};
            }
            string wrongGuessText = "Wrong guesses: " + to_string(wrongGuesses) + "/" + to_string(maxWrong);
            textSurface = TTF_RenderText_Solid(font, wrongGuessText.c_str(), wrongColor);
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect wrongRect = {
                20,
                20,
                textSurface->w,
                textSurface->h
            };
            SDL_RenderCopy(renderer, textTexture, NULL, &wrongRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);

        SDL_RenderPresent(renderer);

        SDL_Delay(50);
    }
    }

    Mix_FreeMusic(backgroundMusic);
    Mix_CloseAudio();
    SDL_DestroyTexture(backgroundTexture);
    TTF_CloseFont(font);
    TTF_CloseFont(largeFont);
    TTF_CloseFont(hugeFont);
    TTF_CloseFont(mediumFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
