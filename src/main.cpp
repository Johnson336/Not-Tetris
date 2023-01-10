#include <ncurses.h>
#include <unistd.h>
#include <random>
#include <chrono>
#include <iostream>


const int Rows = 20;
const int Cols = 10;
const int border_width = 1;
const double frames = 1.0 / 60.0;
double level_timer = 2.0;
unsigned char moving_char = '*';


// Represents a single block in a Tetromino
struct Block {
  int row;
  int col;
};

// Represents a Tetromino piece
struct Tetromino {
  Block blocks[16];
  char c;
  int rotation;
};

// The Tetromino shapes
const Tetromino Tetrominoes[7] = {
  {{ {1, 0}, {1, 1}, {1, 2}, {1, 3},
     {0, 2}, {1, 2}, {2, 2}, {3, 2},
     {1, 0}, {1, 1}, {1, 2}, {1, 3},
     {0, 2}, {1, 2}, {2, 2}, {3, 2} }, 'I', 0},
  {{ {1, 0}, {1, 1}, {1, 2}, {2, 1},
     {0, 1}, {1, 0}, {1, 1}, {2, 1},
     {1, 1}, {2, 0}, {2, 1}, {2, 2},
     {0, 1}, {1, 1}, {1, 2}, {2, 1} }, 'T', 0},
  {{ {1, 0}, {1, 1}, {1, 2}, {2, 0},
     {0, 0}, {0, 1}, {1, 1}, {2, 1},
     {1, 2}, {2, 0}, {2, 1}, {2, 2},
     {0, 1}, {1, 1}, {2, 1}, {2, 2} }, 'L', 0},
  {{ {1, 0}, {1, 1}, {1, 2}, {2, 2},
     {0, 1}, {1, 1}, {2, 0}, {2, 1},
     {1, 0}, {2, 0}, {2, 1}, {2, 2},
     {0, 1}, {0, 2}, {1, 1}, {2, 1} }, 'J', 0},
  {{ {1, 1}, {1, 2}, {2, 0}, {2, 1},
     {0, 0}, {1, 0}, {1, 1}, {2, 1},
     {1, 1}, {1, 2}, {2, 0}, {2, 1},
     {0, 0}, {1, 0}, {1, 1}, {2, 1} }, 'S', 0},
  {{ {1, 0}, {1, 1}, {2, 1}, {2, 2},
     {0, 2}, {1, 1}, {1, 2}, {2, 1},
     {1, 0}, {1, 1}, {2, 1}, {2, 2},
     {0, 2}, {1, 1}, {1, 2}, {2, 1} }, 'Z', 0},
  {{ {1, 1}, {1, 2}, {2, 1}, {2, 2},
     {1, 1}, {1, 2}, {2, 1}, {2, 2},
     {1, 1}, {1, 2}, {2, 1}, {2, 2},
     {1, 1}, {1, 2}, {2, 1}, {2, 2} }, 'O', 0},
};

// The board grid
char board[Rows][Cols];

// Details of the current Tetromino
Tetromino nextTetromino;
Tetromino curTetromino;
Tetromino heldTetromino;
bool held = false;
int curRow;
int curCol;
unsigned long long score;
int level;
int clearedRows;
bool paused;

// initialize our redraw timer
std::uint64_t now;
std::uint64_t now_2;


// Clears the board
void ClearBoard() {
  for (int i = 0; i < Rows; i++) {
    for (int j = 0; j < Cols; j++) {
      board[i][j] = ' ';
    }
  }
}

std::uint64_t getTimeNow() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

// Initializes the board and sets up NCurse
void Init() {
  curTetromino = Tetrominoes[rand() % 7];
  nextTetromino = Tetrominoes[rand() % 7];
  score = 0;
  level = 1;
  clearedRows = 0;
  paused = false;
  ClearBoard();
  initscr();
  
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  now = getTimeNow();
  now_2 = getTimeNow();
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

// Clears the screen
void ClearScreen() {
  for (int i = 0; i < Rows; i++) {
    for (int j = 0; j < Cols; j++) {
      mvaddch(i, j, ' ');
    }
  }
}

void PrintTetromino(Tetromino &t, int y, int x, unsigned char c) {
  for (int i = 0;i < 4; i++) {
    Block &b = t.blocks[i + t.rotation];
    mvaddch(y + b.row, x + b.col, c );
  }
}

// Draws the current Tetromino
void DrawTetromino() {
  PrintTetromino(curTetromino, border_width + curRow, border_width + curCol, moving_char);
}

// Draws the playing board
void DrawBoard() {
  for (int i = 0; i < Rows; i++) {
    for (int j = 0; j < Cols; j++) {
      mvaddch(border_width + i, border_width + j, board[i][j]);
    }
  }
}


  // draw Title
void DrawTitle() {

  mvprintw(25, 2, "Definitely Not Tetris");
}

// draw border around the game board
void DrawBorder() {
  // Draw box corners
  mvaddch(0,0, ACS_ULCORNER);
  mvaddch(Rows + 1, 0, ACS_LLCORNER);
  mvaddch(0, Cols + 16, ACS_URCORNER);
  mvaddch(Rows + 1, Cols + 16, ACS_LRCORNER);
  // Draw box lines
  mvhline(0, 1, ACS_HLINE, Cols + 15);
  mvhline(Rows + 1, 1, ACS_HLINE, Cols + 15);
  mvvline(1, 0, ACS_VLINE, Rows);
  mvvline(1, Cols + 1, ACS_VLINE, Rows);
  mvvline(1, Cols + 16, ACS_VLINE, Rows);

  // Draw score lines
  mvhline(7, Cols + 2, ACS_HLINE, 14);
  mvhline(13, Cols + 2, ACS_HLINE, 14);

  // Draw T junctions
  mvaddch(0, Cols + 1, ACS_TTEE);
  mvaddch(Rows + 1, Cols + 1, ACS_BTEE);
  mvaddch(7, Cols + 1, ACS_LTEE);
  mvaddch(7, Cols + 16, ACS_RTEE);
  mvaddch(13, Cols + 1, ACS_LTEE);
  mvaddch(13, Cols + 16, ACS_RTEE);
}

// draw boxes for score, next piece, level, and cleared lines
void DrawScore() {
  int offset_x = 13;
  int offset_y = 2;
  mvprintw(offset_y, offset_x, "Held piece");
  offset_y++;
  if (held) {
    PrintTetromino(heldTetromino, offset_y, offset_x+1, moving_char);
  }
  offset_y+=5;
  mvprintw(offset_y, offset_x, "Next piece");
  offset_y++;
  PrintTetromino(nextTetromino, offset_y, offset_x+1, moving_char);

  offset_y+=5;
  mvprintw(offset_y, offset_x, "Score");
  offset_y++;
  mvprintw(offset_y, offset_x + 1, "%d", score);
  offset_y++;
  mvprintw(offset_y, offset_x, "Level");
  offset_y++;
  mvprintw(offset_y, offset_x + 1, "%d", level);
  offset_y++;
  mvprintw(offset_y, offset_x, "Lines cleared");
  offset_y++;
  mvprintw(offset_y, offset_x + 1, "%d", clearedRows);
}

// Copies the current Tetromino onto the board
void CopyToBoard() {
  for (int i=0;i<4;i++) {
    Block &b = curTetromino.blocks[i + curTetromino.rotation];
    board[curRow + b.row][curCol + b.col] = curTetromino.c;
  }
}

// Rotates the current Tetromino clockwise
void RotateTetromino() {
  // rotations are 0->4->8->12->0
  curTetromino.rotation+=4;
  if (curTetromino.rotation > 12)
    curTetromino.rotation = 0;
}

// Moves the current Tetromino left
void MoveTetrominoLeft() {
  curCol--;
}

// Moves the current Tetromino right
void MoveTetrominoRight() {
  curCol++;
}

// Moves the current Tetromino down
void MoveTetrominoDown() {
  curRow++;
}

void MoveTetrominoUp() {
  curRow--;
}

// Generates a new Tetromino
void NewTetromino() {
  curTetromino = nextTetromino;
  nextTetromino = Tetrominoes[rand() % 7];
  curRow = 0;
  curCol = Cols / 2;
}

// Returns true if the current Tetromino collides with something
bool Collision(Tetromino &t, int y) {
  for (int i = 0; i < 4; i++) {
    Block &b = t.blocks[i + t.rotation];
    // This allows collision checking for current piece and ghost piece
    int row = y + b.row; 
    int col = curCol + b.col;

    // Bounds and overlap checking
    if (row >= Rows || row < 0 || col < 0 || col >= Cols || board[row][col] != ' ') {
      return true;
    }
  }

  return false;
}

void HoldTetromino() {
  // Swap held and current Tetrominos
  if (held) {
    // only swap if the held piece will fit
    if (!Collision(heldTetromino, curRow)) {
      Tetromino temp = heldTetromino;
      heldTetromino = curTetromino;
      curTetromino = temp;
    }
  } else {
    // Put current Tetromino into held and get a new one
    held = true;
    heldTetromino = curTetromino;
    NewTetromino();
  }
}

// Draw a ghost showing potential drop placement location
void DrawGhost() {
  int curY = curRow;
  while (!Collision(curTetromino, curY)) {
    curY++;
  }
  curY--;
  // Draw ghost dots to indicate potential position
  PrintTetromino(curTetromino, border_width + curY, border_width + curCol, '.');
}

// Pause and un-pause the game
void Pause() {
  paused = !paused;
}

// draw Paused status onto screen
void DrawPaused() {
  if (!paused)
    return;
  mvprintw(Rows/2, 1, "--Paused--");
}

// Drops the current Tetromino down until it collides with something
void DropTetromino() {
  int droppedRows = 0;
  while (!Collision(curTetromino, curRow)) {
    droppedRows++;
    MoveTetrominoDown();
  }
  if (droppedRows)
    score += droppedRows-1;
  curRow--;
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
    }
  }
}

void GameOver() {
  mvprintw(11, 1, "Game Over!!");
  mvprintw(13, 1, "Final score");
  mvprintw(14, 2, "%d", score);
  refresh();
  sleep(5);
  getch();

}

// Draw each element to the screen
void DrawStuff() {
  DrawTitle();
  DrawBoard();
  DrawGhost();
  DrawTetromino();
  DrawBorder();
  DrawScore();
  DrawPaused();
}

int main() {
  Init();
  NewTetromino();

  bool advanceLogic = true;
  bool redraw = true;
  bool done = false;
  paused = false;
  while (1) {
    int c = getch();

    if (c == 'q') {
      break;
    }

    clear();
    system("clear");
    switch (c) {
      case 'a':
        if (paused)
          break;
        MoveTetrominoLeft();
        if (Collision(curTetromino, curRow)) {
          MoveTetrominoRight();
        }
        break;
      case 'd':
        if (paused)
          break;
        MoveTetrominoRight();
        if (Collision(curTetromino, curRow)) {
          MoveTetrominoLeft();
        }
        break;
      case 'w':
        if (paused)
          break;
        RotateTetromino();
        if (Collision(curTetromino, curRow)) {
          // do some collision checking to slide piece toward center
          RotateTetromino();
          RotateTetromino();
          RotateTetromino();
        }
        break;
      case 's':
        if (paused)
          break;
        DropTetromino();
        break;
      case 'e':
        if (paused)
          break;
        HoldTetromino();
        break;
      case 'p':
        Pause();
        break;
      default:
        if (paused)
          break;
        if ((getTimeNow() - now_2) >= (level_timer*200)) {
          now_2 = getTimeNow();
          advanceLogic = true;
        }
        if (advanceLogic) {
          advanceLogic = false;
          MoveTetrominoDown();
          if (Collision(curTetromino, curRow)) {
              MoveTetrominoUp();
              if (curRow == 0) {
                  GameOver();
                  done = true;
                  break;
              }
            CopyToBoard();
            RemoveCompletedRows();
            NewTetromino();
          }
        }
        break;
    }
    if (done)
      break;

    if ((getTimeNow() - now) >= frames) {
      now = getTimeNow();
      redraw = true;
    }
    

    if (redraw) {
      redraw = false;
      DrawStuff();

      refresh();
    }
  }

  endwin();
  return 0;
}
