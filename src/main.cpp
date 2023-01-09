#include <ncurses.h>
#include <unistd.h>
#include <random>
#include <chrono>


const int Rows = 20;
const int Cols = 10;
const int border_width = 1;
const double frames = 1.0 / 60.0;
double level_timer = 1.0;


// Represents a single block in a Tetromino
struct Block {
  int row;
  int col;
  char c;
};

// Represents a Tetromino piece
struct Tetromino {
  Block blocks[4];
  char c;
  int rotation;
};

// The Tetromino shapes
const Tetromino Tetrominoes[7] = {
  {{{0, 0, '*'}, {1, 0, '*'}, {2, 0, '*'}, {3, 0, '*'}}, 'I', 0},
  {{{0, 0, '*'}, {1, 0, '*'}, {0, 1, '*'}, {1, 1, '*'}}, 'O', 0},
  {{{0, 0, '*'}, {1, 0, '*'}, {2, 0, '*'}, {1, 1, '*'}}, 'T', 0},
  {{{1, 0, '*'}, {2, 0, '*'}, {0, 1, '*'}, {1, 1, '*'}}, 'S', 0},
  {{{0, 0, '*'}, {1, 0, '*'}, {1, 1, '*'}, {2, 1, '*'}}, 'Z', 0},
  {{{0, 1, '*'}, {1, 1, '*'}, {2, 1, '*'}, {2, 0, '*'}}, 'J', 0},
  {{{0, 0, '*'}, {1, 0, '*'}, {2, 0, '*'}, {2, 1, '*'}}, 'L', 0},
};

// The board grid
char board[Rows][Cols];

// Details of the current Tetromino
Tetromino nextTetromino;
Tetromino curTetromino;
int curRow;
int curCol;
unsigned long long score;
int level;
int clearedRows;

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

// Draws the current Tetromino
void DrawTetromino() {
  for (auto x: curTetromino.blocks) {
    mvaddch( border_width + curRow + x.row, border_width + curCol + x.col, x.c );
  }
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
  mvhline(0, 0, '-', Cols + (border_width * 2));
  mvhline(Rows+1, 0, '-', Cols + (border_width * 2));
  mvvline(0, 0, '|', Rows + (border_width * 2));
  mvvline(0, Cols+1, '|', Rows + (border_width * 2));
}

// draw boxes for score, next piece, level, and cleared lines
void DrawScore() {
  mvprintw(2, 14, "Next piece");
  for (auto x: nextTetromino.blocks) {
    mvaddch( 3 + x.row, 14 + x.col, x.c );
  }

  mvprintw(8, 14, "Score");
  mvprintw(9, 14, "%d", score);
  mvprintw(12, 14, "Level");
  mvprintw(13, 14, "%d", level);
  mvprintw(15, 14, "Lines cleared");
  mvprintw(16, 14, "%d", clearedRows);
}

// Copies the current Tetromino onto the board
void CopyToBoard() {
  for (int i = 0; i < 4; i++) {
    Block b = curTetromino.blocks[i];
    board[curRow + b.row][curCol + b.col] = curTetromino.c;
  }
}

// Rotates the current Tetromino clockwise
void RotateTetromino() {
  Tetromino old = curTetromino;

  for (int i = 0; i < 4; i++) {
    int x = old.blocks[i].row;
    int y = old.blocks[i].col;
    curTetromino.blocks[i].row = y;
    curTetromino.blocks[i].col = -x;
  }

  curTetromino.rotation = (curTetromino.rotation + 1) % 4;
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
bool Collision() {
  for (int i = 0; i < 4; i++) {
    Block b = curTetromino.blocks[i];
    int row = curRow + b.row;
    int col = curCol + b.col;

    if (row >= Rows || row < 0 || col < 0 || col >= Cols || board[row][col] != ' ') {
      return true;
    }
  }

  return false;
}

// Drops the current Tetromino down until it collides with something
void DropTetromino() {
  int droppedRows = 0;
  while (!Collision()) {
    droppedRows++;
    MoveTetrominoDown();
  }
  if (droppedRows)
    score += increaseScore(droppedRows-1);
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
  refresh();
  sleep(5);
  getch();

}

int main() {
  Init();
  NewTetromino();

  bool advanceLogic = true;
  bool redraw = true;
  while (1) {
    int c = getch();

    if (c == 'q') {
      break;
    }

    // ClearScreen();
    clear();
    system("clear");

    if (c == 'a') {
      MoveTetrominoLeft();
      if (Collision()) {
        MoveTetrominoRight();
      }
    } else if (c == 'd') {
      MoveTetrominoRight();
      if (Collision()) {
        MoveTetrominoLeft();
      }
    } else if (c == 'w') {
      RotateTetromino();
      if (Collision()) {
        RotateTetromino();
        RotateTetromino();
        RotateTetromino();
      }
    } else if (c == 's') {
      DropTetromino();
    } else {
      if ((getTimeNow() - now_2) >= (level_timer*200)) {
        now_2 = getTimeNow();
        advanceLogic = true;
      }
      if (advanceLogic) {
        advanceLogic = false;
        MoveTetrominoDown();
        if (Collision()) {
            MoveTetrominoUp();
            if (curRow == 0) {
                GameOver();
                break;
            }
          CopyToBoard();
          RemoveCompletedRows();
          NewTetromino();
        }
      }
    }

    if ((getTimeNow() - now) >= frames) {
      now = getTimeNow();
      redraw = true;
    }
    

    if (redraw) {
      redraw = false;
      DrawTitle();
      DrawBoard();
      DrawTetromino();
      DrawBorder();
      DrawScore();
    /* int maxy,maxx;
      getmaxyx(stdscr, maxy, maxx);
      int numWaterfalls = (rand() % 10); // somewhere between 1 and 10 waterfalls going at once
      for (int i=0;i<numWaterfalls; i++) {
          int x = (rand() % maxx); // pick a column between 1 and maxx
          for (int y=0;y<maxy;y++) {
              mvprintw(y, x, "%d", y);
          }
      }
      */
      refresh();
    }
  }

  endwin();
  return 0;
}
