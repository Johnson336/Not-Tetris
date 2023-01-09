#include <ncurses.h>
#include <unistd.h>
#include <random>


const int Rows = 20;
const int Cols = 10;
const int border_width = 1;

// Represents a game board
struct Board {
  int Rows;
  int Cols;
  int curRow;
  int curCol;
  int score;
  int level;
};

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
  Tetromino curTetromino;
  int curRow;
  int curCol;


// Clears the board
void ClearBoard() {
  for (int i = 0; i < Rows; i++) {
    for (int j = 0; j < Cols; j++) {
      board[i][j] = ' ';
    }
  }
}

// Initializes the board and sets up NCurse
void Init() {
  ClearBoard();
  initscr();
  
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
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

  mvprintw(-1, Cols/2, "Definitely Not Tetris");
}

// draw border around the game board
void DrawBorder() {
  mvhline(0, 0, '-', Cols + (border_width * 2));
  mvhline(Rows+1, 0, '-', Cols + (border_width * 2));
  mvvline(0, 0, '|', Rows + (border_width * 2));
  mvvline(0, Cols+1, '|', Rows + (border_width * 2));
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
  curTetromino = Tetrominoes[rand() % 7];
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
  while (!Collision()) {
    MoveTetrominoDown();
  }

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
}

void GameOver() {
  ClearScreen();
  int y,x;
  getmaxyx(stdscr, y, x);
  mvprintw(y/2, x/2, "Game Over!!");
  refresh();
  sleep(5);
  getch();

}

int main() {
  Init();
  NewTetromino();

  while (true) {
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
    
    DrawTitle();
    DrawBoard();
    DrawTetromino();
    DrawBorder();
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
    usleep((useconds_t)200000);
  }

  endwin();
  return 0;
}
