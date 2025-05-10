#include <GL/freeglut.h>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <ctime>
#include <string>

enum GameState { START_MENU, PLAYING, GAME_OVER };
GameState gameState = START_MENU;

struct Pipe {
    float x;
    float gapY;
};

const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
const int PIPE_WIDTH = 80;
const float GRAVITY = 0.4f;
const float POWER = -8.0f;
const float PI = 3.14159265f;
const float ballRadius = 20.0f;

float pipeSpeed = 3.0f;
int gapHeight = 200;
const int MIN_GAP_HEIGHT = 120;

float ballY = WINDOW_HEIGHT / 2;
float ballSpeed = 0.0f;
bool gameOver = false;

std::vector<Pipe> pipes;
int score = 0;
int highScore = 0;
int frameCount = 0;

void resetGame() {
    ballY = WINDOW_HEIGHT / 2;
    ballSpeed = 0;
    gameOver = false;
    score = 0;
    frameCount = 0;
    pipeSpeed = 3.0f;
    gapHeight = 200;
    pipes.clear();
}

void drawText(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (char c : text)
        glutBitmapCharacter(font, c);
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

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.5f, 0.8f, 1.0f);

    if (gameState == START_MENU) {
        drawText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 + 30, "FLAPPY BALL");
        drawText(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 10, "Press SPACE to Start");
    } else if (gameState == GAME_OVER) {
        drawText(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 + 30, "Game Over!");
        drawText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 10, "Press R to Restart");
        drawText(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 50, "ESC to Quit");

        std::string scoreStr = "Score: " + std::to_string(score);
        std::string highScoreStr = "High Score: " + std::to_string(highScore);
        drawText(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 90, scoreStr);
        drawText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 120, highScoreStr);
    } else {
        float ballX = 100.0f;
        drawBall(ballX, ballY, ballRadius);

        for (auto& pipe : pipes) {
            drawRectangle(pipe.x, pipe.gapY + gapHeight, PIPE_WIDTH, WINDOW_HEIGHT, 0.0f, 0.8f, 0.0f);
            drawRectangle(pipe.x, 0, PIPE_WIDTH, pipe.gapY, 0.0f, 0.8f, 0.0f);
        }

        // Draw Score
        glColor3f(0, 0, 0);
        drawText(10, WINDOW_HEIGHT - 30, "Score: " + std::to_string(score));
    }

    glutSwapBuffers();
}

void update(int value) {
    if (gameState != PLAYING)
        return;

    ballSpeed += GRAVITY;
    ballY -= ballSpeed;

    frameCount++;
    if (frameCount % 100 == 0) {
        Pipe newPipe;
        newPipe.x = WINDOW_WIDTH;
        newPipe.gapY = rand() % (WINDOW_HEIGHT - gapHeight - 100) + 50;
        pipes.push_back(newPipe);
    }

    for (auto& pipe : pipes)
        pipe.x -= pipeSpeed;

    if (!pipes.empty() && pipes.front().x + PIPE_WIDTH < 0)
        pipes.erase(pipes.begin());

    float ballLeft = 100 - ballRadius;
    float ballRight = 100 + ballRadius;
    float ballTop = ballY + ballRadius;
    float ballBottom = ballY - ballRadius;

    for (auto& pipe : pipes) {
        float pipeLeft = pipe.x;
        float pipeRight = pipe.x + PIPE_WIDTH;

        if (ballRight > pipeLeft && ballLeft < pipeRight) {
            if (ballBottom < pipe.gapY || ballTop > pipe.gapY + gapHeight) {
                gameState = GAME_OVER;
                if (score > highScore)
                    highScore = score;
                return;
            }
        }

        if (pipe.x + PIPE_WIDTH < 100 && pipe.x + PIPE_WIDTH + pipeSpeed >= 100) {
            score++;
            if (score % 10 == 0) {
                if (pipeSpeed < 7.0f) pipeSpeed += 0.5f;
                if (gapHeight > MIN_GAP_HEIGHT) gapHeight -= 10;
            }
        }
    }

    if (ballY < 0 || ballY + 30 > WINDOW_HEIGHT) {
        gameState = GAME_OVER;
        if (score > highScore)
            highScore = score;
        return;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) exit(0); // ESC

    if (gameState == START_MENU && key == 32) { // SPACE to start
        gameState = PLAYING;
        resetGame();
        glutTimerFunc(16, update, 0);
    } else if (gameState == PLAYING && key == 32) {
        ballSpeed = POWER;
    } else if (gameState == GAME_OVER && key == 'r') {
        gameState = PLAYING;
        resetGame();
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

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Flappy Ball Enhanced");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}