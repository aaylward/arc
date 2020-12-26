#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY(K) ((K) & 0x1f)

/*** data ***/

struct termios original_termios;

/*** terminal ***/

void terminal_cursorTopLeft() {
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void clearScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  terminal_cursorTopLeft();
}

void die(const char *s) {
  clearScreen();

  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
    die("tcgetattr");
  }
  atexit(disableRawMode);

  struct termios raw = original_termios;

  // BRKINT: break can send interrupt
  // ICRNL: ctrl-m
  // INPCK: enable parity checking
  // ISTRIP: strip 8th bit of each byte
  // IXON: ctrl-s, ctrl-q
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  // OPOST: disable output post processing
  raw.c_oflag &= ~(OPOST);

  // CS8: char size to 1 byte
  raw.c_cflag |= (CS8);

  // ECHO: echo input to output
  // ICANON: input is line buffered
  // IEXTERN: character literals? wat?
  // ISIG: disable ctrl-c, ctrl-z
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  // VMIN: min bytes before `read` returns
  // VTIME: wait at most 1 tenth of a second before `read` returns
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

/*** input ***/

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) {
      die("read");
    }
  }
  return c;
}

void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
    case CTRL_KEY('q'):
      clearScreen();
      exit(0);
      break;
  }
}

/*** output ***/

void editorDrawRows() {
  int y;
  for (y = 0; y<24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void editorRefreshScreen() {
  clearScreen();

  editorDrawRows();

  terminal_cursorTopLeft();
}

/*** init ***/

int main() {
  enableRawMode();

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}