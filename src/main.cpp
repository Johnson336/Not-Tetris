#define ALLEGRO_NO_MAGIC_MAIN
#define KEY_SEEN 1
#define KEY_RELEASED 2

#include <stdio.h>
#include <stdlib.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include <utility>
#include <time.h>

#include "Individual.hpp"

#include <iostream>


/*
TODO:

Game Menus -
 - Splash screen
 - Title screen
-> - Game Menu
 - High Score Menu
 - Options Menu
   - Windowed/Fullscreen
   - Color Mode
   - Controls - 
     - Modify controls
 - Game Over Menu -
   - High score name entry
   - Exit to Menu
   - New Game

Gameplay - 
 - Level-based drop speed increase
 - Game modes -
   - Powerups
   - Trash pileup 
   - Timed Mode
   - Versus Mode
   - Chellenge Mode (Preset Obstacle-Filled Levels)

Visual Effects - 
 - Animate held piece transitions
 - Animate row clears
 - Effects for combos - 
   - Glowing current Tetromino

Sound Effects -
 - Tetromino landing
 - Tetromino collision
 - Tetromino rotate
 - Tetromino hold
 - Line clear - 
   - Single line
   - Double line
   - Triple line
   - Tetris line
   - Combo Scores - 
     - Increasing line clear combos
   - Game over Death Sound
Pause Music on Pause/During Menus





*/



void must_init(bool test, const char *description) {
  if (test) return;

  printf("Unable to initialize %s\n", description);
  exit(1);
}

  /*
  turn AI on or off
  *
  *
  */
const bool using_ai = true;


const int Rows = 20;
const int Cols = 10;
const int border_width = 1;
const double frames = 1.0 / 60.0;
double level_timer = 2.0;
double volume = 0.0;
double ai_timer_mps = 60.0;

unsigned int time_ui = static_cast<unsigned int>( time(NULL) );



const float piece_size = 25.0;
const float piece_buffer = 2.0;
const float total_piece_size = piece_size + piece_buffer;

const int gameboard_width = (total_piece_size * Cols);
const int gameboard_height = (total_piece_size * Rows);

const int window_max_x = gameboard_width + 350;
const int window_max_y = gameboard_height + 100;

ALLEGRO_COLOR cyan = al_map_rgb(0, 255, 255);
ALLEGRO_COLOR yellow = al_map_rgb(255, 255, 0);
ALLEGRO_COLOR purple = al_map_rgb(128, 0, 128);
ALLEGRO_COLOR green = al_map_rgb(0, 255, 0);
ALLEGRO_COLOR red = al_map_rgb(255, 0, 0);
ALLEGRO_COLOR blue = al_map_rgb(0, 0, 255);
ALLEGRO_COLOR orange = al_map_rgb(255, 127, 0);
ALLEGRO_COLOR grey = al_map_rgb(127, 127, 127);
ALLEGRO_COLOR black = al_map_rgb(0, 0, 0);
ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);


// Represents a single block in a Tetromino
struct Block {
  int row;
  int col;
};

// Represents a Tetromino piece
struct Tetromino {
  Block blocks[16];
  char c;
  int row;
  int col;
  int rotation;
  ALLEGRO_COLOR color;
};

// The Tetromino shapes
const Tetromino Tetrominoes[7] = {
  {{ {1, 0}, {1, 1}, {1, 2}, {1, 3},
     {0, 2}, {1, 2}, {2, 2}, {3, 2},
     {1, 0}, {1, 1}, {1, 2}, {1, 3},
     {0, 2}, {1, 2}, {2, 2}, {3, 2} }, 'I', 0, 0, 0, cyan},
  {{ {1, 0}, {1, 1}, {1, 2}, {2, 1},
     {0, 1}, {1, 0}, {1, 1}, {2, 1},
     {1, 1}, {2, 0}, {2, 1}, {2, 2},
     {0, 1}, {1, 1}, {1, 2}, {2, 1} }, 'T', 0, 0, 0, purple},
  {{ {1, 0}, {1, 1}, {1, 2}, {2, 0},
     {0, 0}, {0, 1}, {1, 1}, {2, 1},
     {1, 2}, {2, 0}, {2, 1}, {2, 2},
     {0, 1}, {1, 1}, {2, 1}, {2, 2} }, 'L', 0, 0, 0, orange},
  {{ {1, 0}, {1, 1}, {1, 2}, {2, 2},
     {0, 1}, {1, 1}, {2, 0}, {2, 1},
     {1, 0}, {2, 0}, {2, 1}, {2, 2},
     {0, 1}, {0, 2}, {1, 1}, {2, 1} }, 'J', 0, 0, 0, blue},
  {{ {1, 1}, {1, 2}, {2, 0}, {2, 1},
     {0, 0}, {1, 0}, {1, 1}, {2, 1},
     {1, 1}, {1, 2}, {2, 0}, {2, 1},
     {0, 0}, {1, 0}, {1, 1}, {2, 1} }, 'S', 0, 0, 0, green},
  {{ {1, 0}, {1, 1}, {2, 1}, {2, 2},
     {0, 2}, {1, 1}, {1, 2}, {2, 1},
     {1, 0}, {1, 1}, {2, 1}, {2, 2},
     {0, 2}, {1, 1}, {1, 2}, {2, 1} }, 'Z', 0, 0, 0, red},
  {{ {1, 1}, {1, 2}, {2, 1}, {2, 2},
     {1, 1}, {1, 2}, {2, 1}, {2, 2},
     {1, 1}, {1, 2}, {2, 1}, {2, 2},
     {1, 1}, {1, 2}, {2, 1}, {2, 2} }, 'O', 0, 0, 0, yellow},
};

/* Initialize functions */

// bool performMoveDown(ALLEGRO_AUDIO_STREAM* music, ALLEGRO_SAMPLE* line_clear, ALLEGRO_SAMPLE* tetromino_land);
// bool performManualMoveDown(ALLEGRO_AUDIO_STREAM* music, ALLEGRO_SAMPLE* line_clear, ALLEGRO_SAMPLE* tetromino_land);
bool performDrop();
bool performMoveRight();
bool performMoveLeft();
// bool performRotate(Tetromino &t, int y, int x);

// The board grid
char board[Rows][Cols];


// Details of the current Tetromino
Tetromino nextTetromino;
Tetromino curTetromino;
char curTetrominoShape;
Tetromino heldTetromino;
bool held;
bool performedSwap = false;
bool firstRun = true;
int score;
int prevScore;
int level;
int clearedRows;
bool paused;
bool redraw;
bool advanceLogic;
Tetromino highlight;
int tetBag[7] = {0, 1, 2, 3, 4, 5, 6};
int tetBagIter;

void initialize_AI();
void calcNeuronFitness();
void assignNewNeuron();

// The state of the board passed to the AI
std::string boardState;

// declare our AI variables
std::vector<Individual> population;
Neuron currentNeuron = Neuron(curTetrominoShape, boardState);
int generation;
int genIter;
int actionIter;

// Clears the board
void ClearBoard() {
  for (int i = 0; i < Rows; i++) {
    for (int j = 0; j < Cols; j++) {
      board[i][j] = ' ';
    }
  }
}


// Pick next tetromino from the bag and remove it
Tetromino getNextTetromino() {
  return Tetrominoes[tetBag[tetBagIter++]];
}

// reshuffle the tet bag and pull one into nextTetromino
void shuffleTetBag() {
  // Shuffle Tets to be picked
  for (int i=0;i<20;i++) {
    // Swap element 0 with random location 20 times to ensure shuffle
    // 7/1 + 7/2 + 7/3 + 7/4 + 7/5 + 7/6 + 7/7 = 18.15 shuffles
    std::swap(tetBag[0], tetBag[(rand() % 6) + 1]);
  }
  // reset tetBagIter to 0 so getNextTetromino() will pull the first item
  tetBagIter = 0;
  nextTetromino = getNextTetromino();
}



// shifts nextTet into curTet and pulls a new nextTet
void NewTetromino() {
  performedSwap = false;
  curTetromino = nextTetromino;
  curTetrominoShape = curTetromino.c; // 
  curTetromino.row = -1;
  curTetromino.col = (Cols/3) + 1;
  // Bag is empty
  if (tetBagIter >= 6) {
    shuffleTetBag();
  } else {
    // Pick next Tetromino from the bag and remove it
    nextTetromino = getNextTetromino();
  }
}



// Initializes the board and sets up NCurse
void Init() {
  shuffleTetBag();
  NewTetromino();
  tetBagIter = 0;
  score = 0;
  prevScore = 0;
  level = 1;
  clearedRows = 0;
  held = false;
  paused = false;
  redraw = true;
  advanceLogic = false;
  performedSwap = false;
  ClearBoard();
  if (firstRun) {
    srand( time_ui );
    initialize_AI();
    firstRun = false;
  }
  boardState = "";
}

int increaseScore(int lines) {
  switch (lines) {
    case 1:
      return 40 * (level + 1);
      break;
    case 2:
      return 100 * (level + 1);
      break;
    case 3:
      return 300 * (level + 1);
      break;
    case 4:
      return 1200 * (level + 1);
      break;
    default:
      return 0;
      break;
  }
}

// Copies the current Tetromino onto the board
void CopyToBoard() {

  if (using_ai) 
    calcNeuronFitness();

  for (int i=0;i<4;i++) {
    Block &b = curTetromino.blocks[i + curTetromino.rotation];
    board[curTetromino.row + b.row][curTetromino.col + b.col] = curTetromino.c;
  }

  if (using_ai) {
    boardState = "";
    for (int i=0;i<Rows;i++) {
      for (int j=0;j<Cols;j++) {
        if (board[i][j] != ' ') {
          // found topmost row, copy that row into boardState
          for (int j=0;j<Cols;j++) {
            boardState += ((board[i][j] != ' ') ? '+' : '-');
          }
          return;
        }
      }
    }
  }
}

// Rotates the current Tetromino clockwise
void RotateTetromino(Tetromino &t) {
  // rotations are 0->4->8->12->0
  t.rotation+=4;
  if (t.rotation > 12)
    t.rotation = 0;
}

// Moves the current Tetromino left
void MoveTetrominoLeft() {
  curTetromino.col--;
}

// Moves the current Tetromino right
void MoveTetrominoRight() {
  curTetromino.col++;
}

// Moves the current Tetromino down
void MoveTetrominoDown() {
  curTetromino.row++;
}

void MoveTetrominoUp() {
  curTetromino.row--;
}


// Returns true if the current Tetromino collides with something
bool Collision(Tetromino &t, int y, int x) {
  for (int i = 0; i < 4; i++) {
    Block &b = t.blocks[i + t.rotation];
    // This allows collision checking for any piece passed as parameter
    int row = y + b.row; 
    int col = x + b.col;

    // Bounds and overlap checking
    if (row >= Rows || row < 0 || col < 0 || col >= Cols || board[row][col] != ' ') {
      return true;
    }
  }

  return false;
}

bool HoldTetromino() {
  // Already performed a swap, unable to swap again until next block
  if (performedSwap)
    return false;
  // Swap held and current Tetrominos
  if (held) {
    // Check if held piece will fit
    // Held piece bumped against a solid surface, try to kick off of it
      int row = curTetromino.row;
      int col = curTetromino.col;
    if (Collision(heldTetromino, row, col)) {
      // Wall kicks
      if (!Collision(heldTetromino, row, col + 1)) {
        return performMoveRight();
      } else if (!Collision(heldTetromino, row, col - 1)) {
        return performMoveLeft();
      } else if (!Collision(heldTetromino, row + 1, col)) {
        MoveTetrominoDown();
        return true;
      }
      // Unable to find a valid location to satisfy wall kick
      // Return false swap condition
      return false;
    }
    // Swapping in the piece required a wall kick, but it succeeded
    // Finally swap in the piece
    std::swap(curTetromino, heldTetromino);
    // Reset swapped piece to top of screen
    curTetromino.row = 0;
    // Only allow one piece swap per block
    performedSwap = true;
  } else {
    // Put current Tetromino into held and get a new one
    performedSwap = true;
    held = true;
    heldTetromino = curTetromino;
    NewTetromino();
  }
  return true;
}

// Pause and un-pause the game
void Pause() {
  paused = !paused;
}

// Drops the current Tetromino down until it collides with something
void DropTetromino() {
  int droppedRows = 0;
  while (!Collision(curTetromino, curTetromino.row, curTetromino.col)) {
    droppedRows++;
    MoveTetrominoDown();
  }
  if (droppedRows)
    score += --droppedRows;
  curTetromino.row--;
}

// Removes any completed rows
void RemoveCompletedRows(ALLEGRO_AUDIO_STREAM* music, ALLEGRO_SAMPLE* line_clear) {
  int completedRows[Rows];
  int numCompletedRows = 0;

  for (int i = 0; i < Rows; i++) {
    bool completed = true;

    for (int j = 0; j < Cols; j++) {
      if (board[i][j] == ' ') {
        completed = false;
        break;
      }
    }

    if (completed) {
      completedRows[numCompletedRows++] = i;
    }
  }

  for (int i = 0; i < numCompletedRows; i++) {
    for (int j = completedRows[i]; j > 0; j--) {
      for (int k = 0; k < Cols; k++) {
        board[j][k] = board[j - 1][k];
      }
    }
  }
  if (numCompletedRows) {
    score += increaseScore(numCompletedRows);
    clearedRows += numCompletedRows;
    if (clearedRows >= (level * 10)) {
      level++;
      // advanced level, play level_increase sound modified by number
      // of rows cleared
      // double speed at level 30
      al_set_audio_stream_speed(music, 1+(level*(1/30)));
    } else {
      // didn't advance level, play line_clear sound modified by number
      // of rows cleared
      // twice as fast at 4 row clear vs 1 row clear
      al_play_sample(line_clear, volume, 0.0, 1.0 + (numCompletedRows * 0.25), ALLEGRO_PLAYMODE_ONCE, NULL);
    }
  } else {
    // didn't complete any rows, play tetromino_land sound
    // play land sound to indicate piece landed
    // al_play_sample(tetromino_land, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
  }
}

int gameboard_start_x = 50;
int gameboard_start_y = 50;

// Draws a solid color box using col/row (float) coordinates
// for finer precision
// drawBox(float columns, float rows, float width, float height, color);
void drawBox(float col, float row, float width, float height, ALLEGRO_COLOR color) {
  al_draw_filled_rectangle((gameboard_start_x + (col * total_piece_size)),
                           (gameboard_start_y + (row * total_piece_size)),
                           (gameboard_start_x + ((col+width) * total_piece_size)),
                           (gameboard_start_y + ((row+height) * total_piece_size)),
                           color);
}

// Draws a solid color white outline using col/row (float) coordinates
// for finer precision
// Draws a square outline using Row/Col coordinates instead of pixels
void drawBoxOutline(float col, float row, float width, float height, float thickness, ALLEGRO_COLOR color) {
  al_draw_rectangle((gameboard_start_x + (col * total_piece_size)) - thickness,
                           (gameboard_start_y + (row * total_piece_size)) - thickness,
                           (gameboard_start_x + ((col + width) * total_piece_size)) + thickness,
                           (gameboard_start_y + ((row + height) * total_piece_size)) + thickness,
                           color, thickness);
}

// Draws a square using Row/Col coordinates instead of pixels
void drawSquare(int col, int row, ALLEGRO_COLOR color) {
  al_draw_filled_rectangle((gameboard_start_x + (col * total_piece_size)) + piece_buffer,
                           (gameboard_start_y + (row * total_piece_size)) + piece_buffer,
                           (gameboard_start_x + ((col+1) * total_piece_size)) - piece_buffer,
                           (gameboard_start_y + ((row+1) * total_piece_size)) - piece_buffer,
                           color);
}
// Draws a square outline using Row/Col coordinates instead of pixels
void drawOutline(int col, int row, ALLEGRO_COLOR color) {
  al_draw_rectangle((gameboard_start_x + (col * total_piece_size)) + piece_buffer,
                           (gameboard_start_y + (row * total_piece_size)) + piece_buffer,
                           (gameboard_start_x + ((col+1) * total_piece_size)) - piece_buffer,
                           (gameboard_start_y + ((row+1) * total_piece_size)) - piece_buffer,
                           color, 1.0);
}
// Draws text using Row/Col coordinates instead of pixels
void drawText(const char *text, float col, float row, ALLEGRO_FONT* f, ALLEGRO_COLOR c) {
  al_draw_text(f, c, (gameboard_start_x + (col * total_piece_size)), (gameboard_start_y + (row * total_piece_size)), 0, text);
}

void drawGrid() {
  for (int i=0;i<Cols;i++) {
    al_draw_line(
      (gameboard_start_x + (i * total_piece_size)),
      (gameboard_start_y),
      (gameboard_start_x + (i * total_piece_size)),
      (gameboard_start_y + (Rows * total_piece_size)),
      grey,
      0.5);
  }
  for (int i=0;i<Rows;i++) {
    al_draw_line(
      (gameboard_start_x),
      (gameboard_start_y + (i * total_piece_size)),
      (gameboard_start_x + (Cols * total_piece_size)),
      (gameboard_start_y + (i * total_piece_size)),
      grey,
      0.5);
  }
  al_draw_rectangle(
      gameboard_start_x,
      gameboard_start_y,
      gameboard_start_x + ( Cols * total_piece_size),
      gameboard_start_y + ( Rows * total_piece_size),
      white,
      1.0 );
}

// Each piece that has landed on the board gets drawn with a unique color
void drawBoard() {
  for (int y=0;y<Rows;y++) {
    for (int x=0;x<Cols;x++) {
      char tile = board[y][x];
      ALLEGRO_COLOR c;
      switch (tile) {
        case ' ':
          c = black;
          break;
        case 'I':
          c = cyan;
          break;
        case 'L':
          c = orange;
          break;
        case 'T':
          c = purple;
          break;
        case 'J':
          c = blue;
          break;
        case 'S':
          c = green;
          break;
        case 'Z':
          c = red;
          break;
        case 'O':
          c = yellow;
          break;
        default:
          c = white;
          break;
      }
      drawSquare(x, y, c);
    }
  }
}

// prints a tetromino onto the board using row/col coordinates instead of pixels
void printTetromino(Tetromino &t, int col, int row) {
  for (int i=0;i<4;i++) {
    Block &b = t.blocks[i + t.rotation];
    drawSquare(col + b.col, row + b.row, t.color);
  }
}

// prints a tetromino outline onto the board using row/col coordinates
void printOutline(Tetromino &t, int col, int row) {
  for (int i=0;i<4;i++) {
    Block &b = t.blocks[i + t.rotation];
    drawOutline(col + b.col, row + b.row, white);
  }
}

void drawTetromino() {
  printTetromino(curTetromino, curTetromino.col, curTetromino.row);
}


void drawScore(ALLEGRO_FONT* font) {
  drawText("Held Piece", Cols + 1.5, 1, font, white);
  if (held)
    printTetromino(heldTetromino, Cols + 2.5, 2);
  drawText("Next Piece", Cols + 1.5, 7, font, white);
  printTetromino(nextTetromino, Cols + 2.5, 8);
  drawText("Score", Cols + 1.5, 13, font, white);
  char outText[20];
  snprintf(outText, 20, "%07d", score);
  drawText(outText, Cols + 2.5, 14, font, white);
  drawText("Level", Cols + 1.5, 15, font, white);
  snprintf(outText, 20, "%d", level);
  drawText(outText, Cols + 2.5, 16, font, white);
  drawText("Lines Cleared", Cols + 1.5, 17, font, white);
  snprintf(outText, 20, "%d", clearedRows);
  drawText(outText, Cols + 2.5, 18, font, white);
  drawBoxOutline(Cols + .5, 0.0, 9, 6.25, 1.0, white);
  drawBoxOutline(Cols + .5, 6.5, 9, 5.25, 1.0, white);
  drawBoxOutline(Cols + .5, 12, 9, 8, 1.0, white);

}
// draw stats about AI
void drawAIStats(ALLEGRO_FONT* font) {
  char outText[20];
  drawText("AI Gen:", (Cols/2) - 5, -1, font, white);
  snprintf(outText, 20, "%d", generation);
  drawText(outText, (Cols / 2) - 1, -1, font, white);
  drawText("Member: ", (Cols / 2) + 1, -1, font, white);
  snprintf(outText, 20, "%d", genIter);
  drawText(outText, (Cols / 2) + 5, -1, font, white);
  drawText("Fit: ", (Cols / 2) + 7, -1, font, white);
  snprintf(outText, 20, "%0.2f", population[genIter].fitness);
  drawText(outText, (Cols / 2) + 9, -1, font, white);
  drawText("Neuron: ", (Cols / 2) - 5, 21, font, white);
  snprintf(outText, 20, "%c + %s", curTetrominoShape, currentNeuron.boardState.c_str());
  drawText(outText, (Cols / 2) - 1, 21, font, white);
}

void drawPaused(ALLEGRO_FONT* font) {
  if (!paused)
    return;
  drawBox((Cols/3)-1, (Rows/2)-1, 10, 3, black);
  drawBoxOutline((Cols/3)-1, (Rows/2)-1, 10, 3, 1.0, white);
  drawText("---Paused---", Cols/3, Rows/2, font, white);
}

// Draw a ghost piece showing potential drop location
void drawGhost() {
  int curY = curTetromino.row;
  while (!Collision(curTetromino, curY, curTetromino.col)) {
    curY++;
  }
  curY--;
  // Draw mostly transparent ghost block
  printOutline(curTetromino, curTetromino.col, curY);
}

// Draw a highlighted block that our AI is trying to reach
void drawHighlight() {
  printTetromino(highlight, highlight.col, highlight.row);
}



void GameOver(ALLEGRO_FONT* font) {
  
  // Draw Game Over across screen
  drawBox((Cols/3), (Rows/2)-1, 12, 6, black);
  drawBoxOutline((Cols/3), (Rows/2)-1, 12, 6, 1.0, white);
  drawText("Game Over", (Cols/2) + 1, Rows/2, font, white);
  char outText[20];
  snprintf(outText, 20, "%d", score);
  drawText("Final Score:", (Cols/2), (Rows/2)+1, font, white);
  drawText(outText, (Cols/2) + 3, (Rows/2)+2, font, white);
  drawText("Press [ESC] to exit", (Cols/2) - 1.25, (Rows/2) + 4, font, white);
}


// Draw each element to the screen
void DrawStuff(ALLEGRO_FONT* font) {
  drawBoard();
  drawGrid();
  drawGhost();
  drawHighlight();
  drawTetromino();
  drawScore(font);
  drawPaused(font);
  if (using_ai) {
    drawAIStats(font);
  }
}

// perform functions move curTetromino around the game board
// while receiving boolean feedback whether the action was
// successfully performed
bool performMoveDown(ALLEGRO_AUDIO_STREAM* music, ALLEGRO_SAMPLE* line_clear) {
    MoveTetrominoDown();
  if (Collision(curTetromino, curTetromino.row, curTetromino.col)) {
    MoveTetrominoUp();
    if (curTetromino.row <= 0) {
      return false;
    }
    CopyToBoard();
    RemoveCompletedRows(music, line_clear);
    NewTetromino();
    assignNewNeuron();
  }
  return true;
}

bool performManualMoveDown(ALLEGRO_AUDIO_STREAM* music, ALLEGRO_SAMPLE* line_clear) {
  MoveTetrominoDown();
  if (Collision(curTetromino, curTetromino.row, curTetromino.col)) {
    MoveTetrominoUp();
    if (curTetromino.row <= 0) {
      return false;
    }
    CopyToBoard();
    RemoveCompletedRows(music, line_clear);
    NewTetromino();
    assignNewNeuron();
  }
  score++;
  return true;
}


bool performDrop() {
  DropTetromino();
  advanceLogic = true;
  return true;
}

bool performMoveLeft() {
  MoveTetrominoLeft();
  if (Collision(curTetromino, curTetromino.row, curTetromino.col)) {
    MoveTetrominoRight();
    return false;
  }
  return true;
}

bool performMoveRight() {
  MoveTetrominoRight();
  if (Collision(curTetromino, curTetromino.row, curTetromino.col)) {
    MoveTetrominoLeft();
    return false;
  }
  return true;
}

bool performRotate(Tetromino &t, int y, int x) {
  RotateTetromino(t);
  // Rotated against a solid surface, try to kick off of it
  if (Collision(t, y, x)) {
    // Wall kicks
    if (!Collision(t, y, x+1)) {
      return performMoveRight();
    } else if (!Collision(t, y, x-1)) {
      return performMoveLeft();
    } else if (!Collision(t, y+1, x)) {
      MoveTetrominoDown();
      return true;
    }
    // Rotated into a solid surface and can't find a way to 
    // kick off of it, rotate back to starting position and 
    // return false rotate condition
    RotateTetromino(t);
    RotateTetromino(t);
    RotateTetromino(t);
    return false;
  }
  return true;
}

bool performHold() {
  return HoldTetromino();
}

int countHolesinColumn(int col) {
  int holes = 0;
  for (int i=Rows;i>1;i--) {
    if (board[i][col] == ' ' && board[i-1][col] != ' ') {
      holes++;
    }
  }
  return holes;
}


void initialize_AI() {
  std::cout << "First time AI init\n";
  // current gen
  generation = 0;
  genIter = 0;
  actionIter = 0;
 
  for (int i=0;i<POPULATION_SIZE;i++) {
    std::vector<Neuron> brain = std::vector<Neuron>();
    Individual *temp = new Individual(brain);
    population.push_back(*temp);
  }
  currentNeuron = population[genIter].findNeuron(curTetrominoShape, boardState);
}


// Begin our AI function
void AI_play(int curTetrominoInt, std::string boardState, ALLEGRO_AUDIO_STREAM* music, ALLEGRO_SAMPLE* line_clear) {
  // Neuron neuron = population[genIter].findNeuron(curTetrominoInt, boardState);
  char move = currentNeuron.sequence[actionIter++];
  // printf("Using neuron (%d, %s)\n", neuron.tetrominoInt, neuron.boardState.c_str());
  switch (move) {
    case 'w':
      performRotate(curTetromino, curTetromino.row, curTetromino.col);
      break;
    case 's':
      performManualMoveDown(music, line_clear);
      break;
    case 'a':
      performMoveLeft();
      break;
    case 'd':
      performMoveRight();
      break;
    case 'e':
      performHold();
      break;
    case 'z':
      performDrop();
      break;
  }
  if (actionIter >= (GENOME_SIZE - 1))
    actionIter = 0;
}

// Called when a piece lands to determine fitness of Neuron
void calcNeuronFitness() {
  // AI signal to reset actionIter upon piece landing
  actionIter = 0;
  currentNeuron.calcFitness(score, prevScore);
  prevScore = score;
}


void assignNewNeuron() {
  if (using_ai) {
    // we don't want to store useless neurons
    if (currentNeuron.fitness > 0) {
      population[genIter].brain.push_back(currentNeuron);
    }
    currentNeuron = population[genIter].findNeuron(curTetrominoShape, boardState);
  }
}

void AI_create_generation() {
  actionIter = 0;
  // sort population in increasing order of fitness
  sort(population.begin(), population.end(), std::greater<Individual>());

  for (int i=0;i<population.size();i++) {
    printf("Sorted population members: %d: %f\n", i, population[i].fitness);
  }

  // Generate new offspring for new generation
  std::vector<Individual> new_generation;

  // Perform Elitism, take 10% of fittest population
  // onto the next generation
  int s = POPULATION_SIZE * 0.1;
  for (int i=0;i<s;i++) {
    std::cout << "Pushing member " << i << " onto new gen, fitness: " << population[i].fitness << '\n';
    new_generation.push_back(population[i]);
  }


  // From 50% of fittest population, Individuals will 
  // mate to produce more offspring for next generation
  s = POPULATION_SIZE * 0.5;
  for (int i=0;i<(POPULATION_SIZE*0.9);i++) {
    int len = population.size();
    int r1 = random_num(0, s);
    int r2 = random_num(0, s);
    // make sure we don't have self-mating
    while (r1 == r2) {
      r2 = random_num(0,2);
    }
    printf("%d mated with %d\n", r1, r2);
    Individual parent1 = population[r1];
    Individual parent2 = population[r2];
    Individual offspring = parent1.mate(parent2);
    new_generation.push_back(offspring);
  }

  population = new_generation;
  printf("Generation: %d\tFitness: %0.2f\tNeurons: %lu\n", generation, population[0].fitness, population[0].brain.size());

  generation++;

}
void AI_next_iteration() {
  if (genIter++ >= (POPULATION_SIZE - 1)) {
    genIter = 0;
    // we tested all of our population, time to pick the best and make a new one
    AI_create_generation();
  }
}


int real_main(int argc, char** argv) {
  // initialize game-related variables
  // Shuffle Tet bag and get first Tetromino
  Init();

  must_init(al_init(), "Allegro");

  must_init(al_install_keyboard(), "keyboard");

  must_init(al_install_mouse(), "mouse");

  // BRING THE JAMS
  must_init(al_install_audio(), "audio");
  must_init(al_init_acodec_addon(), "audio codecs");
  must_init(al_reserve_samples(16), "reserve samples");

  /* Initialize global Allegro structures */

  /* Sound Effects */
  ALLEGRO_SAMPLE* bump = al_load_sample("assets/sounds/bump.wav");
  must_init(bump, "bump");
  ALLEGRO_SAMPLE* level_up = al_load_sample("assets/sounds/level_up.wav");
  must_init(level_up, "level_up");
  ALLEGRO_SAMPLE* line_clear = al_load_sample("assets/sounds/line_clear.wav");
  must_init(line_clear, "line_clear");
  ALLEGRO_SAMPLE* metal_clang = al_load_sample("assets/sounds/metal_clang.wav");
  must_init(metal_clang, "metal_clang");
  ALLEGRO_SAMPLE* tetromino_hold = al_load_sample("assets/sounds/tetromino_hold.wav");
  must_init(tetromino_hold, "tetromino_hold");
  // ALLEGRO_SAMPLE* tetromino_land = al_load_sample("assets/sounds/tetromino_land.wav");
  // must_init(tetromino_land, "tetromino_land");
  ALLEGRO_SAMPLE* tetromino_rotate = al_load_sample("assets/sounds/tetromino_rotate.wav");
  must_init(tetromino_rotate, "tetromino_rotate");

  /* Music Tracks */
  ALLEGRO_AUDIO_STREAM* music = al_load_audio_stream("assets/music/Electro_-Nitro-Fun-New-Game-_Monstercat-Release_.ogg", 2, 2048);
  must_init(music, "music");
  al_set_audio_stream_playmode(music, ALLEGRO_PLAYMODE_LOOP);
  al_attach_audio_stream_to_mixer(music, al_get_default_mixer());
  al_set_audio_stream_gain(music, (volume/2));

  /* Timers */
  ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
  // set our FPS target
  must_init(timer, "timer");
  ALLEGRO_TIMER* tick_timer = al_create_timer(1.0);
  // Timer to control fall speed
  must_init(tick_timer, "tick_timer");
  ALLEGRO_TIMER* ai_timer = al_create_timer(1.0 / ai_timer_mps);
  // Timer to control the action speed of the AI player
  // 1.0 / 30.0 = 30 moves per second
  must_init(ai_timer, "ai_timer");

  /* Allegro System */
  ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
  must_init(queue, "queue");

  /* Display options must be set prior to calling al_create_display() */
  al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
  al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
  al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

  ALLEGRO_DISPLAY* disp = al_create_display(window_max_x, window_max_y);
  must_init(disp, "display");

  al_set_window_title(disp, "Not Tetris");

  must_init(al_init_font_addon(), "font addon");
  must_init(al_init_ttf_addon(), "ttf addon");
  ALLEGRO_FONT* font = al_load_ttf_font("assets/prstart.ttf", 15, 0);
  must_init(font, "font");

  must_init(al_init_image_addon(), "image addon");

  must_init(al_init_primitives_addon(), "primitives");

  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_display_event_source(disp));
  al_register_event_source(queue, al_get_timer_event_source(timer));
  al_register_event_source(queue, al_get_timer_event_source(tick_timer));
  al_register_event_source(queue, al_get_timer_event_source(ai_timer));
  al_register_event_source(queue, al_get_mouse_event_source());


  advanceLogic = true;
  redraw = true;
  bool done = false;
  bool gameover = false;
  paused = false;
  bool ai_move = false;
  ALLEGRO_EVENT event;

  al_grab_mouse(disp);

  al_start_timer(timer);
  al_start_timer(tick_timer);
  al_start_timer(ai_timer);
  while (1) {
    al_wait_for_event(queue, &event);

    switch (event.type) {
      case ALLEGRO_EVENT_TIMER:
        if (paused || gameover)
          break;
        // Fall speed timer
        if (event.timer.source == tick_timer) {
          advanceLogic = true;
        }
        if (event.timer.source == ai_timer && using_ai) {
          ai_move = true;
        }
        if (advanceLogic) {
          // game tick logic goes here, pieces drop at fixed rate
          advanceLogic = false;
          // Failed to move piece down at top row, game lost
          if (!performMoveDown(music, line_clear)) {
            gameover = true;
            redraw = true;
            break;
          }
        }
        // AI timer
        if (ai_move) {
          ai_move = false;
          AI_play(curTetrominoShape, boardState, music, line_clear);
        }
        redraw = true;   
        break;
      case ALLEGRO_EVENT_MOUSE_AXES:
        // do mouse control stuff here
        break;
      case ALLEGRO_EVENT_KEY_CHAR:
        switch (event.keyboard.keycode) {
          case ALLEGRO_KEY_W:
            if (paused || gameover)
              break;
            if (performRotate(curTetromino, curTetromino.row, curTetromino.col)) {
              // play rotate sound to indicate successful rotate
              al_play_sample(tetromino_rotate, volume, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            } else {
              // play bump sound to indicate failed rotate
              al_play_sample(bump, volume, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
          break;
          case ALLEGRO_KEY_S:
            if (paused || gameover)
              break;
            performManualMoveDown(music, line_clear);
          break;
          case ALLEGRO_KEY_SPACE:
            if (paused || gameover)
              break;
            performDrop();
          break;
          case ALLEGRO_KEY_A:
            if (paused || gameover)
              break;
            performMoveLeft();
          break;
          case ALLEGRO_KEY_D:
            if (paused || gameover)
              break;
            performMoveRight();
          break;
          case ALLEGRO_KEY_E:
            if (paused || gameover)
              break;
            if (performHold()) {
              // play hold sound to indicate successful swap
              al_play_sample(tetromino_hold, volume, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            } else {
              // play metal_clang sound to indicate failed swap
              al_play_sample(metal_clang, volume, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
          break;
          case ALLEGRO_KEY_Q:
          case ALLEGRO_KEY_ESCAPE:
            done = true;
          break;
          case ALLEGRO_KEY_P:
            Pause();
            al_set_audio_stream_playing(music, !paused);
            redraw = true;
          break;
          case ALLEGRO_KEY_M:
            if (volume) {
              volume = 0.0;
            } else {
              volume = 1.0;
            }
            // set music volume to half game volume, or mute completely
            al_set_audio_stream_gain(music, ((volume>0) ? (volume/2) : (volume)));
          break;
          case ALLEGRO_KEY_MINUS:
            ai_timer_mps -= 1;
            if (ai_timer_mps < 1.0) {
              ai_timer_mps = 1.0;
            }
            printf("AI Speed: %0.2f\n", ai_timer_mps);
            ai_timer = al_create_timer(1.0 / ai_timer_mps);
            al_start_timer(ai_timer);
          break;
          case ALLEGRO_KEY_EQUALS:
            ai_timer_mps += 1.0;
            if (ai_timer_mps > 60) {
              ai_timer_mps = 60.0;
            }
            printf("AI Speed: %0.2f\n", ai_timer_mps);
            ai_timer = al_create_timer(1.0 / ai_timer_mps);
            al_start_timer(ai_timer);
          break;
        }
        break;
      case ALLEGRO_EVENT_DISPLAY_CLOSE:
        done = true;
        break;
    }
    // Broken out of event switch
    if (done)
      break;
    
    if (redraw && al_is_event_queue_empty(queue)) {
      redraw = false;
      al_clear_to_color(black);

      DrawStuff(font);
      
      // Game ended, print loss screen if a human is playing
      if (gameover) {
        if (!using_ai) {
          GameOver(font);
        } else {
          done = true;
        }
      }
        

      al_flip_display();
    }

  }

  al_destroy_font(font);
  al_destroy_display(disp);
  al_destroy_timer(timer);
  al_destroy_timer(tick_timer);
  al_destroy_timer(ai_timer);
  al_destroy_event_queue(queue);
  al_destroy_sample(bump);
  al_destroy_sample(level_up);
  al_destroy_sample(line_clear);
  al_destroy_sample(metal_clang);
  al_destroy_sample(tetromino_hold);
  // al_destroy_sample(tetromino_land);
  al_destroy_sample(tetromino_rotate);
  al_destroy_audio_stream(music);

  if (using_ai) {
    std::cout << "AI running generation: " << generation << '\n';
    // Score the individual on how well it performed.

    // set the fitness for an individual equal to the sum of its neurons' fitness
    population[genIter].setFitness(score);

    for (int i=0;i<population.size();i++) {
      printf("Population member: %d\tFitness: %f\tNeurons: %lu\n", i, population[i].fitness, population[i].brain.size());
    }
    // Advance AI generation
    AI_next_iteration();
    // reboot forever if AI is training
    real_main(argc, argv);
  }
  return 0;
}

int main(int argc, char** argv) {
  return al_run_main(argc, argv, real_main);
}
