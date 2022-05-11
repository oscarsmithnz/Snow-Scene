/******************************************************************************
 *
 * Animation v1.0 (23/02/2021)
 *
 * This template provides a basic FPS-limited render loop for an animated scene.
 *
 ******************************************************************************/

#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <stdio.h>

 /******************************************************************************
  * Animation & Timing Setup
  ******************************************************************************/

  // Target frame rate (number of Frames Per Second).
#define TARGET_FPS		60

//values for degree/radian conversion for drawing circles
#define PI				3.14159265
#define DEG_TO_RAD PI	/180.0

#define MAX_PARTICLES	1000 //max amount of snow particles
#define GROUND_VERTICES 25 //number of vertices that form the ground's irregular polygon.

// Ideal time each frame should be displayed for (in milliseconds).
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;

// Frame time in fractional seconds.
// Note: This is calculated to accurately reflect the truncated integer value of
// FRAME_TIME, which is used for timing, rather than the more accurate fractional
// value we'd get if we simply calculated "FRAME_TIME_SEC = 1.0f / TARGET_FPS".
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;

// Time we started preparing the current frame (in milliseconds since GLUT was initialized).
unsigned int frameStartTime = 0;

/******************************************************************************
 * Keyboard Input Handling Setup
 ******************************************************************************/

 // Define all character keys used for input (add any new key definitions here).
 // Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
 // characters typed by the user to lowercase, so the SHIFT key is ignored.

#define KEY_EXIT			27  // Escape key.
#define KEY_EXIT2			'q' //secondary exit key
#define KEY_ANIMATE			's' //start and stop animation, 115
#define KEY_DIAGNOSTICS		'd' //toggle diagnostic display
#define KEY_GENERATEGROUND	'g' //regenerate the ground vertices
#define KEY_FASTERSNOW		'2' //make the snow fall faster
#define KEY_SLOWERSNOW		'1' //make the snow fall slower
#define KEY_SNOWDRIFT		'w' //stop snow from drifting towards mouse

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void idle(void);

/******************************************************************************
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/


void main(int argc, char **argv);
void init(void);
void think(void);
void drawSnow(int amount);
void drawSnowman();
void drawGround();
void drawBackground();
void diagnosticsDisplay();
void updateParticle(int i);
void displayString(char* string, float startX, float startY);
void generateGround();
void updateMousePosition(int x, int y);
void drawCircle(float x, float y, float xDivisor, float yDivisor);

/******************************************************************************
 * Animation-Specific Setup (Add your own definitions, constants, and globals here)
 ******************************************************************************/

//vector struct with xy coordinates
typedef struct {
	float x;
	float y;
} Vector;

//struct representing one individual snow particle
typedef struct {
	Vector location;
	float size;
	float transparency;
	float speed;
	int isAlive;
} SnowParticle;

//array holding all snow particles, and counter for current number of snow
SnowParticle snow[MAX_PARTICLES];
int currentSnow = 0;

//toggles for user
int animate = 1;	//snow
int displayOn = 1;	//diagnostics
int snowDrift = 1;	//snow mouse drift

//randomly generated ground vertices, generated when program is run and stored in these arrays
float xcoords[GROUND_VERTICES];
float ycoords[GROUND_VERTICES];

//base snow speed
float baseSpeed = 0.001f;
//for mouse drift usage, needs to be global as used in two methods
float mouseX;
int windowWidth = 1000;
int windowHeight = 800;

 /******************************************************************************
  * Entry Point (don't put anything except the main function here)
  ******************************************************************************/

void main(int argc, char **argv)
{
	// Initialize the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Animation");
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	srand((unsigned)time(NULL));

	// Set up the scene.
	init();

	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutIdleFunc(idle);
	glutPassiveMotionFunc(updateMousePosition);

	// Record when we started rendering the very first frame (which should happen after we call glutMainLoop).
	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

	// Enter the main drawing loop (this will never return).
	glutMainLoop();
}

/******************************************************************************
 * GLUT Callbacks (don't add any other functions here)
 ******************************************************************************/

 /*
	 Called when GLUT wants us to (re)draw the current animation frame.

	 Note: This function must not do anything to update the state of our simulated
	 world. Animation (moving or rotating things, responding to keyboard input,
	 etc.) should only be performed within the think() function provided below.
 */
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	drawBackground();



	if (animate && (currentSnow < MAX_PARTICLES)) {
		snow[currentSnow].isAlive = 1;
		currentSnow += 1;
	}
	drawSnow(0);
	drawGround();
	drawSnow(MAX_PARTICLES/3);
	drawSnowman();
	drawSnow((MAX_PARTICLES/3) * 2);

	if (displayOn)
		diagnosticsDisplay();

	glutSwapBuffers();
}

/*
	Called when the OpenGL window has been resized.
*/
void reshape(int width, int h)
{
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is pressed.
*/
void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

	case KEY_ANIMATE: //toggle snow fall
		animate = !animate;
		for (int i = 0; i < currentSnow; i++) {
			snow[i].isAlive = 0;
		}
		break;
	case KEY_DIAGNOSTICS: //toggle diagnostic display
		displayOn = !displayOn;
		break;
	case KEY_GENERATEGROUND: //regenerate the random ground vertices
		generateGround();
		break;
	case KEY_FASTERSNOW: //make snow fall roughly 10% faster
		baseSpeed *= 1.1f;
		break;
	case KEY_SLOWERSNOW: //make snow fall roughly 10% slower
		baseSpeed *= 0.9f;
		break;
	case KEY_SNOWDRIFT: //toggle snow drifting towards cursor
		snowDrift = !snowDrift;
		break;
	case KEY_EXIT: //escape to exit
		exit(0);
		break;
	case KEY_EXIT2: //s to exit
		exit(0);
		break;
	}
}

/*
	Called by GLUT when it's not rendering a frame.

	Note: We use this to handle animation and timing. You shouldn't need to modify
	this callback at all. Instead, place your animation logic (e.g. moving or rotating
	things) within the think() method provided with this template.
*/
void idle(void)
{
	// Wait until it's time to render the next frame.

	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) - frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		// This frame took less time to render than the ideal FRAME_TIME: we'll suspend this thread for the remaining time,
		// so we're not taking up the CPU until we need to render another frame.
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}

	// Begin processing the next frame.

	frameStartTime = glutGet(GLUT_ELAPSED_TIME); // Record when we started work on the new frame.

	think(); // Update our simulated world before the next call to display().

	glutPostRedisplay(); // Tell OpenGL there's a new frame ready to be drawn.
}

/******************************************************************************
 * Animation-Specific Functions (Add your own functions at the end of this section)
 ******************************************************************************/

 /*
	 Initialise OpenGL and set up our scene before we begin the render loop.
 */
void init(void)
{
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

	//initialise snow array with appropriate values, all start not alive
	for (int i = 0; i < MAX_PARTICLES; i++) {
		snow[i].location.x = (((float)rand() / RAND_MAX) * 3.0f) - 1.0f;
		snow[i].location.y = 1.1f;
		snow[i].size = (((float)rand() / RAND_MAX) * 2.0f) + 1.5f;
		snow[i].speed = (snow[i].size * 0.5) * baseSpeed;
		snow[i].transparency = (((float)rand() / RAND_MAX) * 0.5) + 0.1;
		snow[i].isAlive = 0;
	}

	//generate random ground that is used
	generateGround();
}

/*
	Advance our animation by FRAME_TIME milliseconds.

	Note: Our template's GLUT idle() callback calls this once before each new
	frame is drawn, EXCEPT the very first frame drawn after our application
	starts. Any setup required before the first frame is drawn should be placed
	in init().
*/
void think(void)
{
	for (int i = 0; i < currentSnow; i++) {
		updateParticle(i);
	}
}

/*
	All draw methods below are for drawing its specific item into the frame
*/
void drawSnow(int amount) {
	for (int i = amount; i < currentSnow; i++) {
		//update based on specific snow attributes
		glPointSize(snow[i].size);
		glColor4f(1.0f, 1.0f, 1.0f, snow[i].transparency);
		
		glBegin(GL_POINTS);
			glVertex2f(snow[i].location.x, snow[i].location.y);
		glEnd();
	}
}

void drawSnowman() {
	//lower body
	glBegin(GL_TRIANGLE_FAN);
		//color center 
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glVertex2f(-0.45f, -0.5f);

		//color outer 
		glColor4f(0.475f, 0.565f, 0.655f, 1.0f);
		drawCircle(-0.45f, -0.5f, 5.0f, 4.0f);
	glEnd();

	//upper body
	glBegin(GL_TRIANGLE_FAN);
		//color center
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glVertex2f(-0.45f, -0.15f);

		//color outer
		glColor4f(0.475f, 0.565f, 0.655f, 1.0f);
		drawCircle(-0.45f, -0.15f, (7.5*0.8), (6*0.8));

	glEnd();

	//head
	glBegin(GL_TRIANGLE_FAN);
		//color center
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glVertex2f(-0.45f, 0.15f);

		//color outer
		glColor4f(0.475f, 0.565f, 0.655f, 1.0f);
		drawCircle(-0.45f, 0.15f, 7.5f, 6.0f);
	glEnd();

	//left eye
	glBegin(GL_TRIANGLE_FAN);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex2f(-0.5f, 0.2f);

		glColor3f(0.2f, 0.2f, 0.2f);
		drawCircle(-0.5f, 0.2f, 60, 48);
	glEnd();

	//right eye
	glBegin(GL_TRIANGLE_FAN);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex2f(-0.395f, 0.2f);

		glColor3f(0.2f, 0.2f, 0.2f);
		drawCircle(-0.395f, 0.2f, 60, 48);
	glEnd();

	//nose
	glBegin(GL_POLYGON);
		glColor3f(0.9f, 0.5f, 0.1f);
		glVertex2f(-0.465f, 0.15f);
		glVertex2f(-0.435f, 0.15f);
		glVertex2f(-0.4475f, 0.05f);
	glEnd();

	
	//arms
	glLineWidth(25.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
		glVertex2f(-0.3f, -0.12f);
		glVertex2f(-0.15f, -0.05f);

		glVertex2f(-0.6f, -0.12f);
		glVertex2f(-0.75f, -0.05f);
	glEnd();

	//fingers
	glLineWidth(5.0f);
	glBegin(GL_LINES);
		//right hand
		glVertex2f(-0.16f, -0.049f);
		glVertex2f(-0.09f, 0.035f);

		glVertex2f(-0.16f, -0.049f);
		glVertex2f(-0.07f, -0.01f);

		glVertex2f(-0.16f, -0.049f);
		glVertex2f(-0.085f, -0.08f);

		//left hand
		glVertex2f(-0.74f, -0.049f);
		glVertex2f(-0.81f, 0.035f);

		glVertex2f(-0.74f, -0.049f);
		glVertex2f(-0.83f, -0.01f);

		glVertex2f(-0.74f, -0.049f);
		glVertex2f(-0.815f, -0.08f);
	glEnd();
}

void drawCircle(float x, float y, float xDivisor, float yDivisor) {
	for (int theta = 0; theta <= 360; theta += 2) {
		glVertex2f((cos(theta*DEG_TO_RAD) / xDivisor) + x, (sin(theta*DEG_TO_RAD) / yDivisor) + y);
	}
}

void drawGround() {
	float bottomY = -1.0f;

	glBegin(GL_QUAD_STRIP);

	for (int i = 0; i < GROUND_VERTICES; i++) {
		//bottom vertex
		glColor3f(0.1f, 0.0f, 0.0f);
		glVertex2f(xcoords[i], bottomY);

		//top vertex
		glColor3f(0.1f, 0.9f, 1.0f);
		glVertex2f(xcoords[i], ycoords[i]);
	}

	glEnd();
}

//old method which resulted in random number of vertices + random locations for those vertices, couldn't figure out
//an easy way to not have it change every frame instead of staying the same.
//void drawGround() {
//	float x = -1.0f;
//	float bottomY = -1.0f;
//	float topY = -0.6f;
//
//	glBegin(GL_QUAD_STRIP);
//		glColor3f(0.1f, 0.0f, 0.0f);
//		glVertex2f(x, bottomY);
//		glColor3f(0.1f, 0.9f, 1.0f);
//		glVertex2f(x, topY);
//
//		while (x < 1.0f) {
//			x += (((float)rand() / RAND_MAX));
//			topY += (((float)rand() / RAND_MAX)-0.5f) / 5;
//			glColor3f(0.1f, 0.0f, 0.0f);
//			glVertex2f(x, bottomY);
//			glColor3f(0.1f, 0.9f, 1.0f);
//			glVertex2f(x, topY);
//		}
//	glEnd();
//}

void drawBackground() {
	glBegin(GL_POLYGON);
		glColor4f(0.0f, 0.0f, 0.3f, 0.95f);
		glVertex2f(1.0, 1.0);
		glVertex2f(-1.0, 1.0);

		glColor4f(0.0f, 0.8f, 1.0f, 0.75f);
		glVertex2f(-1.0, -1.0);
		glVertex2f(1.0, -1.0);
	glEnd();
}

void generateGround() {
	xcoords[0] = -1.0f;
	ycoords[0] = -0.4f;
	for (int i = 1; i < GROUND_VERTICES; i++) {
		xcoords[i] = xcoords[i - 1] + (2.0f / (GROUND_VERTICES-1));
		ycoords[i] = ycoords[i - 1] + ((((float)rand() / RAND_MAX) - 0.5f) / 10);
	}
}

void diagnosticsDisplay() {
	glColor3f(0.0f, 1.0f, 0.0f);

	float startX = -0.99f;
	float startY = 0.95f;
	displayString("Diagnostics", startX, startY);

	startY = 0.9f;
	char string[1024];
	snprintf(string, sizeof(string), " current particles: %i of %i", currentSnow, MAX_PARTICLES);
	displayString(string, startX, startY);

	startY = 0.85f;
	snprintf(string, sizeof(string), " current base snow speed: %.1f", (baseSpeed * 1000));
	displayString(string, startX, startY);

	startY = 0.8f;
	displayString("Scene controls:", startX, startY);

	startY = 0.75f;
	displayString(" q: quit", startX, startY);

	startY = 0.7f;
	displayString(" s: toggle snow", startX, startY);

	startY = 0.65f;
	displayString(" d: toggle diagnostics display", startX, startY);

	startY = 0.6f;
	displayString(" g: regerate ground", startX, startY);

	startY = 0.55f;
	displayString(" w: toggle snow cursor drift", startX, startY);

	startY = 0.5f;
	displayString(" 1: reduce snow fall speed", startX, startY);

	startY = 0.45f;
	displayString(" 2: increase snow fall speed", startX, startY);
}

//helper method for displaying strings as glut bitmap characters
void displayString(char* string, float startX, float startY) {
	int size = strlen(string);
	for (int i = 0; i < size; i++) {
		glRasterPos2f(startX + (i * 0.0125f), startY);
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, string[i]);
	}
}

//update a single particles position
void updateParticle(int i) {
	//first update speed for if it has been changed from user input
	snow[i].speed = (snow[i].size * 0.5) * baseSpeed;

	//change position based on speed
	snow[i].location.y -= snow[i].speed;

	//move towards mouse on x axis

	if (snowDrift) {
		float x_difference;
		if (snow[i].location.x > mouseX) {
			x_difference = snow[i].location.x - mouseX;
			snow[i].location.x -= snow[i].speed * 0.25 * x_difference;
		}
		else if (snow[i].location.x < mouseX) {
			x_difference = mouseX - snow[i].location.x;
			snow[i].location.x += snow[i].speed * 0.25 * x_difference;
		}
	}

	//if s has been pressed then do not recycle
	if ((snow[i].location.y < -1.0f) && animate) {
		//recycle
		snow[i].location.x = (((float)rand() / RAND_MAX) * 3.0f) - 1.0f;
		snow[i].location.y = 1.1f;
		snow[i].size = (((float)rand() / RAND_MAX) * 2.0f) + 1.5f;
		snow[i].transparency = (((float)rand() / RAND_MAX) * 0.5) + 0.1;
	}
}

//update position of the mouse every time it moves, for snow drift
void updateMousePosition(int x, int y) {
	//changes from int resolution value to value from 0-1
	mouseX = (float)x / (float)windowWidth;
	//have to change from 0-1 to -1 to 1
	mouseX = (mouseX * 2) - 1;
}
/******************************************************************************/