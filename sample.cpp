#include <GL/freeglut.h>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <ctime>
#include <string>

struct Pipe
{
    float x;
    float gapY;
};

struct PowerUp
{
    float x;
    float y;
    int type; // 0: Shield, 1: Slow Motion, 2: Double Points
    bool active;
};

enum GameState
{
    MENU,
    PLAYING,
    GAME_OVER
};
GameState gameState = MENU;

const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
const int PIPE_WIDTH = 80;
const int GAP_HEIGHT = 200;
const float GRAVITY = 0.4f;
const float POWER = -8.0f;
const float PIPE_SPEED = 3.0f;
const float PI = 22 / 7.0f;
const float ballRadius = 20.0f;

// Difficulty scaling variables
float currentPipeSpeed = PIPE_SPEED;
float currentGapHeight = GAP_HEIGHT;
float currentGravity = GRAVITY;
const float SPEED_INCREASE = 0.2f;    // Speed increase per 5 points
const float GAP_DECREASE = 5.0f;      // Gap decrease per 5 points
const float GRAVITY_INCREASE = 0.02f; // Gravity increase per 5 points
const int DIFFICULTY_INTERVAL = 5;    // Points needed for difficulty increase
const float MIN_GAP_HEIGHT = 100.0f;  // Minimum gap height

// Power-up constants
const int SHIELD = 0;
const int SLOW_MOTION = 1;
const int DOUBLE_POINTS = 2;
const float POWER_UP_RADIUS = 15.0f;
const int POWER_UP_DURATION = 300; // Duration in frames (5 seconds at 60 FPS)
const float POWER_UP_SPEED = 2.0f;

// Power-up state variables
std::vector<PowerUp> powerUps;
bool hasShield = false;
bool hasSlowMotion = false;
bool hasDoublePoints = false;
int powerUpTimer = 0;
int activePowerUp = -1;

float ballY = WINDOW_HEIGHT / 2;
float ballSpeed = 0.0f;

std::vector<Pipe> pipes;
int score = 0;
int frameCount = 0;

void resetGame()
{
    ballY = WINDOW_HEIGHT / 2;
    ballSpeed = 0;
    score = 0;
    pipes.clear();
    frameCount = 0;

    // Reset difficulty
    currentPipeSpeed = PIPE_SPEED;
    currentGapHeight = GAP_HEIGHT;
    currentGravity = GRAVITY;

    // Reset power-ups
    powerUps.clear();
    hasShield = false;
    hasSlowMotion = false;
    hasDoublePoints = false;
    powerUpTimer = 0;
    activePowerUp = -1;
}

void drawRectangle(float x, float y, float w, float h, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawBall(float x, float y, float radius)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++)
    {
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

void drawPowerUp(float x, float y, int type)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++)
    {
        float angle = i * (PI / 180);
        float dx = cos(angle) * POWER_UP_RADIUS;
        float dy = sin(angle) * POWER_UP_RADIUS;

        // Different colors for different power-ups
        switch (type)
        {
        case SHIELD:
            glColor3f(0.0f, 0.0f, 1.0f); // Blue for shield
            break;
        case SLOW_MOTION:
            glColor3f(0.0f, 1.0f, 0.0f); // Green for slow motion
            break;
        case DOUBLE_POINTS:
            glColor3f(1.0f, 1.0f, 0.0f); // Yellow for double points
            break;
        }
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

void drawText(float x, float y, const std::string &text, void *font = GLUT_BITMAP_HELVETICA_18)
{
    glColor3f(0, 0, 0);
    glRasterPos2f(x, y);
    for (char c : text)
    {
        glutBitmapCharacter(font, c);
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    drawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.5f, 0.8f, 1.0f);

    if (gameState == MENU)
    {
        drawText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 + 20, "Press 'S' to Start");
        drawText(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 10, "Flappy Ball by Elmstaba");
    }
    else if (gameState == PLAYING)
    {
        float ballX = 100.0f;

        // Draw power-up effect on ball if shield is active
        if (hasShield)
        {
            glBegin(GL_LINE_LOOP);
            glColor3f(0.0f, 0.0f, 1.0f);
            for (int i = 0; i < 360; i++)
            {
                float angle = i * (PI / 180);
                float dx = cos(angle) * (ballRadius + 5);
                float dy = sin(angle) * (ballRadius + 5);
                glVertex2f(ballX + dx, ballY + dy);
            }
            glEnd();
        }

        drawBall(ballX, ballY, ballRadius);

        for (auto &pipe : pipes)
        {
            drawRectangle(pipe.x, pipe.gapY + currentGapHeight, PIPE_WIDTH, WINDOW_HEIGHT, 0.0f, 0.8f, 0.0f);
            drawRectangle(pipe.x, 0, PIPE_WIDTH, pipe.gapY, 0.0f, 0.8f, 0.0f);
        }

        // Draw power-ups
        for (const auto &powerUp : powerUps)
        {
            if (powerUp.active)
            {
                drawPowerUp(powerUp.x, powerUp.y, powerUp.type);
            }
        }

        // Draw score and active power-ups
        drawText(10, WINDOW_HEIGHT - 30, "Score: " + std::to_string(score));
        if (hasShield)
            drawText(10, WINDOW_HEIGHT - 50, "Shield Active!");
        if (hasSlowMotion)
            drawText(10, WINDOW_HEIGHT - 70, "Slow Motion Active!");
        if (hasDoublePoints)
            drawText(10, WINDOW_HEIGHT - 90, "Double Points Active!");
    }
    else if (gameState == GAME_OVER)
    {
        drawText(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 + 20, "Game Over!");
        drawText(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 10, "Press 'R' to Restart");
        drawText(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 40, "Score: " + std::to_string(score));
    }

    glutSwapBuffers();
}

void update(int value)
{
    if (gameState != PLAYING)
        return;

    // Update power-up timer
    if (activePowerUp != -1)
    {
        powerUpTimer--;
        if (powerUpTimer <= 0)
        {
            // Deactivate power-up
            switch (activePowerUp)
            {
            case SHIELD:
                hasShield = false;
                break;
            case SLOW_MOTION:
                hasSlowMotion = false;
                break;
            case DOUBLE_POINTS:
                hasDoublePoints = false;
                break;
            }
            activePowerUp = -1;
        }
    }

    // Update difficulty based on score
    if (score > 0 && score % DIFFICULTY_INTERVAL == 0)
    {
        float speedIncrease = (score / DIFFICULTY_INTERVAL) * SPEED_INCREASE;
        currentPipeSpeed = hasSlowMotion ? PIPE_SPEED + speedIncrease * 0.5f : // Half speed if slow motion is active
                               PIPE_SPEED + speedIncrease;
        currentGravity = GRAVITY + (score / DIFFICULTY_INTERVAL) * GRAVITY_INCREASE;
        float newGapHeight = GAP_HEIGHT - (score / DIFFICULTY_INTERVAL) * GAP_DECREASE;
        currentGapHeight = (newGapHeight < MIN_GAP_HEIGHT) ? MIN_GAP_HEIGHT : newGapHeight;
    }

    ballSpeed += currentGravity;
    ballY -= ballSpeed;

    // Add new pipe every 100 frames
    frameCount++;
    if (frameCount % 100 == 0)
    {
        Pipe newPipe;
        newPipe.x = WINDOW_WIDTH;
        newPipe.gapY = rand() % (WINDOW_HEIGHT - (int)currentGapHeight - 100) + 50;
        pipes.push_back(newPipe);

        // 20% chance to spawn a power-up
        if (rand() % 5 == 0)
        {
            PowerUp powerUp;
            powerUp.x = WINDOW_WIDTH;
            powerUp.y = rand() % (WINDOW_HEIGHT - 100) + 50;
            powerUp.type = rand() % 3; // Random power-up type
            powerUp.active = true;
            powerUps.push_back(powerUp);
        }
    }

    // Move pipes
    for (auto &pipe : pipes)
    {
        pipe.x -= currentPipeSpeed;
    }

    // Remove off-screen pipes
    if (!pipes.empty() && pipes.front().x + PIPE_WIDTH < 0)
        pipes.erase(pipes.begin());

    // Move and check power-ups
    for (auto it = powerUps.begin(); it != powerUps.end();)
    {
        it->x -= POWER_UP_SPEED;

        // Check collision with ball
        float dx = 100.0f - it->x;
        float dy = ballY - it->y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < ballRadius + POWER_UP_RADIUS && it->active)
        {
            // Activate power-up
            it->active = false;
            powerUpTimer = POWER_UP_DURATION;
            activePowerUp = it->type;

            switch (it->type)
            {
            case SHIELD:
                hasShield = true;
                break;
            case SLOW_MOTION:
                hasSlowMotion = true;
                break;
            case DOUBLE_POINTS:
                hasDoublePoints = true;
                break;
            }
            it = powerUps.erase(it);
        }
        else if (it->x + POWER_UP_RADIUS < 0)
        {
            it = powerUps.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Check collisions and score
    float ballLeft = 100 - ballRadius;
    float ballRight = 100 + ballRadius;
    float ballTop = ballY + ballRadius;
    float ballBottom = ballY - ballRadius;

    for (auto &pipe : pipes)
    {
        float pipeLeft = pipe.x;
        float pipeRight = pipe.x + PIPE_WIDTH;

        // Horizontal overlap
        if (ballRight > pipeLeft && ballLeft < pipeRight)
        {
            if (ballBottom < pipe.gapY || ballTop > pipe.gapY + currentGapHeight)
            {
                if (!hasShield)
                {
                    gameState = GAME_OVER;
                }
            }
        }

        // Score update
        if (pipe.x + PIPE_WIDTH < 100 && pipe.x + PIPE_WIDTH + PIPE_SPEED >= 100)
        {
            score += hasDoublePoints ? 2 : 1;
        }
    }

    if (ballY < 0 || ballY + 30 > WINDOW_HEIGHT)
        gameState = GAME_OVER;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 60 FPS
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
        exit(0); // ESC

    if (gameState == MENU && (key == 's' || key == 'S'))
    {
        resetGame();
        gameState = PLAYING;
        glutTimerFunc(16, update, 0);
    }

    if (gameState == PLAYING && key == 32)
    {
        ballSpeed = POWER;
    }

    if (gameState == GAME_OVER && (key == 'r' || key == 'R'))
    {
        resetGame();
        gameState = PLAYING;
        glutTimerFunc(16, update, 0);
    }
}

void init()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    srand(static_cast<unsigned int>(time(0)));
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Flappy Ball by Elmstaba");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
