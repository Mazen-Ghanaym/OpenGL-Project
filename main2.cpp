#include <GL/freeglut.h>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <ctime>
#include <string>

struct Pipe {
    float x;
    float gapY;
};

enum GameState { MENU, PLAYING, GAME_OVER };
GameState gameState = MENU;

const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
const int PIPE_WIDTH = 80;
const int GAP_HEIGHT = 200;
const float GRAVITY = 0.4f;
const float POWER = -8.0f;
const float PIPE_SPEED = 3.0f;
const float PI = 22 / 7.0f;
const float ballRadius = 20.0f;

float ballY = WINDOW_HEIGHT / 2;
float ballSpeed = 0.0f;

std::vector<Pipe> pipes;
int score = 0;
int frameCount = 0;

void resetGame() {
    ballY = WINDOW_HEIGHT / 2;
    ballSpeed = 0;
    score = 0;
    pipes.clear();
    frameCount = 0;
}

void drawRectangle(float x, float y, float w, float h, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawBall(float x, float y, float radius) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * (PI / 180);
        float dx = cos(angle) * radius;
        float dy = sin(angle) * radius;
        float shadeFactor = 0.5f * (1.0f - (dy + radius) / (2 * radius));
        float r = 1.0f * (1.0f - shadeFactor);
        float g = 0.0f;
        float b = 0.0f;
        glColor3f(r, g, b);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

void drawText(float x, float y, const std::string &text, void *font = GLUT_BITMAP_HELVETICA_18) {
    glColor3f(0, 0, 0);
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}


void checkCollision() {
    // Check collisions and score
    float ballLeft = 100 - ballRadius;
    float ballRight = 100 + ballRadius;
    float ballTop = ballY + ballRadius;
    float ballBottom = ballY - ballRadius;

    for (auto &pipe : pipes) {
        float pipeLeft = pipe.x;
        float pipeRight = pipe.x + PIPE_WIDTH;

        // Horizontal overlap
        if (ballRight > pipeLeft && ballLeft < pipeRight) {
            if (ballBottom < pipe.gapY || ballTop > pipe.gapY + GAP_HEIGHT) {
                gameState = GAME_OVER;
            }
        }
        // Score update
        if (pipe.x + PIPE_WIDTH < 100 && pipe.x + PIPE_WIDTH + PIPE_SPEED >= 100) {
            score++;
        }
    }

    if (ballY < 0 || ballY + 30 > WINDOW_HEIGHT)
        gameState = GAME_OVER;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.5f, 0.8f, 1.0f);

    if (gameState == MENU) {
        drawText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 + 20, "Press 'S' to Start");
        drawText(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 10, "Flappy Ball OpenGL");
    }
    else if (gameState == PLAYING) {
        float ballX = 100.0f;
        drawBall(ballX, ballY, ballRadius);

        for (auto &pipe : pipes) {
            drawRectangle(pipe.x, pipe.gapY + GAP_HEIGHT, PIPE_WIDTH, WINDOW_HEIGHT, 0.0f, 0.8f, 0.0f);
            drawRectangle(pipe.x, 0, PIPE_WIDTH, pipe.gapY, 0.0f, 0.8f, 0.0f);
        }

        drawText(10, WINDOW_HEIGHT - 30, "Score: " + std::to_string(score));
    }
    else if (gameState == GAME_OVER) {
        drawText(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 + 20, "Game Over!");
        drawText(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 10, "Press 'R' to Restart");
        drawText(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 40, "Score: " + std::to_string(score));
    }

    glutSwapBuffers();
}

void update(int value) {
    if (gameState != PLAYING) return;

    ballSpeed += GRAVITY;
    ballY -= ballSpeed;

    // Add new pipe every 100 frames
    frameCount++;
    if (frameCount % 100 == 0) {
        Pipe newPipe;
        newPipe.x = WINDOW_WIDTH;
        newPipe.gapY = rand() % (WINDOW_HEIGHT - GAP_HEIGHT - 100) + 50;
        pipes.push_back(newPipe);
    }

    // Move pipes
    for (auto &pipe : pipes) {
        pipe.x -= PIPE_SPEED;
    }

    // Remove off-screen pipes
    if (!pipes.empty() && pipes.front().x + PIPE_WIDTH < 0)
        pipes.erase(pipes.begin());

    checkCollision();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) exit(0); // ESC

    if (gameState == MENU && (key == 's' || key == 'S')) {
        resetGame();
        gameState = PLAYING;
        glutTimerFunc(16, update, 0);
    }

    if (gameState == PLAYING && key == 32) {
        ballSpeed = POWER;
    }

    if (gameState == GAME_OVER && (key == 'r' || key == 'R')) {
        resetGame();
        gameState = PLAYING;
        glutTimerFunc(16, update, 0);
    }
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    srand(static_cast<unsigned int>(time(0)));
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Flappy Ball");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}