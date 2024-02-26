#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

#include "tinyline.h"

static struct termios originalTermConfig;

// UTF-8 bytes whose most significant bits are 10 are
// never the first byte of a new character.
static int utf8strlen(const char *string) {
  if(string == NULL) return 0;
  int length = 0;
  for (const char *c = string; *c != '\0'; c++) {
      if ((*c & 0b11000000) != 0b10000000) length++;
  }
  return length;
}

static inline int max(int a, int b) { return (a > b) ? a : b; }
static inline int min(int a, int b) { return (a < b) ? a : b; }

static bool isEmpty(const char* string) {
  for (const char* c = string; *c != '\0'; c++) {
    if(!isspace(' ')) return false;
  }
  return true;
}

static void fatalError(const char* message) {
  perror(message);
  exit(1);
}

#define NO_SELECTION -1
#define DEFAULT_PROMPT "> "
#define HISTORY_CAPACITY 100
static char* history[HISTORY_CAPACITY];
static int historySelection = -1;
static int historyCount = 0;
static char* input = NULL;
static int inputCapacity = 0;
static int inputCount = 0;
static char* prompt = NULL;

static void clearLine(void) {
  printf("\33[2K\r"); // Erase line and reset cursor to beginning
}

static void moveCursorToEndOfInput(void) {
  printf("\r\033[%dC", utf8strlen(prompt) + utf8strlen(input) + 1);
}

static void redrawPromptLine(void) {
  clearLine();
  printf("%s%s", prompt, input != NULL ? input : "");
  moveCursorToEndOfInput();
}

static void eraseInputBuffer() {
  if(input != NULL) {
    free(input);
    input = NULL;
  }
  inputCount = 0;
  inputCapacity = 0;
}

static void resizeInputBuffer(int minSize) {
  if(inputCapacity >= minSize) return;
  inputCapacity = max(max(minSize, 255), inputCount);
  input = realloc(input, sizeof(char) * inputCapacity);
}

static void setInputFromHistory(void) {
  char *newInput = "";
  int newInputLength = 1;

  if (historySelection != NO_SELECTION) {
    newInput = history[historySelection];
    newInputLength = strlen(newInput);
  }

  eraseInputBuffer();
  resizeInputBuffer(newInputLength);
  memcpy(input, newInput, newInputLength);
  inputCount = newInputLength;
}

static void historyDown(void) {
  historySelection = max(min(historySelection + 1, historyCount), NO_SELECTION);
  if(historySelection == historyCount) historySelection = NO_SELECTION;
  setInputFromHistory();
  redrawPromptLine();
}

static void historyUp(void) {
  if(historySelection == NO_SELECTION) historySelection = historyCount;
  historySelection = max(min(historySelection - 1, historyCount - 1), NO_SELECTION);
  setInputFromHistory();
  redrawPromptLine();
}

static void saveHistory(void) {
  if(inputCount == 0 || isEmpty(input)) return;
  historyCount = historyCount % HISTORY_CAPACITY;
  if(history[historyCount] != NULL) free(history[historyCount]);
  history[historyCount] = malloc(sizeof(char) * inputCount);
  memcpy(history[historyCount++], input, inputCount);
}

static void setPrompt(const char* p, int length) {
  prompt = malloc(sizeof(char) * length);
  memcpy(prompt, p, length);
  redrawPromptLine();
}

static void resetTerminal(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermConfig) == -1)
    FATAL_ERROR("tcsetattr");
}

static void setupTerminal(void) {
  if (tcgetattr(STDIN_FILENO, &originalTermConfig) == -1)
    FATAL_ERROR("tcgetattr");
  atexit(resetTerminal); // Undo changes before exit

  // Put terminal into "raw" mode:
  // ~ECHO: no echoing back characters.
  // ~ICANON no canonical input behavior, read byte by byte instead.
  struct termios termConfig = originalTermConfig;
  termConfig.c_lflag &= ~(ECHO | ICANON);
  termConfig.c_cc[VMIN] = 0; // Read minimum of 0 bytes before returning.
  termConfig.c_cc[VTIME] = 10; // Block for 1s waiting for input.

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termConfig) == -1)
    FATAL_ERROR("tcsetattr");

  // Flush STDOUT immediately, no buffering.
  setvbuf(stdout, NULL, _IONBF, 0);
}

void TinyLine_setup(void) {
  setupTerminal();
  setPrompt("> ", 3); // includes \0.
}

char* TinyLine_readLine(void) {
  eraseInputBuffer();
  resizeInputBuffer(255);
  redrawPromptLine();

  // The input loop.
  while (1) {
    char c = 0x0;
    read(STDIN_FILENO, &c, 1);

    // If the read times out, restart the input loop.
    if(c == 0x0) continue;

    if (inputCount >= (inputCapacity - 1))
      resizeInputBuffer(inputCapacity * 1.5);

    // Handle deletion via backspace
    if (c == 0x7F) {
      if (inputCount > 0) {
        input[--inputCount] = 0x0;
        printf("\x1b[D\33[0K"); // move cursor back and erase rest of line
      }
      continue;
    }

    // Newline terminates input, terminate string and return.
    if (c == '\n') {
      input[inputCount] = '\0';
      break;
    }

    // We only deal with four escape sequences: arrows
    if (c == 0x1b) {
      read(STDIN_FILENO, &c, 1);
      if (c != '[') break;
      read(STDIN_FILENO, &c, 1);

      switch (c) {
        case 'A': // Arrow up: scroll through history
          historyUp();
          break;
        case 'B': // Arrow down: scroll through history
          historyDown();
          break;
        case 'C': // Arrow left: move cursor left
          printf("\x1b[%c", c);
          break;
        case 'D': // Arrow right: move cursor right
          printf("\x1b[%c", c);
          break;
      }
    } else {
      input[inputCount++] = c;
      printf("%c", c);
    }
  }

  saveHistory();
  clearLine();
  return input;
}
