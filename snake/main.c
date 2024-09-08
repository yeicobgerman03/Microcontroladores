#include <stdio.h>
#include <conio.h> // Para _kbhit() y _getch()
#include <stdlib.h> // Para rand() y srand()
#include <time.h> // Para time()

#define WIDTH 60
#define HEIGHT 20

int gameOver;
int score;
int x, y, fruitX, fruitY;
int tailX[100], tailY[100];
int nTail;
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
enum eDirection dir;

void setup() {
    gameOver = 0;
    dir = STOP;
    x = WIDTH / 2;
    y = HEIGHT / 2;
    fruitX = rand() % WIDTH;
    fruitY = rand() % HEIGHT;
    score = 0;
}

void draw() {
    system("cls"); // Funciona en Windows. Para Linux o Mac, usar system("clear");
    for (int i = 0; i < WIDTH + 2; i++) printf("-");
    printf("\n");

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (j == 0) printf("|");
            if (i == y && j == x)
                printf("0"); // Cabeza de la serpiente
            else if (i == fruitY && j == fruitX)
                printf("x"); // Manzana
            else {
                int print = 0;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        printf("X"); // Cuerpo de la serpiente
                        print = 1;
                    }
                }
                if (!print) printf(" ");
            }

            if (j == WIDTH - 1) printf("|");
        }
        printf("\n");
    }

    for (int i = 0; i < WIDTH + 2; i++) printf("-");
    printf("\n");
    printf("Score: %d\n", score);
}

void input() {
    if (_kbhit()) {
        switch (_getch()) {
            case 'a':
                dir = LEFT;
                break;
            case 'd':
                dir = RIGHT;
                break;
            case 'w':
                dir = UP;
                break;
            case 's':
                dir = DOWN;
                break;
            case 'x':
                gameOver = 1;
                break;
        }
    }
}

void logic() {
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;

    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (dir) {
        case LEFT:
            x--;
            break;
        case RIGHT:
            x++;
            break;
        case UP:
            y--;
            break;
        case DOWN:
            y++;
            break;
        default:
            break;
    }

    // Si la serpiente toca los bordes, se termina el juego
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        gameOver = 1;
    }

    for (int i = 0; i < nTail; i++) {
        if (tailX[i] == x && tailY[i] == y)
            gameOver = 1;
    }

    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = rand() % WIDTH;
        fruitY = rand() % HEIGHT;
        nTail++;
    }
}

int main() {
    srand(time(0));
    setup();
    while (!gameOver) {
        draw();
        input();
        logic();
        _sleep(100); // Controla la velocidad de la serpiente (100 ms)
    }

    // Mensaje de fin de juego
    printf("\nThe game has finished\n");
    return 0;
}


