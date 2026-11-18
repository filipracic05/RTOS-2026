#include <stdio.h>

#ifdef _WIN32
#include <conio.h>

#else
#include <termios.h>
#include <unistd.h>

// POSIX (Linux / macOS) verzija
static struct termios oldt, newt;

static void initTermios(int echo) {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ICANON;
    if (echo)
        newt.c_lflag |= ECHO;
    else
        newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void resetTermios(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

static char getch_(int echo) {
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

char getch(void) {
    return getch_(0);
}

char getche(void) {
    return getch_(1);
}
#endif
