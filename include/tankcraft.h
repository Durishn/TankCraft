/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "graphics.h"

#ifndef H_INCLUDED
#define H_INCLUDED

	/************ EXTERN VARIABLE DECLARATIONS *****************/
	/* mouse function called by GLUT when a button is pressed or released */
	void mouse(int, int, int, int);

	/* initialize graphics library */
	extern void graphicsInit(int *, char **);

	/* lighting control */
	extern void setLightPosition(GLfloat, GLfloat, GLfloat);
	extern GLfloat* getLightPosition();

	/* viewpoint control */
	extern void setViewPosition(float, float, float);
	extern void getViewPosition(float *, float *, float *);
	extern void getOldViewPosition(float *, float *, float *);
	extern void setViewOrientation(float, float, float);
	extern void getViewOrientation(float *, float *, float *);

	/* add cube to display list so it will be drawn */
	extern int addDisplayList(int, int, int);

	/* mob controls */
	extern void createMob(int, float, float, float, float);
	extern void setMobPosition(int, float, float, float, float);
	extern void hideMob(int);
	extern void showMob(int);

	/* player controls */
	extern void createPlayer(int, float, float, float, float);
	extern void setPlayerPosition(int, float, float, float, float);
	extern void hidePlayer(int);
	extern void showPlayer(int);

	/* 2D drawing functions */
	extern void  draw2Dline(int, int, int, int, int);
	extern void  draw2Dbox(int, int, int, int);
	extern void  draw2Dtriangle(int, int, int, int, int, int);
	extern void  set2Dcolour(float []);

	/*View functions*/
	extern float lengthTwoPoints(float x1, float y1, float z1, float x2, float y2, float z2);
	extern float dot (float x1, float y1, float z1, float x2, float y2, float z2);

	/* flag which is set to 1 when flying behaviour is desired */
	extern int flycontrol;
	/* flag used to indicate that the test world should be used */
	extern int testWorld;
	/* flag to print out frames per second */
	extern int fps;
	/* flag to indicate the space bar has been pressed */
	extern int space;
	/* flag to indicate the 'z' has been pressed */
	extern int dig;
	/* flag indicates the program is a client when set = 1 */
	extern int netClient;
	/* flag indicates the program is a server when set = 1 */
	extern int netServer;
	/* size of the window in pixels */
	extern int screenWidth, screenHeight;
	/* flag indicates if map is to be printed */
	extern int displayMap;
	/* flag to change velocity and angle */
	extern int velUp;
	extern int velDown;
	extern int angUp;
	extern int angDown;

	/* frustum corner coordinates, used for visibility determination */
	extern float corners[4][3];

	/* determine which cubes are visible e.g. in view frustum */
	extern void ExtractFrustum();
	extern void tree(float, float, float, float, float, float, int);

	/***************** Function Prototypes ******************/
	/* networking functions*/
	extern void serverMode();
	extern void clientMode();

	/* world generation functions */
	extern float Noise(int, int);
	extern float CosineInterpolate(float, float, float);
	extern float SmoothedNoise(int, int);
	extern float InterpolateNoise(float, float);
	extern float PerlinNoise_2D(float, float, float, int);
	extern void generateWorldFeatures();

	/* world update functions */
	extern void updateWater();
	extern void updateClouds();
	extern void removeCube();

	/* minimap & dash functions */
	extern void playerDeath(int);
	extern void drawDash();
	extern void drawMiniMapLarge( int[WORLDZ][WORLDX][2] );
	extern void drawMiniMapSmall( int[WORLDZ][WORLDX][2] );

	/* missile and explosion functions */
	extern void explosionSphere(int, int, int);
	extern void explosionPyramid(int, int, int);
	extern int fireMissile(int, float*, float*, float*, float*, float*, float*
		, float*, float*);

	/* bot & AI functions*/
	extern void botController(int);
	extern void botDeath(int);
	extern void botSearch(int);
	extern void botFight(int);
	extern void botShoot(int);
	extern void botMove(int);
	extern void botRules(int);
	extern int botSight(int);
	extern int botSightRange(int);

	/* others */
	extern void collisionResponse();
	extern void draw2D();
	extern void update();

	/***************** PARAMETER CONTROL PANEL ******************/
	/* World generation parameters*/
	#define LAND_PERS 0.56 /*land persistance: perlin noise*/
	#define LAND_OCT 7/*land octaves: perlin noise*/
	#define WATER_DEPTH 2/*depth of water from bottom*/
	#define WATER_FLOW_SPEED 5/*water speed: lower is faster*/
	#define BEDROCK_DEPTH 1/*depth of bedrock from bottom*/

	/* Cloud generation parameters*/
	#define CLOUD1_LEVEL WORLDY-5 /*cloud1 height*/
	#define CLOUD1_SPEED 20/*cloud1 speed: lower is faster*/
	#define CLOUD1_PERS 0.85/*cloud1 persistance: perlin noise*/
	#define CLOUD1_OCT 10/*cloud1 persistance: perlin noise*/
	#define CLOUD2_LEVEL WORLDY-1/*cloud2 height*/
	#define CLOUD2_SPEED 80/*cloud2 speed: lower is faster*/
	#define CLOUD2_PERS 0.985/*cloud2 persistance: perlin noise*/
	#define CLOUD2_OCT 6/*cloud2 persistance: perlin noise*/

	/* Bot, Popup, Missile, Explosion & Radio parameters*/
	#define BOT_COUNT 5 //MAX 10
	#define BOT_SIGHT_RANGE 40
	#define BOT_ANG_LOCK 45
	#define POP_COUNT 1 /*defines popup timing*/
	#define RAPIDFIRE_MODE 0 /*RPM: 0 is off, 1 is on*/
	#define MISSILE_COUNT 5 /*missiles: less than 10 (5 on multiplayer)*/
	#define EXPLOSION_RADIUS 3 /*radius of explosion from center*/
	#define EXPLOSION_SHAPE 1 /*0 is pyramid, 1 is sphere*/
	#define VELOCITY_CONSTRAINT 1.1/*velocity multiplier*/
	#define RADIO_ON 0 /*0 to turn off the terminal radio completely*/
	#define RADIO_BLIP 1 /*0 to turn off radio blips*/
	#define RADIO_SCAN_SPEED 60 /*speed of blips: lower is faster*/

	/* Player Controls, Colours & Minimap parameters */
	#define JUMP_LAG 1 
	#define MM_SIZE 2
	#define MM_TRANS 0.85
	static GLfloat p1ColourSolid[4];
	static GLfloat p1ColourTrans[4];

	/*************** CONSTANTS, FLAGS & COUNTERS*******************/
	#define PI 3.1415926
	static int UPDATE_COUNTER;

	/* missile & popup parameters */
	static int mNum[MISSILE_COUNT+BOT_COUNT+1][4];
	static clock_t missile_clock;
	static int MISSILE_FLAG = 0;
	static int BOT_MIS_FLAG[BOT_COUNT];
	static char popUpString[15];
	static int popUpFlag = 0;

	/* player parameters */
	static struct Player {
		double vel;
		int ang;
		int deaths;
		int kills;
		int jump_flag;
	} player;

	/* bot parameters */
	#define BSEARCH 0
	#define BFIGHT 1
	#define BSHOOT 2
	#define BMOVE 3
	static struct Bots {
		float x,y,z,r;
		int state, deaths, mAvoid;
		float wx,wz;
		clock_t wanderCount;
		clock_t sightCount;
		clock_t shotCount;
	} bot[BOT_COUNT];

	/* network socket parameters*/
	static const int bType = 0;
	static const int pType = 1;
	static const int mType = 2;
	static int client_sockfd, sockfd;
	static int wChange[4];
	static float p1Change[3];
	static int mChange[2];

#endif