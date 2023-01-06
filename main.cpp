#include <ncurses.h>
#include <unistd.h>

const int Rows = 20;
const int Cols = 10;

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
  {{{0, 3, '*'}, {1, 3, '*'}, {2, 3, '*'}, {3, 3, '*'}}, 'I', 0},
  {{{0, 3, '*'}, {0, 4, '*'}, {1, 3, '*'}, {1, 4, '*'}}, 'O', 0},
  {{{0, 3, '*'}, {1, 3, '*'}, {2, 3, '*'}, {2, 4, '*'}}, 'T', 0},
  {{{0, 4, '*'}, {1, 3, '*'}, {1, 4, '*'}, {2, 3, '*'}}, 'S', 0},
  {{{0, 3, '*'}, {1, 3, '*'}, {1, 4, '*'}, {2, 4, '*'}}, 'Z', 0},
  {{{0, 3, '*'}, {1, 3, '*'}, {1, 4, '*'}, {1, 5, '*'}}, 'J', 0},
  {{{0, 5, '*'}, {1, 3, '*'}, {1, 4, '*'}, {1, 5, '*'}}, 'L', 0},
};

// The board grid
char board[Rows][Cols];

// The current Tetromino and its position
Tetromino curTetromino;
int curRow;
int curCol;

// Initializes the board and sets up NCurse
void Init() {
  for (int i = 0; i < Rows; i++) {
    for (int j = 0; j < Cols; j++) {
      board[i][j] = ' ';
    }
  }

  initscr();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
}

// Clears the board
void ClearBoard() {
  for (int i = 0; i < Rows; i++) {
    for (int j = 0; j < Cols; j++) {
      board[i][j] = ' ';
    }
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
  for (int i = 0; i < 4; i++) {
    Block b = curTetromino.blocks[i];
    mvaddch(curRow + b.row, curCol + b.col, curTetromino.c);
  }
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

// Generates a new Tetromino
void NewTetromino() {
  curTetromino = Tetrominoes[rand() % 7];
  curRow = 0;
  curCol = Cols / 2;
}

// Drops the current Tetromino down until it collides with something
void DropTetromino() {
  while (!Collision()) {
    MoveTetrominoDown();
  }

  curRow--;
}

// Returns true if the current Tetromino collides with something
bool Collision() {
  for (int i = 0; i < 4; i++) {
    Block b = curTetromino.blocks[i];
    int row = curRow + b.row;
    int col = curCol + b.col;

    if (row >= Rows || col < 0 || col >= Cols || board[row][col] != ' ') {
      return true;
    }
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
}

int main() {
  Init();
  NewTetromino();

  while (true) {
    int c = getch();

    if (c == 'q') {
      break;
    }

    ClearScreen();

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
        CopyToBoard();
        RemoveCompletedRows();
        NewTetromino();
      }
    }

    DrawTetromino();

    for (int i = 0; i < Rows; i++) {
      for (int j = 0; j < Cols; j++) {
        mvaddch(i, j, board[i][j]);
      }
    }

    refresh();
    usleep(200000);
  }

  endwin();
  return 0;
}
