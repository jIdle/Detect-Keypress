#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>

#define RED "\e[41m"
#define BLACK "\e[0m"

int x = 0;
int y = 0;
int tWidth = 0;
int tHeight = 0;

/* 
 * "move" receives the user's keypress as an argument.
 * Steps in order:
 *      1. Store previous coords
 *      2. On valid keypress, modify location
 *      3. Return whether coords have changed
 */

int move(char key) {
    int old = x + y;

    if(key == 'w')
        y -= (y > 0) ? 1 : 0;
    else if(key == 's')
        y += (y < tHeight-3) ? 1 : 0;
    else if(key == 'a')
        x -= (x > 0) ? 1 : 0;
    else if(key == 'd')
        x += (x < tWidth) ? 1 : 0;

    return !(old ^ (x+y));
}

/*
 * "changBG" changes the background color of the terminal using the
 * escape sequences defined at the top. 
 */

void changeBG(char * color) {
    for(int i = 0; i < tHeight; ++i) {
        for(int j = 0; j < tWidth; ++j) {
            printf("%s", color);
        }
        printf("\n");
    }
}

/* 
 * "display" receives a signal indicating that:
 *      1. The terminal and location should be refreshed
 *      2. The terminal should flash red
 */

void display(int atBounds) {
    if(atBounds) {
        changeBG(RED);
        system("clear");
        poll(NULL, NULL, 50);
        changeBG(BLACK);
    }

    system("clear");
    for(int ver = 0; ver < y; ++ver)
        printf("\n");
    for(int hor = 0; hor < x; ++hor)
        printf("  ");
    printf("O\n");
}


int main() {
    struct termios settings;
    struct winsize termDim;
    fd_set readfds;
    char key;

    tcgetattr(fileno(stdin), &settings);
    settings.c_lflag &= (~ICANON & ~ECHO); // Allow terminal to respond without a trailing delimiter in buffer
    tcsetattr(fileno(stdin), TCSANOW, &settings);

    ioctl(0, TIOCGWINSZ, &termDim); // Grab terminal dimensions
    tWidth = termDim.ws_col;
    tHeight = termDim.ws_row;

    system("setterm -cursor off"); // Cursor looks bad
    system("clear");

    // Input loop
    for(;;) {
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);

        int fdNum = select(fileno(stdin)+1, &readfds, NULL, NULL, NULL); // Wait on stdin for input

        if(fdNum > 0) {
            read(fileno(stdin), &key, 1);
            display(move(key)); // Update terminal
        } else if(fdNum < 0)
            exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
