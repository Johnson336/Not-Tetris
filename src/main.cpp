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

#include <ncurses.h>
#include <unistd.h>
#include <random>
#include <chrono>
#include <iostream>



void must_init(bool test, const char *description) {
  if (test) return;

  printf("Unable to initialize %s\n", description);
  exit(1);
}


const int Rows = 20;
const int Cols = 10;
const int border_width = 1;
const double frames = 1.0 / 60.0;
double level_timer = 2.0;
unsigned char moving_char = '*';


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
  int rotation;
  ALLEGRO_COLOR color;
};

// The Tetromino shapes
const Tetromino Tetrominoes[7] = {
  {{ {1, 0}, {1, 1}, {1, 2}, {1, 3},
     {0, 2}, {1, 2}, {2, 2}, {3, 2},
     {1, 0}, {1, 1}, {1, 2}, {1, 3},
     {0, 2}, {1, 2}, {2, 2}, {3, 2} }, 'I', 0, cyan},
  {{ {1, 0}, {1, 1}, {1, 2}, {2, 1},
     {0, 1}, {1, 0}, {1, 1}, {2, 1},
     {1, 1}, {2, 0}, {2, 1}, {2, 2},
     {0, 1}, {1, 1}, {1, 2}, {2, 1} }, 'T', 0, purple},
  {{ {1, 0}, {1, 1}, {1, 2}, {2, 0},
     {0, 0}, {0, 1}, {1, 1}, {2, 1},
     {1, 2}, {2, 0}, {2, 1}, {2, 2},
     {0, 1}, {1, 1}, {2, 1}, {2, 2} }, 'L', 0, orange},
  {{ {1, 0}, {1, 1}, {1, 2}, {2, 2},
     {0, 1}, {1, 1}, {2, 0}, {2, 1},
     {1, 0}, {2, 0}, {2, 1}, {2, 2},
     {0, 1}, {0, 2}, {1, 1}, {2, 1} }, 'J', 0, blue},
  {{ {1, 1}, {1, 2}, {2, 0}, {2, 1},
     {0, 0}, {1, 0}, {1, 1}, {2, 1},
     {1, 1}, {1, 2}, {2, 0}, {2, 1},
     {0, 0}, {1, 0}, {1, 1}, {2, 1} }, 'S', 0, green},
  {{ {1, 0}, {1, 1}, {2, 1}, {2, 2},
     {0, 2}, {1, 1}, {1, 2}, {2, 1},
     {1, 0}, {1, 1}, {2, 1}, {2, 2},
     {0, 2}, {1, 1}, {1, 2}, {2, 1} }, 'Z', 0, red},
  {{ {1, 1}, {1, 2}, {2, 1}, {2, 2},
     {1, 1}, {1, 2}, {2, 1}, {2, 2},
     {1, 1}, {1, 2}, {2, 1}, {2, 2},
     {1, 1}, {1, 2}, {2, 1}, {2, 2} }, 'O', 0, yellow},
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
bool redraw;
bool advanceLogic;

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
  curTetromino = Tetrominoes[rand() % 7];
  nextTetromino = Tetrominoes[rand() % 7];
  score = 0;
  level = 1;
  clearedRows = 0;
  paused = false;
  redraw = true;
  advanceLogic = false;
  ClearBoard();
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
  curCol = 4;
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

// Pause and un-pause the game
void Pause() {
  paused = !paused;
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
void RemoveCompletedRows(ALLEGRO_AUDIO_STREAM* music) {
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
      al_set_audio_stream_speed(music, 1+(level*.005));
    }
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
      grey,
      1.0 );
}





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
  printTetromino(curTetromino, curCol, curRow);
}


void drawScore(ALLEGRO_FONT* font) {
  drawText("Held Piece", Cols + 2, 1, font, white);
  if (held)
    printTetromino(heldTetromino, Cols + 3, 2);
  drawText("Next Piece", Cols + 2, 8, font, white);
  printTetromino(nextTetromino, Cols + 3, 9);
  drawText("Score", Cols + 2, 15, font, white);
  char outText[20];
  snprintf(outText, 20, "%07llu", score);
  drawText(outText, Cols + 3, 16, font, white);
  drawText("Level", Cols + 2, 17, font, white);
  snprintf(outText, 20, "%d", level);
  drawText(outText, Cols + 3, 18, font, white);
  drawText("Lines Cleared", Cols + 2, 19, font, white);
  snprintf(outText, 20, "%d", clearedRows);
  drawText(outText, Cols + 3, 20, font, white);
  

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
  int curY = curRow;
  while (!Collision(curTetromino, curY)) {
    curY++;
  }
  curY--;
  // Draw mostly transparent ghost block
  printOutline(curTetromino, curCol, curY);
}

void GameOver(ALLEGRO_FONT* font) {
  
  // Draw Game Over across screen
  drawBox(0, (Rows/2)-1, 12, 6, black);
  drawBoxOutline(0, (Rows/2)-1, 12, 6, 1.0, white);
  drawText("Game Over", 3, Rows/2, font, white);
  char outText[20];
  snprintf(outText, 20, "%llu", score);
  drawText("Final Score:", 2, (Rows/2)+1, font, white);
  drawText(outText, 4, (Rows/2)+2, font, white);
  drawText("Press [ESC] to exit", 1, (Rows/2) + 4, font, white);
}


// Draw each element to the screen
void DrawStuff(ALLEGRO_FONT* font) {
  drawBoard();
  drawGrid();
  drawGhost();
  drawTetromino();
  drawScore(font);
  drawPaused(font);
}

bool performMoveDown(ALLEGRO_AUDIO_STREAM* music) {
    MoveTetrominoDown();
  if (Collision(curTetromino, curRow)) {
    MoveTetrominoUp();
    if (curRow == 0) {
      return true;
    }
    CopyToBoard();
    RemoveCompletedRows(music);
    NewTetromino();
  }
  return false;
}

void performRotate() {
  RotateTetromino();
  if (Collision(curTetromino, curRow)) {
    // do some collision checking to slide piece toward center
    RotateTetromino();
    RotateTetromino();
    RotateTetromino();
  }
}

void performDrop() {
  DropTetromino();
  advanceLogic = true;
}

void performMoveLeft() {
  MoveTetrominoLeft();
  if (Collision(curTetromino, curRow)) {
    MoveTetrominoRight();
  }
}

void performMoveRight() {
  MoveTetrominoRight();
  if (Collision(curTetromino, curRow)) {
    MoveTetrominoLeft();
  }
}

void performHold() {
  HoldTetromino();
}

// Begin our AI functions
void AI_play(ALLEGRO_AUDIO_STREAM* music) {
  int move = rand() % 4;
  switch (move) {
    case 1:
      performDrop();
      break;
    case 2:
      performMoveLeft();
      break;
    case 3:
      performMoveRight();
      break;
    case 4:
      performRotate();
      break;

  }
}

int real_main(int argc, char** argv) {
  // initialize game-related variables
  Init();

  must_init(al_init(), "Allegro");

  must_init(al_install_keyboard(), "keyboard");

  must_init(al_install_mouse(), "mouse");

  // BRING THE JAMS
  must_init(al_install_audio(), "audio");
  must_init(al_init_acodec_addon(), "audio codecs");
  must_init(al_reserve_samples(16), "reserve samples");


  ALLEGRO_SAMPLE* hammer = al_load_sample("assets/hammer3.wav");
  must_init(hammer, "hammer");

  
  ALLEGRO_AUDIO_STREAM* music = al_load_audio_stream("assets/Electro_-Nitro-Fun-New-Game-_Monstercat-Release_.ogg", 2, 2048);
  must_init(music, "music");
  al_set_audio_stream_playmode(music, ALLEGRO_PLAYMODE_LOOP);
  al_attach_audio_stream_to_mixer(music, al_get_default_mixer());

  // set our FPS target
  ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
  must_init(timer, "timer");

  // Timer to control fall speed
  ALLEGRO_TIMER* tick_timer = al_create_timer(1.0);
  must_init(tick_timer, "tick_timer");

  // Timer to control the action speed of the AI player
  // 1.0 / 30.0 = 30 moves per second
  ALLEGRO_TIMER* ai_timer = al_create_timer(1.0 / 30.0);
  must_init(ai_timer, "ai_timer");

  ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
  must_init(queue, "queue");

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

  // create our first tetromino
  NewTetromino();

  bool using_ai = false;
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
          if (performMoveDown(music)) {
            gameover = true;
            redraw = true;
            break;
          }
        }
        // AI timer
        if (ai_move) {
          ai_move = false;
          AI_play(music);
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
            performRotate();
          break;
          case ALLEGRO_KEY_S:
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
            performHold();
          break;
          case ALLEGRO_KEY_Q:
          case ALLEGRO_KEY_ESCAPE:
            done = true;
          break;
          case ALLEGRO_KEY_P:
            Pause();
            redraw = true;
          break;
        }
        break;
      case ALLEGRO_EVENT_DISPLAY_CLOSE:
        done = true;
        break;
    }
    // Broken out of event switch
    if (done) {
      break;
    }
    
    if (redraw && al_is_event_queue_empty(queue)) {
      redraw = false;
      al_clear_to_color(black);

      DrawStuff(font);
      if (gameover)
        GameOver(font);

      al_flip_display();
    }

  }

  al_destroy_font(font);
  al_destroy_display(disp);
  al_destroy_timer(timer);
  al_destroy_timer(tick_timer);
  al_destroy_timer(ai_timer);
  al_destroy_event_queue(queue);
  al_destroy_sample(hammer);
  al_destroy_audio_stream(music);
  return 0;
}

int main(int argc, char** argv) {
  return al_run_main(argc, argv, real_main);
}
