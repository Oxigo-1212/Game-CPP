#include "include/MainMenu.h"
#include "include/Constants.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>

MainMenu::MainMenu(SDL_Renderer* renderer) : 
    renderer(renderer),
    titleFont(nullptr),
    buttonFont(nullptr),
    scoreFont(nullptr),
    titleTexture(nullptr),
    backgroundTexture(nullptr),
    startButton(nullptr),
    scoresButton(nullptr),
    exitButton(nullptr),    startGame(false),
    exitGame(false),    showScores(false),
    scoreScale(1.0f) {  // Set to 100% scale as requested
    
    // Load high scores during initialization
    LoadHighScores();
}

MainMenu::~MainMenu() {
    // Clean up title texture
    if (titleTexture) {
        SDL_DestroyTexture(titleTexture);
        titleTexture = nullptr;
    }
    
    // Clean up background texture
    if (backgroundTexture) {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
      // Clean up fonts
    if (titleFont) {
        TTF_CloseFont(titleFont);
        titleFont = nullptr;
    }
    
    if (buttonFont) {
        TTF_CloseFont(buttonFont);
        buttonFont = nullptr;
    }
    
    // Clean up score font if it's different from button font
    if (scoreFont && scoreFont != buttonFont) {
        TTF_CloseFont(scoreFont);
        scoreFont = nullptr;
    }
      // Clean up buttons
    if (startButton) {
        delete startButton;
        startButton = nullptr;
    }
    
    if (scoresButton) {
        delete scoresButton;
        scoresButton = nullptr;
    }
    
    if (exitButton) {
        delete exitButton;
        exitButton = nullptr;
    }
}

bool MainMenu::Initialize() {
    // Load fonts
    titleFont = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", 72);
    if (!titleFont) {
        std::cerr << "Failed to load title font: " << TTF_GetError() << std::endl;
        return false;
    }

    buttonFont = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", 32);
    if (!buttonFont) {
        std::cerr << "Failed to load button font: " << TTF_GetError() << std::endl;
        return false;
    }
      // Load font for high scores - slightly larger base size to ensure readability at smaller scale
    scoreFont = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", Scale(32));
    if (!scoreFont) {
        std::cerr << "Failed to load score font: " << TTF_GetError() << std::endl;
        // Not critical, fall back to button font
        scoreFont = buttonFont;
    }

    // Create title texture
    SDL_Color titleColor = {255, 255, 255, 255};
    SDL_Surface* titleSurface = TTF_RenderText_Blended(titleFont, "Zombie Shooter", titleColor);
    if (!titleSurface) {
        std::cerr << "Failed to create title surface: " << TTF_GetError() << std::endl;
        return false;
    }

    titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    SDL_FreeSurface(titleSurface);

    if (!titleTexture) {
        std::cerr << "Failed to create title texture: " << SDL_GetError() << std::endl;
        return false;
    }    // Create buttons - centered horizontally, stacked vertically
    int buttonX = (Constants::WINDOW_WIDTH - BUTTON_WIDTH) / 2;
    int buttonY = Constants::WINDOW_HEIGHT / 2; // Start buttons from middle of screen
    
    startButton = new Button(renderer, buttonFont, "Start Game", buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT);
    
    buttonY += BUTTON_HEIGHT + BUTTON_PADDING;
    scoresButton = new Button(renderer, buttonFont, "Scores", buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT);
    
    buttonY += BUTTON_HEIGHT + BUTTON_PADDING;
    exitButton = new Button(renderer, buttonFont, "Exit", buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT);
    
    // Optional: Load background texture
    /*
    SDL_Surface* bgSurface = IMG_Load("assets/menu_background.png");
    if (bgSurface) {
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
        SDL_FreeSurface(bgSurface);
    }
    */
    
    return true;
}

void MainMenu::HandleEvents(const SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        int mouseX, mouseY;
        bool mouseDown = (event.type == SDL_MOUSEBUTTONDOWN);
        SDL_GetMouseState(&mouseX, &mouseY);        
        // Update buttons with mouse state
        startButton->Update(mouseX, mouseY, mouseDown);
        scoresButton->Update(mouseX, mouseY, mouseDown);
        exitButton->Update(mouseX, mouseY, mouseDown);
    }
}

void MainMenu::Update(float deltaTime) {
    // Check for button clicks
    if (startButton->IsClicked()) {
        startGame = true;
    }
    
    if (scoresButton->IsClicked()) {
        showScores = true;
    }
    
    if (exitButton->IsClicked()) {
        exitGame = true;
    }
}

void MainMenu::Render() {
    // Clear the screen with a dark color
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255); // Dark blue-ish color
    SDL_RenderClear(renderer);
    
    // Draw background if available
    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    }
    
    // Draw title
    if (titleTexture) {        int titleWidth, titleHeight;
        SDL_QueryTexture(titleTexture, nullptr, nullptr, &titleWidth, &titleHeight);
        SDL_Rect titleRect = {
            (Constants::WINDOW_WIDTH - titleWidth) / 2,  // Center horizontally
            Constants::WINDOW_HEIGHT / 4 - titleHeight / 2,  // Position at 1/4 of screen height
            titleWidth,
            titleHeight
        };
        SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);
    }
      // Draw buttons
    startButton->Render(renderer);
    scoresButton->Render(renderer);
    exitButton->Render(renderer);
    
    // Present the final rendered image
    SDL_RenderPresent(renderer);
}

bool MainMenu::ShouldStartGame() const {
    return startGame;
}

bool MainMenu::ShouldExitGame() const {
    return exitGame;
}

bool MainMenu::ShouldShowScores() const {
    return showScores;
}

void MainMenu::Reset() {
    // Reset the menu flags so we can go back to main menu properly
    startGame = false;
    exitGame = false;
    showScores = false;

    // Reset button states if needed
    if (startButton) {
        startButton->Reset();
    }
    if (scoresButton) {
        scoresButton->Reset();
    }
    if (exitButton) {
        exitButton->Reset();
    }
}

void MainMenu::LoadHighScores() {
    highScores.clear();
    
    std::cout << "MainMenu: Loading high scores..." << std::endl;
    
    // Try multiple paths to find the scores file
    std::vector<std::string> possiblePaths = {
        "d:/Game-CPP/score/scores.txt",
        "./score/scores.txt",
        "../score/scores.txt",
        "score/scores.txt"
    };
    
    bool fileOpened = false;
    std::ifstream file;
    
    for (const auto& path : possiblePaths) {
        file.open(path);
        if (file.is_open()) {
            std::cout << "Successfully opened scores file at: " << path << std::endl;
            fileOpened = true;
            break;
        }
    }
    
    if (fileOpened) {
        std::string line;
        int lineCount = 0;
        
        while (std::getline(file, line) && lineCount < 10) {
            lineCount++;
            
            // Skip empty lines or comment lines
            if (line.empty() || line[0] == '/' || line[0] == '#') {
                continue;
            }
            
            std::stringstream ss(line);
            std::string waveStr, date;
            
            if (std::getline(ss, waveStr, ',') && std::getline(ss, date)) {
                try {
                    ScoreEntry entry;
                    entry.wave = std::stoi(waveStr);
                    entry.date = date;
                    highScores.push_back(entry);
                    std::cout << "Loaded score: " << entry.wave << ", " << entry.date << std::endl;
                } catch (std::exception& e) {
                    std::cerr << "Error parsing score at line " << lineCount << ": " << e.what() << std::endl;
                }
            }
        }
        file.close();
        
        // Sort scores (highest first)
        std::sort(highScores.begin(), highScores.end(), 
                  [](const ScoreEntry& a, const ScoreEntry& b) {
                      return a.wave > b.wave;
                  });
    } else {
        std::cerr << "Failed to open scores.txt file from any known location" << std::endl;
        
        // If we can't open the file, add some default scores for testing
        highScores.push_back({10, "2025-05-22"});
        highScores.push_back({9, "2025-05-22"});
        highScores.push_back({8, "2025-05-21"});
    }
}

void MainMenu::RenderHighScores() {
    // Clear screen with a dark color
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255); // Dark blue-ish color
    SDL_RenderClear(renderer);
    
    // "HIGH SCORES" title - keep original size for title
    SDL_Color titleColor = {255, 215, 0, 255}; // Gold
    SDL_Surface* titleSurface = TTF_RenderText_Blended(titleFont, "HIGH SCORES", titleColor);
    if (titleSurface) {
        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
        SDL_FreeSurface(titleSurface);
        
        if (titleTexture) {            int titleWidth, titleHeight;
            SDL_QueryTexture(titleTexture, nullptr, nullptr, &titleWidth, &titleHeight);
            
            SDL_Rect titleRect = {
                (Constants::WINDOW_WIDTH - titleWidth) / 2,  // Center horizontally
                Scale(Constants::WINDOW_HEIGHT / 6) - titleHeight / 2,
                titleWidth,
                titleHeight
            };
            
            SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);
            SDL_DestroyTexture(titleTexture);
        }
    }    // Header for score table
    SDL_Color headerColor = {180, 180, 180, 255}; // Light gray
      // Calculate scaled positions for columns
    const int baseSpacing = 200;  // Base spacing between columns
    const int columnX1 = Constants::WINDOW_WIDTH / 2 - Scale(baseSpacing);  // RANK column position
    const int columnX2 = Constants::WINDOW_WIDTH / 2;                      // WAVE column position (centered)
    const int columnX3 = Constants::WINDOW_WIDTH / 2 + Scale(baseSpacing);  // DATE column position
    const int headerY = Scale(Constants::WINDOW_HEIGHT / 3 - 50);   // Header row Y position
      // Render RANK header
    SDL_Surface* rankSurface = TTF_RenderText_Blended(scoreFont, "RANK", headerColor);
    if (rankSurface) {
        SDL_Texture* rankTexture = SDL_CreateTextureFromSurface(renderer, rankSurface);
        SDL_FreeSurface(rankSurface);
        
        if (rankTexture) {
            int textWidth, textHeight;
            SDL_QueryTexture(rankTexture, nullptr, nullptr, &textWidth, &textHeight);
            
            SDL_Rect textRect = {
                columnX1,
                headerY,
                textWidth,
                textHeight
            };
            
            SDL_RenderCopy(renderer, rankTexture, nullptr, &textRect);
            SDL_DestroyTexture(rankTexture);
        }
    }
    
    // Render WAVE header
    SDL_Surface* waveSurface = TTF_RenderText_Blended(scoreFont, "WAVE", headerColor);
    if (waveSurface) {
        SDL_Texture* waveTexture = SDL_CreateTextureFromSurface(renderer, waveSurface);
        SDL_FreeSurface(waveSurface);
        
        if (waveTexture) {
            int textWidth, textHeight;
            SDL_QueryTexture(waveTexture, nullptr, nullptr, &textWidth, &textHeight);
            
            SDL_Rect textRect = {
                columnX2,
                headerY,
                textWidth,
                textHeight
            };
            
            SDL_RenderCopy(renderer, waveTexture, nullptr, &textRect);
            SDL_DestroyTexture(waveTexture);
        }
    }
    
    // Render DATE header
    SDL_Surface* dateSurface = TTF_RenderText_Blended(scoreFont, "DATE", headerColor);
    if (dateSurface) {
        SDL_Texture* dateTexture = SDL_CreateTextureFromSurface(renderer, dateSurface);
        SDL_FreeSurface(dateSurface);
        
        if (dateTexture) {
            int textWidth, textHeight;
            SDL_QueryTexture(dateTexture, nullptr, nullptr, &textWidth, &textHeight);
            
            SDL_Rect textRect = {
                columnX3,
                headerY,
                textWidth,
                textHeight
            };
            
            SDL_RenderCopy(renderer, dateTexture, nullptr, &textRect);
            SDL_DestroyTexture(dateTexture);
        }
    }    // Render scores (up to 10)
    int yPos = Scale(Constants::WINDOW_HEIGHT / 3 + 20);
    SDL_Color scoreColor = {255, 255, 255, 255}; // White
    
    if (highScores.empty()) {
        // No scores available message
        SDL_Surface* noScoresSurface = TTF_RenderText_Blended(scoreFont, "No scores yet. Play the game!", scoreColor);
        if (noScoresSurface) {
            SDL_Texture* noScoresTexture = SDL_CreateTextureFromSurface(renderer, noScoresSurface);
            SDL_FreeSurface(noScoresSurface);
            
            if (noScoresTexture) {
                int textWidth, textHeight;
                SDL_QueryTexture(noScoresTexture, nullptr, nullptr, &textWidth, &textHeight);
                  SDL_Rect textRect = {
                    (Constants::WINDOW_WIDTH - textWidth) / 2,
                    Scale(Constants::WINDOW_HEIGHT / 2) - textHeight / 2,
                    textWidth,
                    textHeight
                };
                
                SDL_RenderCopy(renderer, noScoresTexture, nullptr, &textRect);
                SDL_DestroyTexture(noScoresTexture);
            }
        }    } else {    // Render each score entry        // Use the same scaled column positions defined earlier
        int rowSpacing = Scale(45);  // Slightly increased spacing between rows for better readability
        int rowY = Scale(Constants::WINDOW_HEIGHT / 3 + 20);  // Starting position for rows
          for (size_t i = 0; i < highScores.size() && i < 10; ++i) {
            // Render RANK (index + 1)
            std::string rankStr = std::to_string(i + 1);
            SDL_Surface* rankSurface = TTF_RenderText_Blended(scoreFont, rankStr.c_str(), scoreColor);
            if (rankSurface) {
                SDL_Texture* rankTexture = SDL_CreateTextureFromSurface(renderer, rankSurface);
                SDL_FreeSurface(rankSurface);
                
                if (rankTexture) {
                    int textWidth, textHeight;                    SDL_QueryTexture(rankTexture, nullptr, nullptr, &textWidth, &textHeight);                      SDL_Rect textRect = {
                        columnX1,
                        rowY + static_cast<int>(static_cast<float>(i) * rowSpacing),
                        textWidth,
                        textHeight
                    };
                    
                    SDL_RenderCopy(renderer, rankTexture, nullptr, &textRect);
                    SDL_DestroyTexture(rankTexture);
                }
            }
            
            // Render WAVE
            std::string waveStr = std::to_string(highScores[i].wave);
            SDL_Surface* waveSurface = TTF_RenderText_Blended(scoreFont, waveStr.c_str(), scoreColor);
            if (waveSurface) {
                SDL_Texture* waveTexture = SDL_CreateTextureFromSurface(renderer, waveSurface);
                SDL_FreeSurface(waveSurface);
                
                if (waveTexture) {
                    int textWidth, textHeight;                    SDL_QueryTexture(waveTexture, nullptr, nullptr, &textWidth, &textHeight);                      SDL_Rect textRect = {
                        columnX2,
                        rowY + static_cast<int>(static_cast<float>(i) * rowSpacing),
                        textWidth,
                        textHeight
                    };
                    
                    SDL_RenderCopy(renderer, waveTexture, nullptr, &textRect);
                    SDL_DestroyTexture(waveTexture);
                }
            }
            
            // Render DATE
            SDL_Surface* dateSurface = TTF_RenderText_Blended(scoreFont, highScores[i].date.c_str(), scoreColor);
            if (dateSurface) {
                SDL_Texture* dateTexture = SDL_CreateTextureFromSurface(renderer, dateSurface);
                SDL_FreeSurface(dateSurface);
                
                if (dateTexture) {
                    int textWidth, textHeight;                    SDL_QueryTexture(dateTexture, nullptr, nullptr, &textWidth, &textHeight);                      SDL_Rect textRect = {
                        columnX3,
                        rowY + static_cast<int>(static_cast<float>(i) * rowSpacing),
                        textWidth,
                        textHeight
                    };
                    
                    SDL_RenderCopy(renderer, dateTexture, nullptr, &textRect);
                    SDL_DestroyTexture(dateTexture);
                }
            }
        }    }
    
    // No ESC text displayed anymore as requested
    
    // Present the final rendered image
    SDL_RenderPresent(renderer);
}

int MainMenu::Scale(int value) const {
    return static_cast<int>(static_cast<float>(value) * scoreScale);
}

void MainMenu::SetHighScoreScale(float scale) {
    // Limit scale to reasonable bounds
    if (scale < 0.5f) scale = 0.5f;
    if (scale > 1.5f) scale = 1.5f;
    
    // Only update if scale has actually changed
    if (scoreScale != scale) {
        scoreScale = scale;
        
        // Reload the score font with new size
        if (scoreFont && scoreFont != buttonFont) {
            TTF_CloseFont(scoreFont);
        }
          scoreFont = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", Scale(32));
        if (!scoreFont) {
            std::cerr << "Failed to reload score font with new scale: " << TTF_GetError() << std::endl;
            scoreFont = buttonFont; // Fall back to button font
        }
    }
}