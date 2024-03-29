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
#include <sqlite3.h>

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

int main(int argc, char** argv);

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
bool using_ai = true;

sqlite3 *db;

const int Rows = 20;
const int Cols = 10;
const int border_width = 1;
const double frames = 1.0 / 60.0;
double level_timer = 2.0;
double volume = 0.0;
double ai_timer_mps = 120.0;

unsigned int time_ui = static_cast<unsigned int>( time(NULL) );


const float piece_size = 25.0;
const float piece_buffer = 2.0;
const float total_piece_size = piece_size + piece_buffer;

const int gameboard_width = (total_piece_size * Cols);
const int gameboard_height = (total_piece_size * Rows);

const int window_max_x = gameboard_width + 350;
const int window_max_y = gameboard_height + 150;

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

ALLEGRO_FONT* font;
ALLEGRO_AUDIO_STREAM* music;
ALLEGRO_SAMPLE* line_clear;

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
bool performMoveDown();
void MoveTetrominoDown();
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
int holes;
int prevHoles;
int height;
int prevHeight;
int lowestColumnHeight;
int prevLowestColumnHeight;
int brain_nodes;
int prevDistance;
int level;
int clearedRows;
bool gameover;
bool paused;
bool redraw;
bool advanceLogic;
Tetromino highlight;
int tetBag[7] = {0, 1, 2, 3, 4, 5, 6};
int tetBagIter = 0;

void initialize_AI();
float calcNeuronFitness();
std::string getBoardState();
std::string getBoardSilhouetteState();
void storeMoveValue(const std::string &state, char move, float value, std::string &nextState);
void createMap(std::map<std::string, char> *um);
void createExtendedMap(std::map<std::string, char> *ue);
void storeCachedMoves();
int getHeightofCol(int col);

// Cache of board States and moves performed that led up to a piece being dropped.
// This chain of moves all share the final score once the piece is landed.
//std::vector<std::pair<std::string, char>> cachedMoves;
std::unordered_map<std::string, char> cachedMoves;

// The 50-bit hex representation of the 200-bit binary board passed to the AI
std::string boardState;

// declare our AI variables
std::vector<Individual> population;
int generation;
int genIter;
int actionIter;
int bestFit;
std::vector<int> highScores;

// create map between binary and its
// equivalent hex code
std::map<std::string, char> bin_hex_map;
std::map<std::string, char> ext_hex_map;
   

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


bool EntireTetrominoIsVisible(Tetromino &t) {
  for (int i=0;i<4;i++) {
    Block &b = t.blocks[i + t.rotation];
    if (t.row + b.row < 0 || t.row + b.row >= Rows || t.col + b.col < 0 || t.col + b.col >= Cols) {
      return false;
    }
  }
  return true;
}

// shifts nextTet into curTet and pulls a new nextTet
void NewTetromino() {
  performedSwap = false;
  curTetromino = nextTetromino;
  curTetrominoShape = curTetromino.c; // 
  curTetromino.row = -4;
  // slide tetromino down until fully visible before player can move it
  while (!EntireTetrominoIsVisible(curTetromino)) {
    MoveTetrominoDown();
  }
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
  //tetBagIter = 0;
  score = 0;
  prevScore = 0;
  holes = 0;
  prevHoles = 0;
  height = 0;
  prevHeight = 0;
  prevDistance = 0;
  lowestColumnHeight = 0;
  prevLowestColumnHeight = 0;
  level = 1;
  gameover = false;
  clearedRows = 0;
  held = false;
  paused = false;
  redraw = true;
  advanceLogic = false;
  performedSwap = false;

  advanceLogic = true;
  redraw = true;
  ClearBoard();
  if (firstRun) {
    initialize_AI();
    firstRun = false;
    bestFit = 0;
    brain_nodes = 0;
    // create our bin to hex map
    createMap(&bin_hex_map);
    createExtendedMap(&ext_hex_map);
  }
  //if (using_ai) {
    // not currently storing cached moves
    //storeCachedMoves();
    // retrieve first board state so movement can begin
    //boardState = getBoardState();
  //}
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

// Function to create map that converts from 200-bit binary
// string representation of the board and its equivalent 50-bit hexadecimal
void createMap(std::map<std::string, char> *um) {
    (*um)["0000"] = '0';
    (*um)["0001"] = '1';
    (*um)["0010"] = '2';
    (*um)["0011"] = '3';
    (*um)["0100"] = '4';
    (*um)["0101"] = '5';
    (*um)["0110"] = '6';
    (*um)["0111"] = '7';
    (*um)["1000"] = '8';
    (*um)["1001"] = '9';
    (*um)["1010"] = 'A';
    (*um)["1011"] = 'B';
    (*um)["1100"] = 'C';
    (*um)["1101"] = 'D';
    (*um)["1110"] = 'E';
    (*um)["1111"] = 'F';
} 

// function to create map that converts 2 digit string into 1 digit hex value
void createExtendedMap(std::map<std::string, char> *ue) {
  (*ue)["00"] = '0';
  (*ue)["01"] = '1';
  (*ue)["02"] = '2';
  (*ue)["03"] = '3';
  (*ue)["04"] = '4';
  (*ue)["05"] = '5';
  (*ue)["06"] = '6';
  (*ue)["07"] = '7';
  (*ue)["08"] = '8';
  (*ue)["09"] = '9';
  (*ue)["10"] = 'A';
  (*ue)["11"] = 'B';
  (*ue)["12"] = 'C';
  (*ue)["13"] = 'D';
  (*ue)["14"] = 'E';
  (*ue)["15"] = 'F';
  (*ue)["16"] = 'G';
  (*ue)["17"] = 'H';
  (*ue)["18"] = 'I';
  (*ue)["19"] = 'J';
  (*ue)["20"] = 'K';
}

// function to retrieve the board silhouette meaning only the top layer, to save on 
// memory and number of possible board states
std::string getBoardSilhouetteState() {
  // we need to know what the board looks like, and where the player tetromino is
  std::string boardHex = "";
  std::string boardState = "";
  for (int i=0;i<Cols;i++) {
    int colHeight = getHeightofCol(i);
    // append leading 0 to numbers below 10
    if (colHeight < 10) {
      boardState += '0';
    }
    boardState += std::to_string(colHeight);
  }
  for (int i=0;i<4;i++) {
    Block &b = curTetromino.blocks[i + curTetromino.rotation];
    int row = curTetromino.row + b.row;
    int col = curTetromino.col + b.col;
    if (row < 10) {
      boardState += '0';
    }
    boardState += std::to_string(curTetromino.row + b.row);
    if (col < 10) {
      boardState += '0';
    }
    boardState += std::to_string(curTetromino.col + b.col);
  }
  for (int i=0;i<boardState.size()-1;i+=2) {
    boardHex += ext_hex_map[boardState.substr(i, 2)];
  }

  //printf("boardState: %s (%lu) boardHex: %s (%lu)\n", boardState.c_str(), boardState.length(), boardHex.c_str(), boardHex.length());
  return boardHex;
}

std::string getBoardState() {
  std::string boardBinary = "";
  std::string boardState = "";
  bool found = false;
  for (int i=0;i<Rows;i++) {
    for (int j=0;j<Cols;j++) {
      found = false;
      for (int a=0;a<4;a++) {
        Block &b = curTetromino.blocks[a + curTetromino.rotation];
        if ((curTetromino.row + b.row) == i && (curTetromino.col + b.col) == j) {
          found = true;
          break;
        }
      }
      // set binary bit if the board contains a piece, or if the current tetromino piece
      // was found at this location
      boardBinary += ((board[i][j] == ' ' && !found) ? '0' : '1');
    }
  }
  // debug board state output to console
  //printf("\nBoard state:");
  //for (int i=0;i<boardBinary.length();i++) {
  //  if (i%Cols==0) printf("\n");
  //  printf("%c", boardBinary.at(i));
  //}
  //printf("\n");
  // perform binary to hex encoding to save memory
  for (int i=0;i<boardBinary.size()-3;i+=4) {
    boardState += bin_hex_map[boardBinary.substr(i, 4)];
  }
  // printf("boardBinary: %s (%lu) boardState: %s (%lu)\n", boardBinary.c_str(), boardBinary.length(), boardState.c_str(), boardState.length());
  return boardState;
}

// Copies the current Tetromino onto the board
void CopyToBoard() {

  // copy tetromino onto the board
  for (int i=0;i<4;i++) {
    Block &b = curTetromino.blocks[i + curTetromino.rotation];
    board[curTetromino.row + b.row][curTetromino.col + b.col] = curTetromino.c;
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
bool DropTetromino() {
  int droppedRows = 0;
  while (performMoveDown()) {
    droppedRows++;
  }
  if (droppedRows) {
    return true;
  }
  return false;
}

// Removes any completed rows
void RemoveCompletedRows() {
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
      al_set_audio_stream_speed(music, 1+(level*((float)1/30)));
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

  // we don't update fitness after a piece lands anymore,
  // we update fitness after each invididual movement and recalculate state
  // update Neuron fitness and boardState
  //if (using_ai) {
  //  calcNeuronFitness();
  //  getBoardState();
  //}

}

int gameboard_start_x = 50;
int gameboard_start_y = 100;

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
      char out[10];
      snprintf(out, 10, "%d", x);
      drawText(out, x, -1, font, white);
    }
    char out[10];
    snprintf(out, 10, "%d", y);
    drawText(out, -1, y, font, white);
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


void drawScore() {
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
void drawAIStats() {
  char outText[20];
  drawText("--AI Learning--", ((float)Cols/2), -3, font, white);
  drawText("Gen:", ((float)Cols/2) - 5, -2, font, white);
  snprintf(outText, 20, "%d", generation+1);
  drawText(outText, ((float)Cols / 2) - 2.5, -2, font, white);
  drawText("Nodes: ", ((float)Cols / 2) + 0, -2, font, white);
  snprintf(outText, 20, "%d", brain_nodes);
  drawText(outText, ((float)Cols / 2) + 3.5, -2, font, white);
  drawText("HiScore: ", ((float)Cols / 2) + 7, -2, font, white);
  snprintf(outText, 20, "%d", bestFit);
  drawText(outText, ((float)Cols / 2) + 11.5, -2, font, white);
  // drawText("Neuron: ", (Cols / 2) - 5, 20.5, font, white);
  // snprintf(outText, 20, "%c + %s", curTetrominoShape, currentNeuron.boardState.c_str());
  // drawText(outText, (Cols / 2) - 1, 20.5, font, white);
}

void drawPaused() {
  if (!paused)
    return;
  drawBox(((float)Cols/3)+7, ((float)Rows/2), 10, 3, black);
  drawBoxOutline(((float)Cols/3)+7, ((float)Rows/2), 10, 3, 1.0, white);
  drawText("---Paused---", (float)Cols/3+8, (float)Rows/2+1, font, white);
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



void GameOver() {
  
  // Draw Game Over across screen
  drawBox(((float)Cols/3), ((float)Rows/2)-1, 12, 6, black);
  drawBoxOutline(((float)Cols/3), ((float)Rows/2)-1, 12, 6, 1.0, white);
  drawText("Game Over", ((float)Cols/2) + 1, (float)Rows/2, font, white);
  char outText[20];
  snprintf(outText, 20, "%d", score);
  drawText("Final Score:", ((float)Cols/2), ((float)Rows/2)+1, font, white);
  drawText(outText, ((float)Cols/2) + 3, ((float)Rows/2)+2, font, white);
  drawText("Press [ESC] to exit", ((float)Cols/2) - 1.25, ((float)Rows/2) + 4, font, white);
}


// Draw each element to the screen
void DrawStuff() {
  drawBoard();
  drawGrid();
  drawGhost();
  drawHighlight();
  drawTetromino();
  drawScore();
  drawPaused();
  if (using_ai) {
    drawAIStats();
  }
}

// perform functions move curTetromino around the game board
// while receiving boolean feedback whether the action was
// successfully performed
bool performMoveDown() {
    MoveTetrominoDown();
  if (Collision(curTetromino, curTetromino.row, curTetromino.col)) {
    MoveTetrominoUp();
    CopyToBoard();
    RemoveCompletedRows();
    //assignNewNeuron();
    if (using_ai) {
      // store any more we cached
      storeCachedMoves();
    }
    // this is after fitness calculation so we can determine the placement of the
    // current tetromino before getting a new one
    NewTetromino();
    return false;
  }
  score++;
  return true;
}

bool performDrop() {
  advanceLogic = true;
  return DropTetromino();
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
  // don't count top 4 rows for holes
  int i=0;
  while (i<Rows) {
    // count each time we find a block above an empty space
    if (board[i][col] != ' ' && board[i+1][col] == ' ') {
      holes++;
    }
    i++;
  }
  return holes;
}

int countTotalHoles() {
  int holes = 0;
  for (int i=0;i<Cols;i++) {
    holes += countHolesinColumn(i);
  }
  return holes;
}

// returns height of column in terms of Rows-1-row_height
// So a column with 0 blocks in it will have row height of
// 20 - 20 = 0
int getHeightofCol(int col) {
  for (int row=0;row<Rows;row++) {
    if (board[row][col] != ' ') {
       return Rows-row;
    }
  }
  return 0;
}

// returns highest overall block currently on the board
// in terms of rows away from the bottom,
// e.g. a row with 3 blocks from the bottom would be
// 20 - 17 = 3
int getMaxHeight() {
  int max = 0;
  for (int j=0;j<Cols;j++) {
    max = std::fmaxf(max, getHeightofCol(j));
  }
  return max;
}

int getLowestColumn() {
  int lowest = Rows-1;
  int lowestCol = 0;
  for (int i=0;i<Cols;i++) {
    int colheight = getHeightofCol(i);
    if (colheight < lowest) {
      lowest = colheight;
      lowestCol = i;
    }
  }
  return lowestCol;
}

// returns height of lowest column on the board
int getLowestColumnHeight() {
  int lowest = Rows;
  for (int i=0;i<Cols;i++) {
    lowest = std::fminf(lowest, getHeightofCol(i));
  }
  return lowest;
}

int getTetrominoLowestPoint(Tetromino curTetromino) {
  int lowest = 0;
  for (int i=0;i<4;i++) {
    Block &b = curTetromino.blocks[i + curTetromino.rotation];
    // max is used here because as row increases, height decreases
    lowest = std::fmaxf(lowest, curTetromino.row + b.row);
  }
  return Rows-1-lowest;
}

std::string dbFile = "brain.db";

void initialize_AI() {
  std::cout << "First time AI init\n";
  // current gen
  generation = 0;
  genIter = 0;
  actionIter = 0;
  float learningRate = 0.1f;
  float discountFactor = 0.9f;
  float explorationRate = 1.0f;
  float minExplorationRate = 0.01f;
  float explorationDecayRate = 0.99f;

  highScores = std::vector<int>();
  int rc = sqlite3_open(dbFile.c_str(), &db);
  if (rc) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
  }
  Individual temp = Individual(db, learningRate, discountFactor, explorationRate, minExplorationRate, explorationDecayRate);
  population.push_back(temp);
 
  //for (int i=0;i<POPULATION_SIZE;i++) {
  //  std::unordered_map<std::string, std::vector<float>> brain;
  //  brain.clear();
  //  Individual temp = Individual(brain);
  //  population.push_back(temp);
  //}
}

char currentMove;

// Begin our AI function
void AI_play() {
  // Neuron neuron = population[genIter].findNeuron(curTetrominoInt, boardState);
  // chance is the odds of mutating to perform a random action instead of the best action we have stored
  //int chance = std::fmaxf(50, 1000-generation);
  // 50 / 1000 = 5% mutation rate
  //int chance = 50;
  // move value must cross this threshold, or we keep trying random moves
  //int threshold = 20;
  //retrieve current board state for the AI to view
  boardState = getBoardState();
  // retrieve the best action for the current board state
  currentMove = population[genIter].getBestAction(boardState);
  // cache the move that was performed here
  cachedMoves[boardState] = currentMove;
  //cachedMoves.push_back({boardState, currentMove});
  //printf("board[%s][%c]\n", boardState.c_str(), currentMove);
  switch (currentMove) {
    case 'w':
      performRotate(curTetromino, curTetromino.row, curTetromino.col);
      break;
    case 's':
      performMoveDown();
      break;
    case 'a':
      performMoveLeft();
      break;
    case 'd':
      performMoveRight();
      break;
    case 'z':
      performDrop();
      break;
    // currently disabled hold
    case 'e':
      performHold();
      break;
  }
  //int currentFitness = calcNeuronFitness();
  //storeMoveValue(currentFitness);
  // update board state after a move is performed
  //std::string nextState = getBoardState();
  // very simple reward state just means if our score went up, then we get a reward
  // this is not very ideal because it only rewards moving down, but we'll see how
  // it goes
}


// Store every move that was performed that led up to a piece being landed.
// Here we calculate the score difference and give that score to every 
// move that was performed.
void storeCachedMoves() {
  if (!cachedMoves.empty()) {
    int height = getMaxHeight();
    //printf("Board height after piece placement: %d\n", height);

    float pieceValue = calcNeuronFitness();
    std::string nextState = getBoardState();
    //printf("Storing %lu moves with score %d\n", cachedMoves.size(), pieceValue);
    for (const auto &it : cachedMoves) {
      storeMoveValue(it.first, it.second, pieceValue, nextState);
    }
    cachedMoves.clear();

    brain_nodes = population[0].getNodeCount();
    population[genIter].decayExplorationRate();
  }
}

void printCachedMoves() {
  std::string out = "";
  for (const auto &it : cachedMoves) {
    out += it.second;
  }
  printf("Moves: %s\n", out.c_str());
}

// Called when a piece lands to determine fitness of the performed move
float calcNeuronFitness() {
  // assign weights to determine how highly we reward specific AI actions
  // these are all penalties except for score
  // =========================================
  // ===== FUNCTION WEIGHT DECLARATIONS  =====
  // =========================================
  float holes_weight = 1.0f;
  float height_weight = 1.0f;
  float score_weight = 1.0f;
  float moves_weight = 1.0f;

  // get the baseline values of each weight
  int holes = countTotalHoles();
  int lowestPlacedBlock = getTetrominoLowestPoint(curTetromino);
  // check how close the piece was to the lowest possible point it could have been dropped
  int height = lowestPlacedBlock - prevLowestColumnHeight;
  int moves = cachedMoves.size();
  int new_holes = holes - prevHoles;
  //int new_distance = prevDistance - distanceFromLowestCol;
  int new_score = (score - prevScore);
  // filling holes gives positive number
  // filledHoles range is -4 <-> 4
  // a great move would be a 4
  // terrible move would be a -4
  // therefore our fitness modifier is: ((filledHoles + 2) / 4)
  // filledHoles
  // -----------
  // -4 = 0.00
  // -3 = 0.25
  // -2 = 0.50
  // -1 = 0.75
  //  0 = 1.00
  //  1 = 1.25
  //  2 = 1.50
  //  3 = 1.75
  //  4 = 2.00
  // losing height gives positive number
  // heightLost range is -4 <-> 4
  // a great move would clear 4 rows
  // a terrible move would clear -4 rows
  // therefore our fitness modifier is: ((heightLost + 2) / 4)
  // -4 = -1.00
  // -3 = -0.75
  // -2 = -0.50
  // -1 = -0.25
  //  0 = 0.00
  //  1 = 0.25
  //  2 = 0.50
  //  3 = 0.75
  //  4 = 1.00


  float holes_value = (3 - new_holes) * holes_weight;
  float height_value = 20 - height * height_weight;
  float score_value = new_score * score_weight;
  // ensure we have at least 1 move to divide by
  float moves_value = moves * moves_weight;

  // add up total value of all parameters
  float total_value = (score_value + holes_value + height_value - moves_value);
  //printf("action taken: %c\n", currentMove);
  printCachedMoves();
  printf("Holes: %d Best H: %d Height: %d Score: %d Moves: %d\n", new_holes, prevLowestColumnHeight, lowestPlacedBlock, new_score, moves);
  printf("HV: %2.2f HV: %2.2f SV: %2.2f MV: %2.2f\n", holes_value, height_value, score_value, -moves_value);
  printf("Total move value: %2.2f\n", total_value);
  //printf("prevScore: %2d prevHoles: %2d prevHeight: %d\n", prevScore, prevHoles, prevHeight);
  //printf("newScore:  %2d newHoles:  %2d newHeight:  %d\n", score, holes, height);
  //printf("distance from lowest column %d: %d\n", lowestColumn, distanceFromLowestColumn);
  //printf("distance modifier: %d\n", distanceModifier);
  //printf("Subscore:  %2d holesMod:%0.2f heightM: %0.2f = %d\n", subScore, holesModifier, heightModifier, newScore);

  // carry over current values to be previous for the next piece
  prevScore = score;
  prevHoles = holes;
  prevHeight = height;
  // assign new lowest column to be used for the next piece
  prevLowestColumnHeight = getLowestColumnHeight();

  return total_value;
}

void storeMoveValue(const std::string &state, char move, float value, std::string &nextState) {
  // we don't want to store useless neurons
  // store current Neuron
  population[genIter].storeMoveValue(state, move, value, nextState);
}

/*
void AI_create_generation() {
  actionIter = 0;
  // sort population in increasing order of fitness
  sort(population.begin(), population.end(), std::greater<Individual>());

  for (int i=0;i<population.size();i++) {
    printf("Sorted population members: %d: %d\n", i, population[i].fitness);
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
      r2 = random_num(0, s);
    }
    // printf("%d mated with %d\n", r1, r2);
    Individual parent1 = population[r1];
    Individual parent2 = population[r2];
    Individual offspring = parent1.mate(parent2);
    new_generation.push_back(offspring);
  }

  population = new_generation;
  printf("Generation: %d\tFitness: %d\tNeurons: %lu\n", generation+1, population[0].fitness, population[0].brain.size());

  generation++;

}
*/
void AI_next_iteration() {
  if (genIter++ >= (POPULATION_SIZE - 1)) {
    genIter = 0;

    // store high score for this generation
    // only store every 100th generation high score
    if (generation%100==0) {
      highScores.push_back(bestFit);
      printf("High Scores:\n");
      for (int i=0;i<highScores.size();i++) {
        printf("\tGen: %d\tHigh Score: %d\n", i+1, highScores[i]);
      }
    }
    //bestFit = 0;
    // increment the ai generation
    generation++;

    // print high scores

    // we tested all of our population, time to pick the best and make a new one
    //AI_create_generation();
  }
}

void soft_reset() {
  if (using_ai) {
    // Score the individual on how well it performed.

    // set the fitness for an individual equal to the sum of its neurons' fitness
    // edit: we are only considering total score to be fitness rather than sum of its neurons fitness
    //population[genIter].setFitness(score);

    // update best high score
    if (score > bestFit) {
      bestFit = score;
    }

    printf("Generation: %d\tScore: %d\tNeurons: %d\n", generation, score, brain_nodes); 
    //printf("Generation: %d\tScore: %d\n", generation, score);
    // Advance AI generation
    AI_next_iteration();
    // reboot forever if AI is training
    // real_main(argc, argv);
    Init();
  }
}

int real_main(int argc, char** argv) {
  // seed random number generator
  srand(time_ui);
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
  line_clear = al_load_sample("assets/sounds/line_clear.wav");
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
  music = al_load_audio_stream("assets/music/Electro_-Nitro-Fun-New-Game-_Monstercat-Release_.ogg", 2, 2048);
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
  font = al_load_ttf_font("assets/prstart.ttf", 15, 0);
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
          if (!performMoveDown() && curTetromino.row <= 0) {
            gameover = true;
            redraw = true;
            break;
          }
        }
        // AI timer
        if (ai_move) {
          ai_move = false;
          AI_play();
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
            performMoveDown();
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
          case ALLEGRO_KEY_L:
            using_ai = !using_ai;
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

      DrawStuff();
      
      // Game ended, print loss screen if a human is playing
      if (gameover) {
        if (using_ai) {
          soft_reset();
          // done = true;
        } else {
          GameOver();
        }
      }
        

      al_flip_display();
    }

  }


  sqlite3_close(db);
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

  return 0;
}

int main(int argc, char** argv) {
  return al_run_main(argc, argv, real_main);
}
