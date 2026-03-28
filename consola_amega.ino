  #include <TFT_HX8357.h>
  #include <EEPROM.h>
  TFT_HX8357 tft = TFT_HX8357();

  #define COLS 10       
  #define ROWS 20  
  #define TETRIS_OFFSET_Y 35 

  #define GRID 20      
  #define MAX_SNAKE 200  

  #define SNAKE_W 360  
  #define SNAKE_H 320

  #define BTN_UP     4
  #define BTN_DOWN   5
  #define BTN_LEFT   6
  #define BTN_RIGHT  7

  #define TETRIS_X_LAND 140  
  #define TETRIS_Y_LAND 40

  int jocSelectat = 0;   // 0 = Meniu, 1 = Snake, 2 = Tetris
  int menuSelection = 1; // Ce opțiune este selectată (1 sau 2)

  // Matricea tablei de joc (0 = gol, 1-7 = blocuri blocate)
  int board[ROWS][COLS] = {0};

  // Cele 7 piese x 4 Rotații x 4 Rânduri x 4 Coloane
  const byte pieces[7][4][4][4] = {
    // 0: Forma I
    {
      {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
      {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}},
      {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
      {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
    },
    // 1: Forma T
    {
      {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
      {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
      {{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
      {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // 2: Forma O (Pătrat) 
    {
      {{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, 
      {{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, 
      {{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, 
      {{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}} 
    },
    // 3: Forma L
    {
      {{1,0,0,0}, {1,0,0,0}, {1,1,0,0}, {0,0,0,0}}, 
      {{1,1,1,0}, {1,0,0,0}, {0,0,0,0}, {0,0,0,0}}, 
      {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}, 
      {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}} 
    },
    // 4: Forma J
    {
      {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}, 
      {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, 
      {{1,1,0,0}, {1,0,0,0}, {1,0,0,0}, {0,0,0,0}}, 
      {{1,1,1,0}, {0,0,1,0}, {0,0,0,0}, {0,0,0,0}} 
    },
    // 5: Forma S
    {
      {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, 
      {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}, 
      {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, 
      {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}} 
    },
    // 6: Forma Z (S invers)
    {
      {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}, 
      {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}, 
      {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}, 
      {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}
    }
  };

  uint16_t colors[8] = {TFT_BLACK, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW, TFT_GREEN, TFT_RED, TFT_ORANGE, TFT_BLUE};

  int currentType = 0;     
  int currentRotation = 0; 
  int currentX = 3;        
  int currentY = 0;  
  int scor_tetris = 0;    
  int scor_snake = 0;  
  int highscore_snake = 0;
  int highscore_tetris = 0;
  int currentBlockSize = 20; 
  int tetrisMode = 0;        

  unsigned long lastDropTime = 0;
  unsigned long lastInputTime = 0; 
  int dropSpeed = 500;     
  bool gameOver = false;

  struct Point { int x, y; };
  Point snake[MAX_SNAKE];
  int snakeLen = 3;
  int dir = 1; 
  int lastMovedDir = 1;
  
  Point food;

  unsigned long prevM = 0;
  int speed = 100; 
  bool dead = false;
  bool gameStarted = false;


  void setup() {
    tft.init();

    pinMode(BTN_UP,     INPUT_PULLUP); 
    pinMode(BTN_DOWN,   INPUT_PULLUP);
    pinMode(BTN_LEFT,   INPUT_PULLUP);
    pinMode(BTN_RIGHT,  INPUT_PULLUP);

    randomSeed(analogRead(A15));

    tft.setRotation(1); 
    tft.fillScreen(TFT_BLACK);
    arataMeniu(true); 

    EEPROM.get(0, highscore_snake);
    EEPROM.get(10, highscore_tetris);

    if (highscore_snake < 0 || highscore_snake > 5000) highscore_snake = 0;
    if (highscore_tetris < 0 || highscore_tetris > 5000) highscore_tetris = 0;
  }

  void loop() {
    if (jocSelectat == 0) {
      navigheazaMeniu();
    } else if (jocSelectat == 1) {
      joacaSnake(); 
    } else if (jocSelectat == 2) {
      joacaTetris(); 
    }
  }

  void arataMeniu(bool clearScreen) {
    if (clearScreen) {
      tft.setRotation(1); 
      tft.fillScreen(TFT_BLACK);

      tft.setTextColor(TFT_CYAN);
      tft.setTextSize(2);
      tft.drawCentreString("ALEGE UN JOC", 240, 50, 4);
    }

    tft.setTextSize(2);

    if (menuSelection == 1) {
      tft.setTextColor(TFT_GREEN, TFT_BLACK); 
      tft.drawString("1. SNAKE ", 130, 150, 4);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("2. TETRIS", 130, 200, 4);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("1. SNAKE ", 130, 150, 4);
      tft.setTextColor(TFT_GREEN, TFT_BLACK); 
      tft.drawString("2. TETRIS", 130, 200, 4);
    }
  }

  void navigheazaMeniu() {
    if (millis() - lastInputTime > 200) { 
      if (digitalRead(BTN_DOWN) == LOW) {
        menuSelection = 2;
        arataMeniu(false); 
        lastInputTime = millis();
      }
      if (digitalRead(BTN_UP) == LOW) {
        menuSelection = 1;
        arataMeniu(false);
        lastInputTime = millis();
      }
      if (digitalRead(BTN_RIGHT) == LOW) {
        jocSelectat = menuSelection;
        if (jocSelectat == 1) startSnake();
        if (jocSelectat == 2) startTetris();
        lastInputTime = millis();
      }
    }
  }

  void startSnake() {
    tft.setRotation(1); 
    showStartScreen();
    gameStarted = false;
    dead = false;
    scor_snake = 0;
    delay(500); 
  }

  void joacaSnake(){

      if ((!gameStarted || dead) && digitalRead(BTN_LEFT) == LOW) {
          jocSelectat = 0;
          arataMeniu(true);
          delay(500); // pauză ca să nu sară prin meniu
          return;
      }

      // 2. Start joc nou (Doar butonul DREAPTA)
      if (!gameStarted) {
        if (digitalRead(BTN_RIGHT) == LOW) {
          gameStarted = true;
          resetGame();
          delay(200); // mic debouncing
        }
        return;
      }

      // 3. Restart după Game Over (Doar butonul DREAPTA)
      if (dead) {
        if (digitalRead(BTN_RIGHT) == LOW) {
          resetGame();
          delay(200); // mic debouncing
        }
        return;
      }

      readInputs();

      if (millis() - prevM > speed) {
        moveSnake();
        prevM = millis();
      }
  }

  bool checkAnyButton() {
    if (digitalRead(BTN_UP) == LOW || digitalRead(BTN_DOWN) == LOW || 
        digitalRead(BTN_LEFT) == LOW || digitalRead(BTN_RIGHT) == LOW) {
      delay(200); 
      return true;
    }
    return false;
  }
  
  void readInputs() {
    if (digitalRead(BTN_UP) == LOW    && lastMovedDir != 2) dir = 0;
    if (digitalRead(BTN_RIGHT) == LOW && lastMovedDir != 3) dir = 1;
    if (digitalRead(BTN_DOWN) == LOW  && lastMovedDir != 0) dir = 2;
    if (digitalRead(BTN_LEFT) == LOW  && lastMovedDir != 1) dir = 3;
  }
  
  void moveSnake() {
    lastMovedDir = dir;

    Point tailEnd = snake[snakeLen - 1];
  
    for (int i = snakeLen - 1; i > 0; i--) {
      snake[i] = snake[i - 1];
    }
  
    if (dir == 0) snake[0].y -= GRID;
    if (dir == 1) snake[0].x += GRID;
    if (dir == 2) snake[0].y += GRID;
    if (dir == 3) snake[0].x -= GRID;
  
    if (snake[0].x < 0 || snake[0].x >= SNAKE_W || snake[0].y < 0 || snake[0].y >= SNAKE_H) {
      endGame(); return;
    }
  
    for (int i = 1; i < snakeLen; i++) {
      if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
        endGame(); return;
      }
    }
  
    if (snake[0].x == food.x && snake[0].y == food.y) {
      if (snakeLen < MAX_SNAKE) {
        snake[snakeLen] = tailEnd; 
        snakeLen++;
      }
      spawnFood();
      scor_snake = scor_snake + 1;
      if(scor_snake >= highscore_snake){
        highscore_snake = scor_snake;
        EEPROM.put(0, highscore_snake);
      }
      actualizeazaScorSnake();
      if (speed > 50) speed -= 2;
    } else {
      tft.fillRect(tailEnd.x, tailEnd.y, GRID, GRID, TFT_BLACK);
    }
  
    tft.fillRect(snake[0].x, snake[0].y, GRID, GRID, TFT_GREEN);
    tft.fillRect(food.x, food.y, GRID, GRID, TFT_RED);
  }
  
  void spawnFood() {
    bool onSnake;
    do {
      onSnake = false;
      food.x = (random(0, SNAKE_W / GRID)) * GRID;
      food.y = (random(0, SNAKE_H / GRID)) * GRID;
      
      for (int i = 0; i < snakeLen; i++) {
        if (food.x == snake[i].x && food.y == snake[i].y) {
          onSnake = true;
          break;
        }
      }
    } while (onSnake);
  }

  void actualizeazaScorSnake() {
    tft.setCursor(SNAKE_W + 30, 90); 
    tft.setTextColor(TFT_WHITE, TFT_BLACK); 
    tft.setTextSize(3); 
    tft.print(scor_snake);

    tft.setCursor(SNAKE_W + 30, 200); 
    tft.setTextColor(TFT_WHITE, TFT_BLACK); 
    tft.setTextSize(3); 
    tft.print(highscore_snake);
  }
  
  void showStartScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(3);
    tft.drawCentreString("SNAKE", tft.width()/2, tft.height()/2 - 70, 4);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    // Instrucțiuni noi:
    tft.drawCentreString("Apasa DREAPTA pt START", tft.width()/2, tft.height()/2 + 30, 2);
    tft.setTextColor(TFT_CYAN);
    tft.drawCentreString("Apasa STANGA pt MENIU", tft.width()/2, tft.height()/2 + 60, 2);
  }

  void resetGame() {
    tft.fillScreen(TFT_BLACK);

    tft.drawLine(SNAKE_W, 0, SNAKE_W, SNAKE_H, TFT_WHITE);

    tft.setTextColor(TFT_YELLOW); 
    tft.setTextSize(1);
    tft.drawString("SCOR:", SNAKE_W + 20, 50, 4);
    tft.drawString("BEST:", SNAKE_W + 20, 160, 4);

    snakeLen = 3;
    dir = 1;
    lastMovedDir = 1;
    speed = 150;
    dead = false;
    scor_snake = 0;

    actualizeazaScorSnake();
    
    int startX = GRID * 5;
    int startY = GRID * 5;
    for(int i = 0; i < snakeLen; i++) {
      snake[i].x = startX - (i * GRID);
      snake[i].y = startY;
    }
    spawnFood();
  }
  
  void endGame() {
     dead = true;
     tft.fillScreen(TFT_BLACK);
     tft.setTextColor(TFT_RED);
     tft.setTextSize(3);
     tft.drawCentreString("GAME OVER", tft.width()/2, tft.height()/2 - 140, 4);
     tft.setTextColor(TFT_WHITE);
     tft.setTextSize(2);
     tft.drawCentreString("SCOR : ", tft.width()/2-25, tft.height()/2-30, 2);
     tft.drawNumber(scor_snake,tft.width()/2+30, tft.height()/2-30, 2);
     tft.drawCentreString("BEST : ", tft.width()/2-25, tft.height()/2, 2);
     tft.drawNumber(highscore_snake,tft.width()/2+30, tft.height()/2, 2);
     tft.setTextColor(TFT_GREEN);
     tft.setTextSize(2);
     tft.drawCentreString("Apasa DREAPTA pt RESTART", tft.width()/2, tft.height()/2 + 80, 2);
     tft.setTextColor(TFT_CYAN);
     tft.drawCentreString("Apasa STANGA pt MENIU", tft.width()/2, tft.height()/2 + 110, 2);
     delay(500); 
  }

// pana aici functii snake

  void showStartScreenTetris() {
  tft.setRotation(1);
  
  tft.fillRect(0, 120, 480, 100, TFT_BLACK); 

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.drawCentreString("TETRIS CONFIG", 240, 50, 4);

  tft.setTextSize(2);
  // Portrait
  tft.setTextColor(tetrisMode == 0 ? TFT_GREEN : TFT_WHITE);
  tft.drawCentreString("-> MOD PORTRAIT  ", 240, 130, 2);

  // Landscape
  tft.setTextColor(tetrisMode == 1 ? TFT_GREEN : TFT_WHITE);
  tft.drawCentreString("-> MOD LANDSCAPE", 240, 170, 2);

  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(1);
  tft.drawCentreString("Apasa DREAPTA pt START", 240, 250, 2);
  tft.setTextColor(TFT_CYAN);
  tft.drawCentreString("Apasa STANGA pt MENIU", 240, 270, 2);
  }

  void startTetris() {
    tft.setRotation(0); 

    gameOver = false;
    gameStarted = false; 

    for(int r=0; r<ROWS; r++) {
      for(int c=0; c<COLS; c++) {
        board[r][c] = 0;
      }
    }
    scor_tetris = 0;          

    tft.fillScreen(TFT_BLACK);
    showStartScreenTetris();
    delay(500);
  }

  void resetTetris() {
    tft.fillScreen(TFT_BLACK);
    int offsetX = (tetrisMode == 0) ? 20 : (480 - (COLS * currentBlockSize)) / 2;
    int offsetY = (tetrisMode == 0) ? 35 : (320 - (ROWS * currentBlockSize)) / 2;

    for(int r=0; r<ROWS; r++) {
      for(int c=0; c<COLS; c++) {
        board[r][c] = 0;
      }
    }
    scor_tetris = 0;
    dropSpeed = 500;
    gameOver = false;

    tft.drawRect(offsetX - 1, offsetY - 1, (COLS * currentBlockSize) + 2, (ROWS * currentBlockSize) + 2, TFT_WHITE);

    int scoreX = offsetX + (COLS * currentBlockSize) + 20;

    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW); 
    tft.drawString("SCOR:", scoreX, 50, 4);

    tft.setCursor(scoreX + 10, 90);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3); 
    tft.print(scor_tetris);

    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW); 
    tft.drawString("BEST:", scoreX, 130, 4);

    tft.setCursor(scoreX + 10, 170);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); 
    tft.setTextSize(3); 
    tft.print(highscore_tetris);

    spawnPiece();
  }


  void joacaTetris(){
    if (gameOver) {
      // Aici doar citim butoanele, NU mai desenăm nimic pe ecran
      if (digitalRead(BTN_LEFT) == LOW) {
        jocSelectat = 0;
        arataMeniu(true);
        delay(500);
      }
      if (digitalRead(BTN_RIGHT) == LOW) {
        startTetris(); // Resetăm jocul
        delay(500);
      }
      return; // Ieșim imediat
    }

    if (!gameStarted) {
      if (digitalRead(BTN_UP) == LOW)   { tetrisMode = 0; currentBlockSize = 20; showStartScreenTetris(); delay(150); }
      if (digitalRead(BTN_DOWN) == LOW) { tetrisMode = 1; currentBlockSize = 15; showStartScreenTetris(); delay(150); }

      if (digitalRead(BTN_RIGHT) == LOW) {
        gameStarted = true;
        tft.setRotation(tetrisMode); // Aplicăm rotația aleasă!
        resetTetris();
        delay(200);
      }
      if (digitalRead(BTN_LEFT) == LOW) { // Înapoi la meniul principal
        jocSelectat = 0;
        arataMeniu(true);
        delay(500);
      }
      return;
    }

    if (millis() - lastInputTime > 150) {
      bool moved = false;
      int nextX = currentX;
      int nextY = currentY;
      int nextRot = currentRotation;

      if (digitalRead(BTN_LEFT) == LOW)   { nextX--; moved = true; }
      if (digitalRead(BTN_RIGHT) == LOW)  { nextX++; moved = true; }
      if (digitalRead(BTN_DOWN) == LOW)   { nextY++; moved = true; }
      if (digitalRead(BTN_UP) == LOW) { nextRot = (currentRotation + 1) % 4; moved = true; }

      if (moved) {
        if (!checkCollision(nextX, nextY, nextRot)) {
          drawPiece(currentX, currentY, currentType, currentRotation, TFT_BLACK);
          currentX = nextX;
          currentY = nextY;
          currentRotation = nextRot;
          drawPiece(currentX, currentY, currentType, currentRotation, colors[currentType + 1]);
        }
        lastInputTime = millis();
      }
    }

    if (millis() - lastDropTime > dropSpeed) {
      if (!checkCollision(currentX, currentY + 1, currentRotation)) {
        drawPiece(currentX, currentY, currentType, currentRotation, TFT_BLACK);
        currentY++;
        drawPiece(currentX, currentY, currentType, currentRotation, colors[currentType + 1]);
      } else {
        lockPiece();
        clearLines();
        drawBoard(); 
        spawnPiece();
      }
      lastDropTime = millis();
    }

  }

  void spawnPiece() {
    currentType = random(0, 7);
    currentRotation = 0;
    currentX = 3;
    currentY = 0;

    if (checkCollision(currentX, currentY, currentRotation)) {
        gameOver = true;

      // Desenăm ecranul de moarte O SINGURĂ DATĂ
      tft.setRotation(1); // Revenim la Landscape pentru mesajul de Game Over
      tft.fillScreen(TFT_BLACK);

      tft.setTextColor(TFT_RED);
      tft.setTextSize(3);
      tft.drawCentreString("GAME OVER", 240, 60, 4);

      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.drawCentreString(("SCOR : " + String(scor_tetris)).c_str(), 240, 135, 2);
      tft.drawCentreString(("BEST : " + String(highscore_tetris)).c_str(), 240, 165, 2);

      tft.setTextColor(TFT_GREEN);
      tft.drawCentreString("Apasa DREAPTA pt RESTART", 240, 210, 2);
      tft.setTextColor(TFT_CYAN);
      tft.drawCentreString("Apasa STANGA pt MENIU", 240, 240, 2);
    } else {
      drawPiece(currentX, currentY, currentType, currentRotation, colors[currentType + 1]);
    }
  }

  bool checkCollision(int testX, int testY, int testRot) {
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        if (pieces[currentType][testRot][r][c] == 1) {
          int boardX = testX + c;
          int boardY = testY + r;

          if (boardX < 0 || boardX >= COLS || boardY >= ROWS) return true;

          if (boardY >= 0 && board[boardY][boardX] != 0) return true;
        }
      }
    }
    return false;
  }

  void lockPiece() {
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        if (pieces[currentType][currentRotation][r][c] == 1) {
          board[currentY + r][currentX + c] = currentType + 1; 
        }
      }
    }
  }

  void clearLines() {
  bool linesClearedThisTurn = false;

  for (int y = ROWS - 1; y >= 0; y--) {
    bool fullLine = true;

    for (int x = 0; x < COLS; x++) {
      if (board[y][x] == 0) fullLine = false;
    }

    if (fullLine) {
      linesClearedThisTurn = true;
      for (int moveY = y; moveY > 0; moveY--) {
        for (int x = 0; x < COLS; x++) {
          board[moveY][x] = board[moveY - 1][x];
        }
      }
      for (int x = 0; x < COLS; x++) {
        board[0][x] = 0;
      }
      y++; 
      scor_tetris = scor_tetris + 10;
      if (scor_tetris >= highscore_tetris){
        highscore_tetris = scor_tetris;
        EEPROM.put(10, highscore_tetris);
      }
      
      // --- CALCULĂM POZIȚIA CORECTĂ A SCORULUI ÎN FUNCȚIE DE MOD ---
      int offsetX = (tetrisMode == 0) ? 20 : (480 - (COLS * currentBlockSize)) / 2;
      int scoreX = offsetX + (COLS * currentBlockSize) + 20;

      tft.setCursor(scoreX + 10, 90);
      tft.setTextColor(TFT_WHITE, TFT_BLACK); 
      tft.setTextSize(3); 
      tft.print(scor_tetris);
      
      tft.setCursor(scoreX + 10, 170);
      tft.setTextColor(TFT_WHITE, TFT_BLACK); 
      tft.setTextSize(3);
      tft.print(highscore_tetris);
      // -------------------------------------------------------------

      drawBoard();
    }
  }
  if(linesClearedThisTurn)
    if(dropSpeed > 120)
      dropSpeed -= 5;
}

  void drawPiece(int x, int y, int type, int rot, uint16_t color) {
    int offsetX = (tetrisMode == 0) ? 20 : (480 - (COLS * currentBlockSize)) / 2;
    int offsetY = (tetrisMode == 0) ? 35 : (320 - (ROWS * currentBlockSize)) / 2;
    
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        if (pieces[type][rot][r][c] == 1) {
          int px = offsetX + (x + c) * currentBlockSize;
          int py = offsetY + (y + r) * currentBlockSize;

          if(color == TFT_BLACK)
            tft.fillRect(px, py, currentBlockSize, currentBlockSize, TFT_BLACK);
          else
            tft.fillRect(px, py, currentBlockSize - 1, currentBlockSize - 1, color);
        }
      }
    }
  }

  void drawBoard() {
    int offsetX = (tetrisMode == 0) ? 20 : (480 - (COLS * currentBlockSize)) / 2;
    int offsetY = (tetrisMode == 0) ? 35 : (320 - (ROWS * currentBlockSize)) / 2;
    for (int y = 0; y < ROWS; y++) {
      for (int x = 0; x < COLS; x++) {
        int px = offsetX + x * currentBlockSize;
        int py = offsetY + y * currentBlockSize;

        int colorIdx = board[y][x];
        uint16_t color = colors[colorIdx];

        if (color == TFT_BLACK) 
          tft.fillRect(px, py, currentBlockSize, currentBlockSize, TFT_BLACK);
        else 
          tft.fillRect(px, py, currentBlockSize - 1, currentBlockSize - 1, color);
      }
    }
  }
  //pana aici functii tetris