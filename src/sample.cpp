#include <GL/freeglut.h>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <ctime>
#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>

// Constants
const float PI = 22.0f / 7.0f;

// WAV file header structure
struct WAVHeader
{
    char riff[4];
    unsigned int overallSize;
    char wave[4];
    char fmt[4];
    unsigned int fmtSize;
    unsigned short format;
    unsigned short channels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
};

// Sound effect IDs
ALuint jumpSound;
ALuint scoreSound;
ALuint powerupSound;
ALuint gameoverSound;
ALCdevice *device;
ALCcontext *context;

// Structs
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

struct Cloud
{
    float x, y;
    float scale;
    float speed;
};

struct Particle
{
    float x, y;
    float vx, vy;
    float life;
    float r, g, b, a;
};

// Forward declarations
void drawCircle(float x, float y, float radius);
void initClouds();
void drawCloud(float x, float y, float scale);
void drawParticle(const Particle &p);
void updateClouds();
void updateParticles();
void addParticles(float x, float y, float r, float g, float b);
void createExplosionEffect(float x, float y, float r, float g, float b);
void createScoreEffect(float x, float y);
void drawPowerUpTimer(float x, float y, float progress, int type);

// Sound functions
bool generateBeepSound(ALuint *buffer, float frequency, float duration)
{
    const int sampleRate = 44100;
    const int samples = duration * sampleRate;

    // Generate sine wave
    short *data = new short[samples];
    for (int i = 0; i < samples; i++)
    {
        float t = (float)i / sampleRate;
        data[i] = 32767 * sin(2.0f * PI * frequency * t);
    }

    // Create buffer
    alGenBuffers(1, buffer);
    alBufferData(*buffer, AL_FORMAT_MONO16, data, samples * sizeof(short), sampleRate);

    delete[] data;
    return true;
}

void playSound(ALuint source)
{
    alSourcePlay(source);
}

void initAudio()
{
    device = alcOpenDevice(NULL);
    if (!device)
        return;

    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    // Create sources for each sound
    alGenSources(1, &jumpSound);
    alGenSources(1, &scoreSound);
    alGenSources(1, &powerupSound);
    alGenSources(1, &gameoverSound);

    // Generate sound effects
    ALuint buffer;
    generateBeepSound(&buffer, 880.0f, 0.1f); // High pitch for jump
    alSourcei(jumpSound, AL_BUFFER, buffer);

    generateBeepSound(&buffer, 1000.0f, 0.1f); // Higher pitch for score
    alSourcei(scoreSound, AL_BUFFER, buffer);

    generateBeepSound(&buffer, 660.0f, 0.2f); // Medium pitch for power-up
    alSourcei(powerupSound, AL_BUFFER, buffer);

    generateBeepSound(&buffer, 440.0f, 0.5f); // Low pitch for game over
    alSourcei(gameoverSound, AL_BUFFER, buffer);
}

void cleanupAudio()
{
    alDeleteSources(1, &jumpSound);
    alDeleteSources(1, &scoreSound);
    alDeleteSources(1, &powerupSound);
    alDeleteSources(1, &gameoverSound);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

enum GameState
{
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

// Game mode enum
enum GameMode
{
    MODE_MENU,
    MODE_EASY,
    MODE_MEDIUM,
    MODE_HARD,
    MODE_TIME_TRIAL
};

// Difficulty settings for each mode
struct DifficultySettings
{
    float pipeSpeed;
    float gapHeight;
    float gravity;
    int spawnInterval;
    float speedIncrease;
    float gapDecrease;
    float gravityIncrease;
} modes[] = {
    {2.0f, 250.0f, 0.3f, 120, 0.1f, 3.0f, 0.01f}, // Easy
    {3.0f, 200.0f, 0.4f, 100, 0.2f, 5.0f, 0.02f}, // Medium
    {4.0f, 150.0f, 0.5f, 80, 0.3f, 7.0f, 0.03f},  // Hard
    {3.0f, 200.0f, 0.4f, 100, 0.2f, 5.0f, 0.02f}  // Time Trial (starts at medium)
};

GameState gameState = MENU;
GameMode currentMode = MODE_MENU;
float timeTrialTimer = 0.0f; // For Time Trial mode
int currentSpawnInterval = 100;

const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
const int PIPE_WIDTH = 80;
const int GAP_HEIGHT = 200;
const float GRAVITY = 0.4f;
const float POWER = -8.0f;
const float PIPE_SPEED = 3.0f;
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
const int INVINCIBILITY_DURATION = 120; // 2 seconds of invincibility after losing a life

// Visual enhancement constants
const int MAX_CLOUDS = 5;
const int MAX_PARTICLES = 200; // Increased for more particles
const float CLOUD_MIN_SPEED = 0.5f;
const float CLOUD_MAX_SPEED = 1.5f;
const float PARTICLE_LIFE = 60.0f; // frames
const float PARTICLE_SPEED = 5.0f;
const float EXPLOSION_SPEED = 8.0f;      // Speed for explosion particles
const int EXPLOSION_PARTICLE_COUNT = 20; // Number of particles in explosion
const int SCORE_PARTICLE_COUNT = 10;     // Number of particles for scoring

// State variables for game objects
const int INITIAL_LIVES = 3; // Number of lives at game start
int lives = INITIAL_LIVES;   // Current number of lives
int invincibilityTimer = 0;  // Timer for invincibility after losing a life

std::vector<PowerUp> powerUps;
std::vector<Cloud> clouds;
std::vector<Particle> particles;

// Power-up state variables
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

// Function to handle losing a life
void loseLife()
{
    lives--;
    if (lives <= 0)
    {
        gameState = GAME_OVER;
        playSound(gameoverSound);
    }
    else
    {
        // Reset ball position and give temporary invincibility
        ballY = WINDOW_HEIGHT / 2;
        ballSpeed = 0;
        invincibilityTimer = INVINCIBILITY_DURATION;
        playSound(gameoverSound);

        // Clear nearby obstacles
        auto it = pipes.begin();
        while (it != pipes.end())
        {
            if (it->x < WINDOW_WIDTH / 2)
            {
                it = pipes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

void resetGame()
{
    // Reset ball state
    ballY = WINDOW_HEIGHT / 2;
    ballSpeed = 0;

    // Reset game progress
    score = 0;
    lives = INITIAL_LIVES;
    invincibilityTimer = 0;
    timeTrialTimer = 0.0f;

    // Clear game objects
    pipes.clear();
    powerUps.clear();
    particles.clear();

    // Reset power-up states
    hasShield = false;
    hasSlowMotion = false;
    hasDoublePoints = false;
    powerUpTimer = 0;
    activePowerUp = -1;

    // Reset counters
    frameCount = 0;

    // Set initial difficulty based on mode
    if (currentMode != MODE_MENU)
    {
        int modeIndex = currentMode - 1; // -1 because MODE_MENU is 0
        currentPipeSpeed = modes[modeIndex].pipeSpeed;
        currentGapHeight = modes[modeIndex].gapHeight;
        currentGravity = modes[modeIndex].gravity;
        currentSpawnInterval = modes[modeIndex].spawnInterval;
    }
    else
    {
        currentPipeSpeed = PIPE_SPEED;
        currentGapHeight = GAP_HEIGHT;
        currentGravity = GRAVITY;
        currentSpawnInterval = 100;
    }

    // Reset power-ups
    powerUps.clear();
    particles.clear();
    hasShield = false;
    hasSlowMotion = false;
    hasDoublePoints = false;
    powerUpTimer = 0;
    activePowerUp = -1;

    // Initialize visual effects
    initClouds();
}

void drawCircle(float x, float y, float radius)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++)
    {
        float angle = i * PI / 180;
        glVertex2f(x + cos(angle) * radius, y + sin(angle) * radius);
    }
    glEnd();
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

        // Add flickering effect during invincibility
        if (invincibilityTimer > 0)
        {
            if ((invincibilityTimer / 5) % 2)
            { // Flicker every 5 frames
                r = 1.0f;
                g = 1.0f;
                b = 1.0f;
            }
        }

        glColor3f(r, g, b);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

void drawPowerUp(float x, float y, int type)
{
    static float animationTime = 0.0f;
    animationTime += 0.05f;
    float pulseScale = 1.0f + 0.1f * sin(animationTime); // Pulsing effect
    float outerGlow = POWER_UP_RADIUS * 1.3f;
    float innerRadius = POWER_UP_RADIUS * 0.7f;

    // Draw outer glow
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++)
    {
        float angle = i * (PI / 180);
        float dx = cos(angle) * outerGlow * pulseScale;
        float dy = sin(angle) * outerGlow * pulseScale;

        switch (type)
        {
        case SHIELD:
            glColor4f(0.3f, 0.3f, 1.0f, 0.2f); // Blue glow
            break;
        case SLOW_MOTION:
            glColor4f(0.3f, 1.0f, 0.3f, 0.2f); // Green glow
            break;
        case DOUBLE_POINTS:
            glColor4f(1.0f, 1.0f, 0.3f, 0.2f); // Yellow glow
            break;
        }
        glVertex2f(x + dx, y + dy);
    }
    glEnd();

    // Draw main power-up circle
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++)
    {
        float angle = i * (PI / 180);
        float dx = cos(angle) * POWER_UP_RADIUS * pulseScale;
        float dy = sin(angle) * POWER_UP_RADIUS * pulseScale;

        switch (type)
        {
        case SHIELD:
            glColor4f(0.0f, 0.0f, 1.0f, 0.8f); // Blue
            break;
        case SLOW_MOTION:
            glColor4f(0.0f, 1.0f, 0.0f, 0.8f); // Green
            break;
        case DOUBLE_POINTS:
            glColor4f(1.0f, 1.0f, 0.0f, 0.8f); // Yellow
            break;
        }
        glVertex2f(x + dx, y + dy);
    }
    glEnd();

    // Draw inner symbol based on power-up type
    glBegin(GL_POLYGON);
    switch (type)
    {
    case SHIELD: // Shield symbol
        for (int i = 0; i < 360; i++)
        {
            float angle = i * (PI / 180);
            float dx = cos(angle) * innerRadius;
            float dy = sin(angle) * innerRadius;
            if (dy > -innerRadius * 0.3f) // Create shield shape
            {
                glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
                glVertex2f(x + dx, y + dy);
            }
        }
        break;

    case SLOW_MOTION: // Clock symbol
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        for (int i = 0; i < 12; i++) // Clock marks
        {
            float angle = i * (PI / 6);
            float dx1 = cos(angle) * innerRadius;
            float dy1 = sin(angle) * innerRadius;
            float dx2 = cos(angle) * (innerRadius * 0.8f);
            float dy2 = sin(angle) * (innerRadius * 0.8f);
            glVertex2f(x + dx1, y + dy1);
            glVertex2f(x + dx2, y + dy2);
        }
        break;

    case DOUBLE_POINTS: // Star symbol
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        for (int i = 0; i < 5; i++)
        {
            float angle = i * (2 * PI / 5) - PI / 2;
            float dx = cos(angle) * innerRadius;
            float dy = sin(angle) * innerRadius;
            glVertex2f(x + dx, y + dy);

            // Inner points of the star
            angle += PI / 5;
            dx = cos(angle) * (innerRadius * 0.4f);
            dy = sin(angle) * (innerRadius * 0.4f);
            glVertex2f(x + dx, y + dy);
        }
        break;
    }
    glEnd();
}

void drawText(float x, float y, const std::string &text, void *font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (char c : text)
    {
        glutBitmapCharacter(font, c);
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Draw gradient background
    glDisable(GL_BLEND);
    glBegin(GL_QUADS);
    glColor3f(0.6f, 0.8f, 1.0f); // Top color (lighter blue)
    glVertex2f(0, WINDOW_HEIGHT);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glColor3f(0.4f, 0.6f, 0.9f); // Bottom color (darker blue)
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(0, 0);
    glEnd();

    // Enable alpha blending for overlays and sprites
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw clouds
    for (const auto &cloud : clouds)
    {
        drawCloud(cloud.x, cloud.y, cloud.scale);
    }

    // Draw particles
    for (const auto &particle : particles)
    {
        drawParticle(particle);
    }

    if (gameState == MENU)
    {
        // Draw semi-transparent overlay
        glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glVertex2f(0, WINDOW_HEIGHT);
        glEnd();

        // Draw title with shadow effect
        glColor3f(0.8f, 0.0f, 0.0f);
        drawText(WINDOW_WIDTH / 2 - 98, WINDOW_HEIGHT - 50, "FLAPPY BALL", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(1.0f, 0.0f, 0.0f);
        drawText(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT - 52, "FLAPPY BALL", GLUT_BITMAP_TIMES_ROMAN_24);

        // Start from a higher position and use consistent spacing
        float startY = WINDOW_HEIGHT - 120; // Start lower than the title
        float spacing = 25;                 // Reduced spacing

        // Game Modes Section
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(WINDOW_WIDTH / 2 - 150, startY, "Game Modes:", GLUT_BITMAP_HELVETICA_18);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing, "1: Easy Mode - Wider gaps, slower speed", GLUT_BITMAP_HELVETICA_12);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing * 2, "2: Medium Mode - Balanced difficulty", GLUT_BITMAP_HELVETICA_12);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing * 3, "3: Hard Mode - Narrow gaps, faster speed", GLUT_BITMAP_HELVETICA_12);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing * 4, "4: Time Trial - Progressive difficulty", GLUT_BITMAP_HELVETICA_12);

        // Controls Section
        startY = startY - spacing * 5.5; // Reduced gap between sections
        drawText(WINDOW_WIDTH / 2 - 150, startY, "Controls:", GLUT_BITMAP_HELVETICA_18);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing, "SPACE - Jump/Flap & Start Game", GLUT_BITMAP_HELVETICA_12);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing * 2, "P - Pause/Resume", GLUT_BITMAP_HELVETICA_12);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing * 3, "ESC - Return to Menu/Quit", GLUT_BITMAP_HELVETICA_12);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing * 4, "R - Restart (after game over)", GLUT_BITMAP_HELVETICA_12);

        // Power-ups Section
        startY = startY - spacing * 5.5; // Reduced gap between sections
        drawText(WINDOW_WIDTH / 2 - 150, startY, "Power-ups:", GLUT_BITMAP_HELVETICA_18);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing, "Blue Shield - Temporary invincibility", GLUT_BITMAP_HELVETICA_12);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing * 2, "Green Clock - Slows down obstacles", GLUT_BITMAP_HELVETICA_12);
        drawText(WINDOW_WIDTH / 2 - 150, startY - spacing * 3, "Yellow Star - Double points", GLUT_BITMAP_HELVETICA_12);

        // Add "Press SPACE to Start" message at the bottom
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow color
        drawText(WINDOW_WIDTH / 2 - 100, 100, "Press SPACE to Start Easy Mode", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == PLAYING || gameState == PAUSED)
    {
        float ballX = 100.0f;

        // Draw power-up effect on ball if shield is active
        if (hasShield)
        {
            static float shieldAnimTime = 0.0f;
            shieldAnimTime += 0.03f;
            float pulseScale = 1.0f + 0.1f * sin(shieldAnimTime);

            // Outer glow
            glBegin(GL_POLYGON);
            glColor4f(0.2f, 0.2f, 1.0f, 0.2f);
            for (int i = 0; i < 360; i++)
            {
                float angle = i * (PI / 180);
                float dx = cos(angle) * (ballRadius + 8) * pulseScale;
                float dy = sin(angle) * (ballRadius + 8) * pulseScale;
                glVertex2f(ballX + dx, ballY + dy);
            }
            glEnd();

            // Shield ring
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i <= 360; i += 5)
            {
                float angle = i * (PI / 180);
                float wave = sin(angle * 6 + shieldAnimTime * 2) * 2; // Wavy effect
                float radius = (ballRadius + 5) * pulseScale + wave;
                float dx = cos(angle) * radius;
                float dy = sin(angle) * radius;
                float alpha = 0.8f + 0.2f * sin(angle * 3 + shieldAnimTime); // Shimmer effect
                glColor4f(0.0f, 0.0f, 1.0f, alpha);
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

        // Draw score/time and active power-ups
        if (currentMode == MODE_TIME_TRIAL)
        {
            drawText(10, WINDOW_HEIGHT - 30, "Time: " + std::to_string(static_cast<int>(timeTrialTimer)) + "s");
        }
        else
        {
            drawText(10, WINDOW_HEIGHT - 30, "Score: " + std::to_string(score));
        }
        drawText(10, WINDOW_HEIGHT - 50, "Lives: " + std::to_string(lives));

        // Draw power-up timers
        float timerY = WINDOW_HEIGHT - 80;
        float timerSpacing = 60.0f;

        if (hasShield)
        {
            drawPowerUpTimer(40, timerY, (float)powerUpTimer / POWER_UP_DURATION, SHIELD);
        }
        if (hasSlowMotion)
        {
            drawPowerUpTimer(40 + timerSpacing, timerY, (float)powerUpTimer / POWER_UP_DURATION, SLOW_MOTION);
        }
        if (hasDoublePoints)
        {
            drawPowerUpTimer(40 + timerSpacing * 2, timerY, (float)powerUpTimer / POWER_UP_DURATION, DOUBLE_POINTS);
        }

        // Show pause message
        if (gameState == PAUSED)
        {
            // Draw semi-transparent dark overlay
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            glBegin(GL_QUADS);
            glVertex2f(0, 0);
            glVertex2f(WINDOW_WIDTH, 0);
            glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
            glVertex2f(0, WINDOW_HEIGHT);
            glEnd();

            // Draw pause text
            glColor3f(1.0f, 1.0f, 1.0f); // White text
            drawText(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 + 20, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
            drawText(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 20, "Press 'P' to Resume", GLUT_BITMAP_HELVETICA_18);
        }
    }
    else if (gameState == GAME_OVER)
    {
        // Draw semi-transparent dark overlay
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        drawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 0.0f, 0.0f);

        float centerY = WINDOW_HEIGHT / 2;

        // Draw Game Over text with shadow effect
        glColor3f(0.8f, 0.0f, 0.0f);
        drawText(WINDOW_WIDTH / 2 - 58, centerY + 22, "Game Over!", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(1.0f, 0.0f, 0.0f);
        drawText(WINDOW_WIDTH / 2 - 60, centerY + 20, "Game Over!", GLUT_BITMAP_TIMES_ROMAN_24);

        // Draw score/time based on game mode
        if (currentMode == MODE_TIME_TRIAL)
        {
            drawText(WINDOW_WIDTH / 2 - 100, centerY - 20, "Time Survived: " + std::to_string(static_cast<int>(timeTrialTimer)) + "s");
        }
        else
        {
            drawText(WINDOW_WIDTH / 2 - 60, centerY - 20, "Score: " + std::to_string(score));
        }

        // Draw game mode info
        std::string modeText;
        switch (currentMode)
        {
        case MODE_EASY:
            modeText = "Easy Mode";
            break;
        case MODE_MEDIUM:
            modeText = "Medium Mode";
            break;
        case MODE_HARD:
            modeText = "Hard Mode";
            break;
        case MODE_TIME_TRIAL:
            modeText = "Time Trial Mode";
            break;
        default:
            modeText = "Unknown Mode";
        }
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(WINDOW_WIDTH / 2 - 60, centerY - 40, modeText);

        // Draw options with better visibility
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow color for better visibility
        drawText(WINDOW_WIDTH / 2 - 100, centerY - 80, "Press 'R' to Restart");
        drawText(WINDOW_WIDTH / 2 - 100, centerY - 100, "Press 'M' for Main Menu");
        drawText(WINDOW_WIDTH / 2 - 100, centerY - 120, "Press 'ESC' to Quit");
    }

    glutSwapBuffers();
}

void update([[maybe_unused]] int value)
{
    if (gameState != PLAYING)
    {
        if (gameState == PAUSED)
        {
            glutPostRedisplay(); // Keep updating display while paused
        }
        return;
    }

    // Update visual effects
    updateClouds();
    updateParticles();

    // Update invincibility timer
    if (invincibilityTimer > 0)
    {
        invincibilityTimer--;
    }

    // Update power-up timer
    if (activePowerUp != -1 && powerUpTimer > 0)
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
            powerUpTimer = 0;
        }
    }

    // Update Time Trial timer and difficulty
    if (currentMode == MODE_TIME_TRIAL)
    {
        // Track previous time to detect 30 second intervals
        static int lastDifficultyIncrease = 0;
        timeTrialTimer += 0.016f; // 16ms per frame

        // Increase difficulty every 30 seconds
        int currentTime = static_cast<int>(timeTrialTimer);
        if (currentTime >= lastDifficultyIncrease + 30)
        {
            lastDifficultyIncrease = currentTime;

            // Use gentler difficulty scaling
            int modeIndex = MODE_MEDIUM - 1;
            currentPipeSpeed += modes[modeIndex].speedIncrease * 0.5f;                     // Half speed increase
            currentGravity += modes[modeIndex].gravityIncrease * 0.3f;                     // 30% gravity increase
            float newGapHeight = currentGapHeight - (modes[modeIndex].gapDecrease * 0.7f); // 70% gap decrease
            currentGapHeight = (newGapHeight < MIN_GAP_HEIGHT) ? MIN_GAP_HEIGHT : newGapHeight;

            printf("Time Trial difficulty increased at %d seconds\n", currentTime);
            printf("New values - Speed: %.2f, Gravity: %.2f, Gap: %.2f\n",
                   currentPipeSpeed, currentGravity, currentGapHeight);
        }
    }
    // Update difficulty based on score for other modes
    else if (score > 0 && score % DIFFICULTY_INTERVAL == 0)
    {
        int modeIndex = currentMode - 1;
        float speedIncrease = (score / DIFFICULTY_INTERVAL) * modes[modeIndex].speedIncrease;
        currentPipeSpeed = hasSlowMotion ? modes[modeIndex].pipeSpeed + speedIncrease * 0.5f : // Half speed if slow motion is active
                               modes[modeIndex].pipeSpeed + speedIncrease;
        currentGravity = modes[modeIndex].gravity + (score / DIFFICULTY_INTERVAL) * modes[modeIndex].gravityIncrease;
        float newGapHeight = modes[modeIndex].gapHeight - (score / DIFFICULTY_INTERVAL) * modes[modeIndex].gapDecrease;
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
            // Deactivate any existing power-up
            if (activePowerUp != -1)
            {
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
            }

            // Activate new power-up
            it->active = false;
            powerUpTimer = POWER_UP_DURATION;
            activePowerUp = it->type;

            switch (it->type)
            {
            case SHIELD:
                hasShield = true;
                addParticles(100, ballY, 0.0f, 0.0f, 1.0f); // Blue particles for shield
                playSound(powerupSound);
                break;
            case SLOW_MOTION:
                hasSlowMotion = true;
                addParticles(100, ballY, 0.0f, 1.0f, 0.0f); // Green particles for slow motion
                playSound(powerupSound);
                break;
            case DOUBLE_POINTS:
                hasDoublePoints = true;
                addParticles(100, ballY, 1.0f, 1.0f, 0.0f); // Yellow particles for double points
                playSound(powerupSound);
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
                    // Create red explosion effect on impact
                    createExplosionEffect(100, ballY, 1.0f, 0.2f, 0.2f);
                    loseLife(); // Use loseLife function
                }
            }
        }

        // Score update
        if (pipe.x + PIPE_WIDTH < 100 && pipe.x + PIPE_WIDTH + currentPipeSpeed >= 100)
        {
            score += hasDoublePoints ? 2 : 1;
            // Create golden score effect
            createScoreEffect(100, ballY);
            playSound(scoreSound); // Play score sound
        }
    }

    if (ballY < 0 || ballY + 30 > WINDOW_HEIGHT)
    {
        // Create red explosion effect on boundary collision
        createExplosionEffect(100, ballY, 1.0f, 0.2f, 0.2f);
        loseLife(); // Use loseLife function
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 60 FPS
}

void keyboard(unsigned char key, [[maybe_unused]] int x, [[maybe_unused]] int y)
{
    // Handle game over state first
    if (gameState == GAME_OVER)
    {
        if (key == 'r' || key == 'R')
        {
            resetGame();
            gameState = PLAYING;
            glutTimerFunc(16, update, 0);
            glutPostRedisplay();
            return;
        }
        else if (key == 'm' || key == 'M')
        {
            resetGame();
            gameState = MENU;
            currentMode = MODE_MENU;
            glutPostRedisplay();
            return;
        }
        else if (key == 27) // ESC
        {
            exit(0);
        }
    }

    // Handle other states
    if (key == 27) // ESC
    {
        if (gameState == PLAYING || gameState == PAUSED)
        {
            gameState = MENU;
            currentMode = MODE_MENU;
            glutPostRedisplay();
            return;
        }
        else if (gameState == MENU)
        {
            exit(0);
        }
    }

    if (gameState == MENU)
    {
        switch (key)
        {
        case '1':
            currentMode = MODE_EASY;
            gameState = PLAYING;
            resetGame();
            glutTimerFunc(16, update, 0);
            break;
        case '2':
            currentMode = MODE_MEDIUM;
            gameState = PLAYING;
            resetGame();
            glutTimerFunc(16, update, 0);
            break;
        case '3':
            currentMode = MODE_HARD;
            gameState = PLAYING;
            resetGame();
            glutTimerFunc(16, update, 0);
            break;
        case '4':
            currentMode = MODE_TIME_TRIAL;
            gameState = PLAYING;
            resetGame();
            glutTimerFunc(16, update, 0);
            break;
        }
    }

    if (key == 'p' || key == 'P')
    {
        if (gameState == PLAYING)
        {
            gameState = PAUSED;
        }
        else if (gameState == PAUSED)
        {
            gameState = PLAYING;
            glutTimerFunc(16, update, 0);
        }
    }

    if (key == 32) // Spacebar
    {
        if (gameState == MENU)
        {
            // Start the game in Easy mode when space is pressed in menu
            currentMode = MODE_EASY;
            gameState = PLAYING;
            resetGame();
            glutTimerFunc(16, update, 0);
        }
        else if (gameState == PLAYING)
        {
            ballSpeed = POWER;
            // Add jump particles
            addParticles(100, ballY, 1.0f, 1.0f, 1.0f); // White particles for jumping
            playSound(jumpSound);                       // Play jump sound
        }
    }

    // Game over handling moved to the top of the function
}

void init()
{
    // Set up OpenGL state
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Initialize game state
    gameState = MENU;
    currentMode = MODE_MENU;

    // Initialize randomization
    srand(static_cast<unsigned int>(time(0)));

    // Initialize game systems
    initClouds();
    initAudio();

    // Reset game objects
    resetGame();
}

void initClouds()
{
    clouds.clear();
    for (int i = 0; i < MAX_CLOUDS; i++)
    {
        Cloud cloud;
        cloud.x = rand() % WINDOW_WIDTH;
        cloud.y = rand() % (WINDOW_HEIGHT / 2);
        cloud.scale = 0.5f + (rand() % 100) / 100.0f;
        cloud.speed = CLOUD_MIN_SPEED + (rand() % 100) * (CLOUD_MAX_SPEED - CLOUD_MIN_SPEED) / 100.0f;
        clouds.push_back(cloud);
    }
}

// Function to create an explosion effect
void createExplosionEffect(float x, float y, float r, float g, float b)
{
    for (int i = 0; i < EXPLOSION_PARTICLE_COUNT; i++)
    {
        if (particles.size() < MAX_PARTICLES)
        {
            Particle p;
            p.x = x;
            p.y = y;
            // Create a circular explosion pattern
            float angle = (float)i / EXPLOSION_PARTICLE_COUNT * 2.0f * PI;
            p.vx = cos(angle) * EXPLOSION_SPEED;
            p.vy = sin(angle) * EXPLOSION_SPEED;
            p.life = PARTICLE_LIFE;
            p.r = r;
            p.g = g;
            p.b = b;
            p.a = 1.0f;
            particles.push_back(p);
        }
    }
}

// Function to create score collection effect
void createScoreEffect(float x, float y)
{
    for (int i = 0; i < SCORE_PARTICLE_COUNT; i++)
    {
        if (particles.size() < MAX_PARTICLES)
        {
            Particle p;
            p.x = x;
            p.y = y;
            // Create upward moving particles
            float angle = (PI / 4.0f) + (PI / 2.0f) * ((float)rand() / RAND_MAX); // Spread between 45 and 135 degrees
            float speed = PARTICLE_SPEED * (0.5f + ((float)rand() / RAND_MAX));   // Random speed variation
            p.vx = cos(angle) * speed;
            p.vy = sin(angle) * speed;
            p.life = PARTICLE_LIFE;
            // Gold color with slight variation
            p.r = 1.0f;
            p.g = 0.8f + ((float)rand() / RAND_MAX) * 0.2f;
            p.b = 0.0f;
            p.a = 1.0f;
            particles.push_back(p);
        }
    }
}

void addParticles(float x, float y, float r, float g, float b)
{
    for (int i = 0; i < 5; i++)
    {
        if (particles.size() < MAX_PARTICLES)
        {
            Particle p;
            p.x = x;
            p.y = y;
            float angle = (rand() % 360) * 3.14159f / 180.0f;
            p.vx = cos(angle) * PARTICLE_SPEED;
            p.vy = sin(angle) * PARTICLE_SPEED;
            p.life = PARTICLE_LIFE;
            p.r = r;
            p.g = g;
            p.b = b;
            p.a = 1.0f;
            particles.push_back(p);
        }
    }
}

void updateClouds()
{
    for (auto &cloud : clouds)
    {
        cloud.x -= cloud.speed;
        if (cloud.x + 100 < 0)
        {
            cloud.x = WINDOW_WIDTH + 100;
            cloud.y = rand() % (WINDOW_HEIGHT / 2);
            cloud.scale = 0.5f + (rand() % 100) / 100.0f;
        }
    }
}

void updateParticles()
{
    for (auto it = particles.begin(); it != particles.end();)
    {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += 0.2f;  // Stronger gravity effect
        it->vx *= 0.99f; // Air resistance
        it->life--;
        it->a = (it->life / PARTICLE_LIFE) * (it->life / PARTICLE_LIFE); // Quadratic fade out

        if (it->life <= 0)
        {
            it = particles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void drawCloud(float x, float y, float scale)
{
    glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
    float size = 30.0f * scale;

    // Draw multiple circles to create cloud shape
    for (int i = 0; i < 3; i++)
    {
        float offsetX = i * size * 0.7f;
        float offsetY = (i % 2) * size * 0.2f;
        drawCircle(x + offsetX, y + offsetY, size);
    }
}

void drawParticle(const Particle &p)
{
    glColor4f(p.r, p.g, p.b, p.a);
    drawCircle(p.x, p.y, 3.0f);
}

// Draw circular timer for power-ups
void drawPowerUpTimer(float x, float y, float progress, int type)
{
    const float outerRadius = 25.0f;
    const float innerRadius = 20.0f;

    // Draw outer circle (background)
    glBegin(GL_TRIANGLE_STRIP);
    glColor4f(0.2f, 0.2f, 0.2f, 0.5f);
    for (int i = 0; i <= 360; i++)
    {
        float angle = i * PI / 180.0f;
        float cos_val = cos(angle);
        float sin_val = sin(angle);

        glVertex2f(x + cos_val * innerRadius, y + sin_val * innerRadius);
        glVertex2f(x + cos_val * outerRadius, y + sin_val * outerRadius);
    }
    glEnd();

    // Draw progress arc
    glBegin(GL_TRIANGLE_STRIP);
    switch (type)
    {
    case SHIELD:
        glColor4f(0.0f, 0.0f, 1.0f, 0.8f);
        break;
    case SLOW_MOTION:
        glColor4f(0.0f, 1.0f, 0.0f, 0.8f);
        break;
    case DOUBLE_POINTS:
        glColor4f(1.0f, 1.0f, 0.0f, 0.8f);
        break;
    }

    int progressDegrees = progress * 360;
    for (int i = 0; i <= progressDegrees; i++)
    {
        float angle = i * PI / 180.0f;
        float cos_val = cos(angle);
        float sin_val = sin(angle);

        glVertex2f(x + cos_val * innerRadius, y + sin_val * innerRadius);
        glVertex2f(x + cos_val * outerRadius, y + sin_val * outerRadius);
    }
    glEnd();

    // Draw icon in the middle based on power-up type
    glBegin(GL_POLYGON);
    switch (type)
    {
    case SHIELD:
        // Draw shield icon
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        for (int i = 0; i < 360; i++)
        {
            if (i * PI / 180.0f > PI * 0.2f)
            { // Create shield shape
                float angle = i * PI / 180.0f;
                glVertex2f(x + cos(angle) * (innerRadius * 0.6f),
                           y + sin(angle) * (innerRadius * 0.6f));
            }
        }
        break;

    case SLOW_MOTION:
        // Draw clock icon
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        for (int i = 0; i < 12; i++)
        {
            float angle = i * PI / 6.0f;
            glVertex2f(x + cos(angle) * (innerRadius * 0.6f),
                       y + sin(angle) * (innerRadius * 0.6f));
        }
        // Draw clock hands
        glVertex2f(x, y);
        glVertex2f(x + innerRadius * 0.4f, y);
        glVertex2f(x, y + innerRadius * 0.3f);
        break;

    case DOUBLE_POINTS:
        // Draw star icon
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        for (int i = 0; i < 5; i++)
        {
            float angle = i * 2 * PI / 5.0f;
            glVertex2f(x + cos(angle) * (innerRadius * 0.6f),
                       y + sin(angle) * (innerRadius * 0.6f));
            angle += PI / 5.0f;
            glVertex2f(x + cos(angle) * (innerRadius * 0.3f),
                       y + sin(angle) * (innerRadius * 0.3f));
        }
        break;
    }
    glEnd();

    // Draw remaining time in seconds, capped at 99s
    char timeStr[16]; // Increased buffer size to safely handle any integer
    int secondsLeft = std::min(99, (powerUpTimer / 60) + 1);
    snprintf(timeStr, sizeof(timeStr), "%ds", secondsLeft);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(x - 10, y - innerRadius - 20, timeStr);
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
    glutTimerFunc(16, update, 0); // Start the update timer for 60 FPS

    atexit(cleanupAudio); // Register cleanup function

    glutMainLoop();
    return 0;
}
