#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <random>
#include <glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <Windows.h>
#define FPS 60
#define M_PI 3.14159
#define DEG2RAD(a) (a * 0.0174532925)
#include <SDL.h>
#include <SDL_mixer.h>

// Model Variables
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_warehouse1;
Model_3DS model_zombie;
Model_3DS model_zombie2;
Model_3DS model_player;
Model_3DS model_clock;
Model_3DS model_crate;
Model_3DS model_crate2;
Model_3DS model_heart;
Model_3DS model_key;
Model_3DS model_key2;
Model_3DS model_door;
Model_3DS model_door2;
Model_3DS model_warehouse2;

GLuint tex;
GLTexture tex_ground;

Mix_Music* hypeMusic = NULL;
Mix_Chunk* damage = NULL;
Mix_Chunk* lesgo = NULL;
Mix_Chunk* gameWin = NULL;
Mix_Chunk* levelUp = NULL;
Mix_Chunk* yummers = NULL;
Mix_Chunk* clockSound = NULL;

#define TO_RADIANS 3.14/180.0

//width and height of the window ( Aspect ratio 16:9 )
const int width = 1280;
const int height = 720;

float pitch = 0.0, yaw = 0.0;
float camX = 0.0, camZ = 10.0;
float playerx = 0.0, playerz = 10.0;

bool firstPerson = true;
bool flashlight = true;
double clockAngle = 0;
double zombieAngle = 0;
double zombieX = 10;
double zombieZ = 10;

int clockX[] = { 0, 0, };
int clockZ[] = { 0, -7 };
double keyX = 0;
double keyZ = 0;
int countFPS = 0;
int gameTimer = 200;
int level = 1;
int score = 1000;
int playerHealth = 100;
bool keyCollected = false;
bool won = false;
bool gameOver = false;

double lampAngle = 0;
bool upLamp = true;

float lightbulbx = 0.0;
float lighthintensitybulb = 0.1;

void display();
void reshape(int w, int h);
void timer(int);
void passive_motion(int, int);
void camera();
void keyboard(unsigned char key, int x, int y);
void keyboard_up(unsigned char key, int x, int y);
void LoadAssets(void);

struct Motion
{
    bool Forward, Backward, Left, Right;
};

Motion motion = { false,false,false,false };

void DrawLightBulb() {
  
    // Draw the plate-like structure above the bulb
    glPushMatrix();
    // translated by tot 8
    glTranslated(0, 8.75, 0);
    glRotated(lampAngle, 0, 0, 1);
    //glTranslated(0, 8, 0);
    glTranslated(0, -0.75, 0);
    glPushMatrix();
    glColor3f(0.9, 0.9, 0.9);
    glTranslated(0, -0.8, 0); // 7. 6
    glRotated(-90,1,0,0);
    glScaled(2, 2.7, 2);
    //glRotated(-90, 1, 0, 0);
    glutWireCone(0.1, 0.1, 100, 100);
    glPopMatrix();
    glPushMatrix();
    glColor3f(0, 0, 0);
    glTranslated(0, 0, 0); // 8.5
    glScaled(0.05, 1.5, 0.05);
    glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
    glColor3f(1, 1, 0.2);
    glTranslated(0, -1.0, 0); // 7.38
    glutWireSphere(0.27, 200, 200);
    glPopMatrix();

    glPopMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
}

void getPosThird() {
    double dist = sqrt(1 + (1.6 * 1.6));
    double actX = camX - dist * sin(DEG2RAD(yaw));
    double actZ = camZ - dist * cos(DEG2RAD(yaw));
}

void InitMaterial()
{
    // Enable Material Tracking
    glEnable(GL_COLOR_MATERIAL);

    // Sich will be assigneet Material Properties whd by glColor
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set Material's Specular Color
    // Will be applied to all objects
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

    // Set Material's Shine value (0->128)
    GLfloat shininess[] = { 96.0f };
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

void simulateKeyPress(char key) {
    // Simulate a key press
    keybd_event(VkKeyScan(key), 0, KEYEVENTF_SCANCODE, 0);

    // Simulate a key release
    keybd_event(VkKeyScan(key), 0, KEYEVENTF_KEYUP, 0);
}


void init()
{
    glutSetCursor(GLUT_CURSOR_NONE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glutWarpPointer(width / 2, height / 2);
}

void clockAnim(int x) {
    clockAngle += 5;
    glutPostRedisplay();
    glutTimerFunc(100, clockAnim, 0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Hunted Warehouse");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
   //music = Mix_LoadMUS("minecraftMusic.wav");
    hypeMusic = Mix_LoadMUS("music.wav");
    Mix_PlayMusic(hypeMusic, -1);
    damage = Mix_LoadWAV("damage.wav");
    lesgo = Mix_LoadWAV("lesgo.wav");
    gameWin = Mix_LoadWAV("gameWin.wav");
    levelUp = Mix_LoadWAV("level2.wav");
    yummers = Mix_LoadWAV("yummers.wav");
    clockSound = Mix_LoadWAV("marioPowerUp.wav");


    init();
    InitMaterial();
    LoadAssets();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(passive_motion);
    glutTimerFunc(0, timer, 0);    //more info about this is given below at definition of timer()
    // glutTimerFunc(0, clockAnim, 0);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboard_up);
    glEnable(GL_LIGHTING);
   // glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);


    glutMainLoop();
    return 0;
}

/* This function just draws the scene. I used Texture mapping to draw
   a chessboard like surface. If this is too complicated for you ,
   you can just use a simple quadrilateral */

void setupLights() {
    GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
    GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
    GLfloat shininess[] = { 50 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

    GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
    GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);



    GLfloat l1Diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat l1Ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat l1Position[] = { 0, 00.0f, 10.0f, true };
    GLfloat l1Direction[] = { 0.0, 0.0, -1.0 };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, l1Diffuse);
    glLightfv(GL_LIGHT1, GL_AMBIENT, l1Ambient);
    glLightfv(GL_LIGHT1, GL_POSITION, l1Position);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 90.0);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, l1Direction);

   
    GLfloat light_diffuse[] = { 1* lighthintensitybulb, 1* lighthintensitybulb, 1* lighthintensitybulb, 0 };
    GLfloat pos[] = { lightbulbx, 8, -0.4, 1 };

   // glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT2, GL_POSITION, pos);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 60.0f);

}

void RenderGround()
{
    glDisable(GL_LIGHTING);	// Disable lighting 

    if (level == 1)
        glColor3f(0.4, 0.4, 0.4);	// Dim the ground texture a bit
    else
        glColor3f(0.2f, 0.2f, 0.2f);

    glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

    glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

    glPushMatrix();
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);	// Set quad normal direction.
    glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
    glVertex3f(-15, 0, -15);
    glTexCoord2f(5, 0);
    glVertex3f(15, 0, -15);
    glTexCoord2f(5, 5);
    glVertex3f(15, 0, 15);
    glTexCoord2f(0, 5);
    glVertex3f(-15, 0, 15);
    glEnd();
    glPopMatrix();

    glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

    glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void draw()
{
    glEnable(GL_TEXTURE_2D);
    GLuint texture;
    glGenTextures(1, &texture);

    unsigned char texture_data[2][2][4] =
    {
        0,0,0,255,  255,255,255,255,
        255,255,255,255,    0,0,0,255
    };

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_NEAREST);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 0.0);  glVertex3f(-50.0, -5.0, -50.0);
    glTexCoord2f(25.0, 0.0);  glVertex3f(50.0, -5.0, -50.0);
    glTexCoord2f(25.0, 25.0);  glVertex3f(50.0, -5.0, 50.0);
    glTexCoord2f(0.0, 25.0);  glVertex3f(-50.0, -5.0, 50.0);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void DrawPlayer() {
    if (firstPerson) {
        glPushMatrix();
        glTranslated(camX, 0, camZ);
        glTranslated(0, 2.5, 0);
        glRotatef(yaw, 0.0, 1.0, 0.0);
        glTranslated(0, 0, 0.9);
        glRotated(-90, 0, 1, 0);
        glRotated(90, 1, 0, 0);
        glScaled(0.1, 0.09, 0.1);
        model_player.Draw();
        glPopMatrix();
    }
    else {
        playerx = 1 + camX;
        playerz = 1.6 + camZ;
        glPushMatrix();
        glTranslated(playerx, 0, playerz);
        glTranslated(0, 2.5, 0);
        //glTranslated(1, 0, 1.6);
        glRotatef(yaw, 0.0, 1.0, 0.0);
        glTranslated(-1, 0, -1.6);
        glRotated(-90, 0, 1, 0);
        glRotated(90, 1, 0, 0);
        glScaled(0.1, 0.09, 0.1);
        model_player.Draw();
        glPopMatrix();
    }
}



void drawZombie(int x, int z) {
    if (level == 1) {
        glPushMatrix();
        glTranslated(x, 2.5, z);
        glRotated(zombieAngle, 0, 1, 0);
        glRotated(90, 1, 0, 0);
        glScaled(0.1, 0.09, 0.1);
        model_zombie.Draw();
        glPopMatrix();
    }
    else {
        glPushMatrix();
        glTranslated(x, 2.5, z);
        glRotated(zombieAngle, 0, 1, 0);
        glRotated(90, 1, 0, 0);
        glScaled(0.1, 0.09, 0.1);
        model_zombie2.Draw();
        glPopMatrix();
    }
}

void clockCollision() {
    for (int i = 0; i < 2; i++) {
        if (clockX[i] != -100) {
            if ((int)camX == clockX[i] && (int)camZ == clockZ[i] || (int)camX == (clockX[i] - 0.5) && (int)camZ == clockZ[i]) {
                clockX[i] = -100;
                if (level == 1) {
                    gameTimer += 5;
                    Mix_PlayChannel(-1, clockSound, 0);
                }
                else {
                    playerHealth += 10;
                    Mix_PlayChannel(-1, yummers, 0);
                }
            }
        }
    }
}

void drawCrate(double x, double z) {
    if (level == 1) {
        glPushMatrix();
        glTranslated(x, 1.2, z);
        glScaled(0.05, 0.05, 0.05);
        model_crate.Draw();

        glPopMatrix();
    }
    else {
        glPushMatrix();
        glTranslated(x, 1.2, z);
        glScaled(0.05, 0.05, 0.05);
        model_crate2.Draw();

        glPopMatrix();
    }

}

void drawHeart(double x, double z) {
    glPushMatrix();
    glTranslated(x, 3, z);
    glRotated(clockAngle, 0, 1, 0);
    glScaled(0.025, 0.025, 0.025);
    glRotated(90, 1, 0, 0);
    model_heart.Draw();
    glPopMatrix();
}

void drawClock(int x, int z) {
    if (level == 1) {
        glPushMatrix();
        glTranslated(x, 2.5, z);
        glRotated(clockAngle, 0, 1, 0);
        glScaled(0.03, 0.03, 0.03);
        glRotated(37, 1, 0, 0);
        glRotated(40, 0, 0, 1);
        glRotated(-30, 0, 1, 0);
        model_clock.Draw();
        glPopMatrix();
    }
    else {
        drawHeart(x, z);
    }
}

void drawKey(double x, double z) {
    if (level == 1) {
        glPushMatrix();
        glTranslated(x, 1.5, z);
        glRotated(clockAngle, 0, 1, 0);
        glScaled(0.025, 0.025, 0.025);
        glRotated(90, 1, 0, 0);
        model_key.Draw();
        glPopMatrix();
    }
    else {
        glPushMatrix();
        glTranslated(x, 1.5, z);
        glRotated(clockAngle, 0, 1, 0);
        glScaled(0.025, 0.025, 0.025);
        glRotated(90, 1, 0, 0);
        model_key2.Draw();
        glPopMatrix();
    }
}

void print(double x, double y, double z, char* string)
{
    int len, i;

    //set the position of the text in the window using the x and y coordinates
    glRasterPos3f(x, y, z);
    glColor3f(0.0f, 0.0f, 0.0f);

    //get the length of the string to display
    len = (int)strlen(string);

    //loop to display character by character
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
}

void renderBitmapString(const char* str) {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render main text
    glColor3f(1.0f, 1.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 0.0f); // Set text color to white
    float x = glutGet(GLUT_WINDOW_WIDTH) - 130.0f;  // X-coordinate
    float y = glutGet(GLUT_WINDOW_HEIGHT) - 30.0f;
    glRasterPos2f(x, y);

    while (*str) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *str);
        ++str;
    }

    // Render health information
    char healthStr[20];
    sprintf(healthStr, "Health: %d", playerHealth);
    int index = 0;  // Index variable
    x = glutGet(GLUT_WINDOW_WIDTH) - 130.0f;  // Reset X-coordinate
    y = glutGet(GLUT_WINDOW_HEIGHT) - 60.0f; 
    glRasterPos2f(x, y);

    while (healthStr[index]) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, healthStr[index]);
        ++index;
    }

    // Render health bar
    x = glutGet(GLUT_WINDOW_WIDTH) - 130.0f;;  // Reset X-coordinate
    y = glutGet(GLUT_WINDOW_HEIGHT) - 90.0f;
    glColor3f(0.0, 0.0, 0.0);  // Red color
    glBegin(GL_QUADS);
    glVertex3f(x, y, -10.0f);  // Top-left corner
    glVertex3f(x + playerHealth * 0.1f, y, -10.0f);  // Top-right corner
    glVertex3f(x + playerHealth * 0.1f, y - 2.0f, -10.0f);  // Bottom-right corner
    glVertex3f(x, y - 2.0f, -10.0f);  // Bottom-left corner
    glEnd();

    // Render score information
    glColor3f(1.0f, 1.0f, 0.0f);
    char scoreStr[20];
    sprintf(scoreStr, "Score: %d", score);
    index = 0;  // Reset index
    x = glutGet(GLUT_WINDOW_WIDTH) - 130.0f;;  // Reset X-coordinate
    y = glutGet(GLUT_WINDOW_HEIGHT) - 90.0f;
    glRasterPos2f(x, y);

    while (scoreStr[index]) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, scoreStr[index]);
        ++index;
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_LIGHTING);
}


void checkKeyCollision() {
    if (keyCollected)
        return;
    if (firstPerson) {
        if (abs(camX - keyX) <= 1 && abs(camZ - keyZ) <= 1) {
            keyCollected = 1;
            Mix_PlayChannel(-1, lesgo, 0);
        }
    }
    else {
        if (abs(playerx - keyX) <= 1 && abs(playerz - keyZ) <= 1) {
            keyCollected = 1;
            Mix_PlayChannel(-1, lesgo, 0);
        }

    }
}

void crateCollision() {
    if (firstPerson) {
        if (camX <= 10.95 && camX >= 9.05 && camZ <= 1 && camZ >= -1) {

            //yaw = yaw + 180;

        }
        else if (camX <= -9.05 && camX >= -10.95 && camZ <= 1 && camZ >= -1) {
          //  yaw = yaw + 180;
        }

    }
    else {
        if (playerx <= 10.95 && playerx >= 9.05 && playerz <= 1 && playerz >= -1) {

            yaw = yaw + 180;

        }
        else if (playerx <= -9.05 && playerx >= -10.95 && playerz <= 1 && playerz >= -1) {
            yaw = yaw + 180;
        }


    }
}






void drawWarehouse() {
    if (level == 1) {
        glPushMatrix();
        glTranslated(0, 5, 0);
        glScaled(1.0, 1.0, 1.32);
        glRotated(90, 1, 0, 0);
        model_warehouse1.Draw();
        glPopMatrix();
    }
    else {
        glPushMatrix();
        glTranslated(0, 4, 3.1);
        glScaled(1.08, 0.9, 0.75);
        glRotated(90, 1, 0, 0);
        model_warehouse2.Draw();
        glPopMatrix();
    }
}

void drawDoor() {
    if (level == 1) {
        glDisable(GL_LIGHTING);
        glPushMatrix();
        glTranslated(14.5, 2.6, -4.5);
        glScaled(0.1, 0.1, 0.15);
        glRotated(90, 1, 0, 0);
        model_door.Draw();
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }
    else {
        glDisable(GL_LIGHTING);
        glPushMatrix();
        glTranslated(10.5, 2.48, -11.5);
        glScaled(0.12, 0.09, 0.15);
        glRotated(90, 0, 1, 0);
        glRotated(90, 1, 0, 0);
        model_door.Draw();
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }
}

void gameEnd() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glClearColor(1.0, 1.0, 1.0, 1.0);
 

    // Render main text
    glColor3f(1.0f, 1.0f, 1.0f);
    glColor3f(1.0f, 0.0f, 0.0f); // Set text color to white
    float x = glutGet(GLUT_WINDOW_WIDTH) - 130.0f;  // X-coordinate
    float y = glutGet(GLUT_WINDOW_HEIGHT) - 30.0f;
    glRasterPos2f(x, y);


    int index = 0;  

    glColor3f(1.0f, 1.0f, 0.0f);
    char word[20];
    if (won)
        sprintf(word, "You %s", "won");
    else
        sprintf(word, "You %s", "lost");
    index = 0;  // Reset index
    x = glutGet(GLUT_WINDOW_WIDTH) / 2 - 20.0f;  // Reset X-coordinate
    y = glutGet(GLUT_WINDOW_HEIGHT) / 2;
    glRasterPos2f(x, y);

    while (word[index]) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, word[index]);
        ++index;
    }

    glColor3f(0.0f, 1.0f, 1.0f);
    char scoreStr[20];
    sprintf(scoreStr, "Score: %d", score);
    index = 0;  // Reset index
    x = glutGet(GLUT_WINDOW_WIDTH) / 2 - 20.0f;  // Reset X-coordinate
    y = glutGet(GLUT_WINDOW_HEIGHT) / 2 - 30.0f;
    glRasterPos2f(x, y);

    while (scoreStr[index]) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, scoreStr[index]);
        ++index;
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void display()
{


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    setupLights();
    // glDisable(GL_LIGHTING);
    GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
    GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);


    //printf("camX: %f camZ: %f yaw: %f zombieX :%f zombieZ : %f\n", camX, camZ, yaw, zombieX, zombieZ);
    if(!gameOver)
        camera();
    // Draw Ground
    if (gameTimer < 0 || won || playerHealth <= 0 || gameOver) {
        gameEnd();
        gameOver = true;
        glutSwapBuffers();
        return;
    }
    
    if (!gameOver) {
        RenderGround();


        drawWarehouse();

        drawCrate(-10, 0);
        drawCrate(10, 0);

        glPushMatrix();
        DrawLightBulb();
        glPopMatrix();

        double dist = sqrt(1 + (1.6 * 1.6));
        double actX = camX - dist * sin(DEG2RAD(yaw));
        double actZ = camZ - dist * cos(DEG2RAD(yaw));
        //printf("actX: %f actZ: %f camX: %f camZ: %f\n\n", actX, actZ, camX, camZ);


        if (level == 1) {
            keyX = -12;
            keyZ = 0;
        }
        else {
            keyX = 12;
            keyZ = 0;
        }
        checkKeyCollision();
        if (!keyCollected)
            drawKey(keyX, keyZ);
        else {
            drawDoor();
        }

        // if (abs(camX - zombieX) < 1 && abs(camZ - zombieZ) < 1) {
          //   printf("zombie in me\n");
         //}

        if (firstPerson) {
            clockCollision();
        }
        else {
            for (int i = 0; i < 2; i++) {
                if (clockX[i] != -100) {
                    if ((int)camX >= clockX[i] && (int)camX <= clockX[i] + 1 && (int)camZ >= clockZ[i] && (int)camZ <= clockZ[i] + 1.6 ||
                        (int)camX >= clockX[i] && (int)camX <= clockX[i] + 1 && (int)camZ >= clockZ[i] - 1.6 && (int)camZ <= clockZ[i] ||
                        (int)camX >= clockX[i] - 1 && (int)camX <= clockX[i] && (int)camZ >= clockZ[i] && (int)camZ <= clockZ[i] + 1.6 ||
                        (int)camX >= clockX[i] - 1 && (int)camX <= clockX[i] && (int)camZ >= clockZ[i] - 1.6 && (int)camZ <= clockZ[i] ||
                        (int)camX >= clockX[i] && (int)camX <= clockX[i] + 1.6 && (int)camZ >= clockZ[i] && (int)camZ <= clockZ[i] + 1 ||
                        (int)camX >= clockX[i] && (int)camX <= clockX[i] + 1.6 && (int)camZ >= clockZ[i] - 1 && (int)camZ <= clockZ[i] ||
                        (int)camX >= clockX[i] - 1.6 && (int)camX <= clockX[i] && (int)camZ >= clockZ[i] && (int)camZ <= clockZ[i] + 1 ||
                        (int)camX >= clockX[i] - 1.6 && (int)camX <= clockX[i] && (int)camZ >= clockZ[i] - 1 && (int)camZ <= clockZ[i]
                        ) {
                        clockX[i] = -100;
                        if (level == 1) {
                            gameTimer += 5;
                            Mix_PlayChannel(-1, clockSound, 0);
                        }
                        else {
                            playerHealth += 10;
                            Mix_PlayChannel(-1, yummers, 0);
                        }
                    }
                }
            }

        }


        drawZombie(zombieX, zombieZ);
        for (int i = 0; i < 2; i++) {
            if (clockX[i] != -100)
                drawClock(clockX[i], clockZ[i]);
        }
        DrawPlayer();

        /*
        glPushMatrix();
        glTranslatef(10, 0, 0);
        glScalef(0.7, 0.7, 0.7);
        model_tree.Draw();
        glPopMatrix();*/

        // Draw house Model

        if (keyCollected) {
            if (firstPerson) {
                if (level == 1 && camX >= 12 && camZ >= -6.3 && camZ <= -3.6) {
                    level = 2;
                    tex_ground.Load("Textures/ground3.bmp");
                    Mix_PlayChannel(-1, levelUp, 0);
                    keyCollected = false;
                    clockX[0] = 0;
                    clockX[1] = 0;
                    clockZ[0] = 0;
                    clockZ[1] = -7;
                    camX = 0;
                    camZ = 10;
                    yaw = 0;
                    pitch = 0;
                }
                //10.490259 camZ: -9.500000
                else if (level == 2 && camX >= 10.0 && camX <= 12 && camZ <= -9.5) {
                    won = 1;
                    if (!gameOver)
                        Mix_PlayChannel(-1, gameWin, 0);
                }
            }
            else if (!firstPerson) {
                if (level == 1 && camX >= 12 && camZ >= -6 && camZ <= -3) {
                    level = 2;
                    tex_ground.Load("Textures/ground3.bmp");
                    Mix_PlayChannel(-1, levelUp, 0);
                    keyCollected = false;
                    clockX[0] = 0;
                    clockX[1] = 0;
                    clockZ[0] = 0;
                    clockZ[1] = -7;
                    camX = 0;
                    camZ = 10;
                    yaw = 0;
                    pitch = 0;
                }
                else if (level == 2 && camX >= 9.4 && camX <= 12 && camZ <= -9.5) {
                    won = 1;
                    if (!gameOver)
                        Mix_PlayChannel(-1, gameWin, 0);
                }
            }
        }
        std::string scoreStr = "Time: " + std::to_string(gameTimer);
        renderBitmapString(scoreStr.c_str());
    }

    

    // Render the score text
    glDisable(GL_LIGHTING);
    

    glEnable(GL_LIGHTING);
    //printf("camX: %f camZ: %f yaw: %f\n", camX, camZ, yaw);
    //printf("playerX: %f playerZ: %f yaw: %f\n", playerz, camZ, yaw);
    
    glutSwapBuffers();

}

void LoadAssets()
{
    // Loading Model files
    model_house.Load("Models/house/house.3DS");
    model_tree.Load("Models/tree/Tree1.3ds");
    model_warehouse1.Load("Models/warehouse/warehouse.3ds");
    model_warehouse2.Load("Models/warehouse2/untitled.3ds");
    model_zombie.Load("Models/zombie/untitled.3ds");
    model_zombie2.Load("Models/zombie2/untitled.3ds");
    model_clock.Load("Models/clock/untitled.3ds");
    model_player.Load("Models/robo/untitled.3ds");
    model_crate.Load("Models/crate/untitled.3ds");
    model_crate2.Load("Models/crate2/untitled.3ds");
    model_key2.Load("Models/key2/untitled.3ds");
    model_heart.Load("Models/heart/untitled.3ds");
    model_key.Load("Models/key/untitled.3ds");
    model_door.Load("Models/door/untitled.3ds");
    model_door2.Load("Models/door2/untitled.3ds");

    // Loading texture files
    tex_ground.Load("Textures/ground2.bmp");
    loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 16.0 / 9.0, 1, 75);
    glMatrixMode(GL_MODELVIEW);

}


/*this funtion is used to keep calling the display function periodically
  at a rate of FPS times in one second. The constant FPS is defined above and
  has the value of 60
*/
void timer(int)
{
    glutWarpPointer(width / 2, height / 2);
    clockAngle += 3;
    countFPS++;
    if (upLamp) {
        lampAngle += 0.7;
        lightbulbx += 0.0325;
        lighthintensitybulb += 0.01;
    }
    else {
        lampAngle -= 0.7;
        lightbulbx -= 0.0325;
        lighthintensitybulb -= 0.01;
    }
    if (lampAngle > 30) {
       // printf("lightbulbx: %f", lightbulbx);
        lampAngle = 30;
        upLamp = !upLamp;
    }
    if (lampAngle < -30) {
        lampAngle = -30;
        upLamp = !upLamp;
    }

    if (countFPS % 60 == 0) {
        gameTimer--;
        if (score > 0 && !gameOver)
            score = score - 20;
        //printf("gameTimer :%d count fps: %d\n", gameTimer, countFPS);
    }
    if (countFPS % 30 == 0 && abs(zombieX - camX) < 1 && abs(zombieZ - camZ) < 1) {
       // printf("zombie in here\n");
        if (playerHealth > 0 && !gameOver) {
            playerHealth = playerHealth - 10;
            Mix_PlayChannel(-1, damage, 0);
        }
    }
    //if (countFPS % (60 * 3) == 0) {
      //  glEnable(GL_LIGHT1);
    //}
   // if (countFPS % (60 * 5) == 0) {
     //   glDisable(GL_LIGHT1);
    //}
    if (flashlight) {
        glEnable(GL_LIGHT1);
    }
    else
        glDisable(GL_LIGHT1);


    if (camX < zombieX) {
        zombieAngle = atan((camZ - zombieZ) / (abs(zombieX - camX)));
        zombieAngle = zombieAngle * (180 / M_PI);
        if (zombieX > camX && abs(zombieX - camX) > 1)
            zombieX -= 0.047;
        else if (abs(zombieX - camX) > 1)
            zombieX += 0.047;
        if (zombieZ > camZ && abs(zombieZ - camZ) > 1)
            zombieZ -= 0.047;
        else if (abs(zombieZ - camZ) > 1)
            zombieZ += 0.047;
    }
    else {
        zombieAngle = atan(abs(camZ - zombieZ) / ((camX - zombieX)));
        zombieAngle = zombieAngle * (180 / M_PI);
        zombieAngle += 180;
        if (zombieX > camX && abs(zombieX - camX) > 1)
            zombieX -= 0.047;
        else if (abs(zombieX - camX) > 1)
            zombieX += 0.047;
        if (zombieZ > camZ && abs(zombieZ - camZ) > 1)
            zombieZ -= 0.047;
        else if (abs(zombieZ - camZ) > 1)
            zombieZ += 0.047;
    }
    // if (zombieX < camX)
     //   zombieAngle *= -1;
    glutPostRedisplay();
    glutTimerFunc(14, timer, 0);
}

void passive_motion(int x, int y)
{
    /* two variables to store X and Y coordinates, as observed from the center
      of the window
    */
    int dev_x, dev_y;
    dev_x = (width / 2) - x;
    dev_y = (height / 2) - y;

    double dist = sqrt(1 + (1.6 * 1.6));
    double actX = camX - dist * sin(DEG2RAD(yaw));
    double actZ = camZ - dist * cos(DEG2RAD(yaw));
    printf("actX: %f actZ: %f camX: %f camZ: %f\n", actX, actZ, camX, camZ);
    

    /* apply the changes to pitch and yaw*/
    if (gameTimer >= 0) {
        double currYaw = yaw;
        yaw += (float)dev_x / 10.0;
        double dist = sqrt(1 + (1.6 * 1.6));
        double actX = camX - dist * sin(DEG2RAD(yaw));
        double actZ = camZ - dist * cos(DEG2RAD(yaw));
        // (actX >= -10.95 && actX <= -6.7)
        if (!firstPerson && ((actX >= 6.7 && actX <= 10.95) || false) && actZ <= 2.5 && actZ >= -2.5) {
            yaw = currYaw;
        }
        pitch += (float)dev_y / 10.0;
    }
    if (yaw > 360)
        yaw = 0;
    if (yaw < 0)
        yaw = 360;
}


void checkCrateSides(double oldcamX, double oldCamZ) {
    if (firstPerson) {
        if (camZ <= 1.3 && camZ >= -1.3) {
            printf("camX: %f oldCamX :%f \n", camX, oldcamX);
            if (camX <= 10.95 && camX >= 9.05) {
                if (camX > oldcamX)
                    camX = 9.04;
                else
                    camX = 10.96;
            }
            else if (camX >= -10.95 && camX <= -9.05) {
                if (camX > oldcamX)
                    camX = -10.96;
                else {
                    camX = -9.04;
                    printf("\n----------------COL COL COL COL-----------------\n ");
                }
            }
        }
    }
    else {
        double dist = sqrt(1 + (1.6 * 1.6));
        double actX = camX - dist * sin(DEG2RAD(yaw));
        double actZ = camZ - dist * cos(DEG2RAD(yaw));
         //  printf("actX: %f oldCamX :%f \n", actX, oldcamX);
            if (actX <= 10.95 && actX >= 7.05 && actZ <= 1 && actZ >= -3.0) {
                camX = oldcamX;
                camZ = oldCamZ;
            }
            else if (actX >= -10.95 && actX <= -9.05 && actZ <= 0 && actZ >= -4.0) {
                camX = oldcamX;
                camZ = oldCamZ;
            }
    }
}

void checkCrateTopSides(double oldcamZ, double oldcamX) {
    if (firstPerson) {
        if (camX <= 10.95 && camX >= 9.05 || camX >= -10.95 && camX <= -9.05) {
            if (camZ <= 1.3 && camZ >= -1.3) {
                if (camZ > oldcamZ)
                    camZ = -1.4;
                else
                    camZ = 1.4;
            }
        }
    }
    else {
        double dist = sqrt(1 + (1.6 * 1.6));
        double actX = camX - dist * sin(DEG2RAD(yaw));
        double actZ = camZ - dist * cos(DEG2RAD(yaw));
        if (actX <= 10.00 && actX >= 6.05 || actX >= -12.00 && actX <= -9.05) {
            if (actZ <= 0.1 && actZ >= -3) {
                camX = oldcamX;
                camZ = oldcamZ;
            }
        }
    }
}

void camera()
{
    double incCamX = 0;
    double incCamZ = 0;
    if (gameTimer >= 0) {
        if (motion.Forward)
        {
            double oldcamX = camX;
            double oldcamZ = camZ;
          
            camX += cos((yaw + 90) * TO_RADIANS) / 7.5;
            checkCrateSides(oldcamX, oldcamZ);
            camZ -= sin((yaw + 90) * TO_RADIANS) / 7.5;
            checkCrateTopSides(oldcamZ, oldcamX);
        }
        if (motion.Backward)
        {
            double oldcamX = camX;
            double oldcamZ = camZ;
            
            camX += cos((yaw + 90 + 180) * TO_RADIANS) / 7.5;
            checkCrateSides(oldcamX, oldcamZ);
            camZ -= sin((yaw + 90 + 180) * TO_RADIANS) / 7.5;
            checkCrateTopSides(oldcamZ, oldcamX);
        }
        if (motion.Left)
        {
            double oldcamX = camX;
            double oldcamZ = camZ;
            
            camX += cos((yaw + 90 + 90) * TO_RADIANS) / 7.5;
            checkCrateSides(oldcamX, oldcamZ);
            camZ -= sin((yaw + 90 + 90) * TO_RADIANS) / 7.5;
            checkCrateTopSides(oldcamZ, oldcamX);
        }
        if (motion.Right)
        {
            double oldcamX = camX;
            double oldcamZ = camZ;
            
            camX += cos((yaw + 90 - 90) * TO_RADIANS) / 7.5;
            checkCrateSides(oldcamX, oldcamZ);
            camZ -= sin((yaw + 90 - 90) * TO_RADIANS) / 7.5;
            checkCrateTopSides(oldcamZ, oldcamX);
        }
    }

    /*
    if (camZ >= 1 && camZ <= -1) {
        if (camX + incCamX <= 10.95 && camX + incCamX >= 9.05) {
            if (camX + incCamX > camX)
                camX = 9.04;
            else
                camX = 10.94;
        }
        else if (camX + incCamX >= -10.95 && camX + incCamX <= -9.05) {
            if (camX + incCamX > camX)
                camX = -10.94;
            else
                camX = -9.04;
        }
        camZ -= incCamZ;
    }
    else {
        camZ -= incCamZ;
        camX += incCamX;
    }*/

   // else {
     //   if (camX >= 10.)
    //}


    if (camX > 12)
        camX = 12;
    if (camX < -12)
        camX = -12;
    if (camZ > 11)
        camZ = 11;
    if (camZ < -9.5)
        camZ = -9.5;

    /*limit the values of pitch
      between -60 and 70
    */
    if (pitch >= 70)
        pitch = 75;
    if (pitch <= -90)
        pitch = -90;

    glRotatef(-pitch, 1.0, 0.0, 0.0); // Along X axis
    glRotatef(-yaw, 0.0, 1.0, 0.0);    //Along Y axis

    glTranslatef(-camX, -4.5, -camZ);
    if (!firstPerson)
        glTranslated(-1, 0, -1.6);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'W':
    case 'w':
        motion.Forward = true;
        break;
    case 'A':
    case 'a':
        motion.Left = true;
        break;
    case 'S':
    case 's':
        motion.Backward = true;
        break;
    case 'D':
    case 'd':
        motion.Right = true;
        break;
    case 'f':
    case 'F':
        flashlight = !flashlight;
        break;
    case 27:
        exit(EXIT_SUCCESS);
    }
}
void keyboard_up(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'W':
    case 'w':
        motion.Forward = false;
        break;
    case 'A':
    case 'a':
        motion.Left = false;
        break;
    case 'S':
    case 's':
        motion.Backward = false;
        break;
    case 'D':
    case 'd':
        motion.Right = false;
        break;
    case 'R':
    case 'r':
        firstPerson = !firstPerson;
        break;
    }
}














