/***********************************tankcraft.c*********************************
   Nic Durish (#0757227)(ndurish@uoguelph.ca)                  March 18th, 2015
   CIS 4820 - Game Programming                                 Assignment 4
The following Game Package contains the files; a1.c, visible.c, graphics.c,
graphics.h and a makefile. Together these files can be compiled to build a game
world. The following game world consists of a procedurally generated map, basic
collision control, gravity and cloud movement.

* LICENSE *
***********
The MIT License (MIT)

Copyright (c) 2015 Nic Durish

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


* REFERENCES *
**************
Algorithms from http://freespace.virgin.net/hugo.elias/models/m_perlin.htm have
been implemented in the following functions; Noise, CosineInterpolate,
InterpolateNoise, SmoothedNoise and PerlinNoise_2D.
********************************************************************************/
#include "tankcraft.h"

/* updateWater(): Updates water to flow to adjecent empty cubes*/
void updateWater(){

   int i,j,k;

   /* if proper count, update water to flow to adjacent squares */
   if (UPDATE_COUNTER % WATER_FLOW_SPEED == 0)
      /*Use buffer cube (purple: 6)*/
      for (i=0; i<WORLDX; i++)
         for (j=BEDROCK_DEPTH; j<=WATER_DEPTH; j++)
            for(k=0; k<WORLDZ; k++)
               if (world[i][j][k] == 0)
                  if ((world[i+1][j][k] == 2) || (world[i-1][j][k] == 2) ||
                     (world[i][j+1][k] == 2) || (world[i][j][k+1] == 2) ||
                     (world[i][j][k-1] == 2))
                     world[i][j][k] = 6;

   /*Change all buffers to water*/
   for (i=0; i<WORLDX; i++){
      for (j=BEDROCK_DEPTH; j<=WATER_DEPTH; j++){
         for(k=0; k<WORLDZ; k++){
            if (world[i][j][k] == 6){
               world[i][j][k] = 2;
               if (netServer){
                  write(client_sockfd, &bType, sizeof(bType));
                  wChange[0]=i; wChange[1]=j;
                  wChange[2]=k; wChange[3]=2;
                  write(client_sockfd, &wChange, sizeof(wChange));
               }
            }
         }
      }
   }
}

/*updateClouds(): update cloud movements */
void updateClouds(){

   int i, j;

   /*Move cloud layer 1*/
   if (UPDATE_COUNTER % CLOUD1_SPEED == 0){

      /*Save first cloud row*/
      int cloudRow[WORLDX];
      for(i=0; i<WORLDX; i++){
         cloudRow[i] = world[i][CLOUD1_LEVEL][WORLDZ-1];
         world[i][CLOUD1_LEVEL][WORLDZ-1] = 0;
         if (netServer){
            write(client_sockfd, &bType, sizeof(bType));
            wChange[0]=i; wChange[1]=CLOUD1_LEVEL;
            wChange[2]=WORLDZ-1; wChange[3]=0;
            write(client_sockfd, &wChange, sizeof(wChange));
         }
      }

      /*Shift clouds*/
      for(i=WORLDX-1; i>=0; i--) {
         for(j=WORLDZ-2; j>=0; j--){
            if (world[i][CLOUD1_LEVEL][j] != 0){
               world[i][CLOUD1_LEVEL][j] = 0;
               if (netServer){
                  write(client_sockfd, &bType, sizeof(bType));
                  wChange[0]=i; wChange[1]=CLOUD1_LEVEL;
                  wChange[2]=j; wChange[3]=0;
                  write(client_sockfd, &wChange, sizeof(wChange));
               }
               world[i][CLOUD1_LEVEL][j+1] = 5;
               if (netServer){
                  write(client_sockfd, &bType, sizeof(bType));
                  wChange[0]=i; wChange[1]=CLOUD1_LEVEL;
                  wChange[2]=j+1; wChange[3]=5;
                  write(client_sockfd, &wChange, sizeof(wChange));
               }
            }
         }
      }
      for(i=0; i<WORLDX; i++){
         world[i][CLOUD1_LEVEL][0] = cloudRow[i];
         if (netServer){
            write(client_sockfd, &bType, sizeof(bType));
            wChange[0]=i; wChange[1]=CLOUD1_LEVEL;
            wChange[2]=0; wChange[3]=cloudRow[i];
            write(client_sockfd, &wChange, sizeof(wChange));
         }
      }
   }


   /*Move cloud layer 2*/
   if (UPDATE_COUNTER % CLOUD2_SPEED == 0){

      /*Save first cloud row*/
      int cloudRow[WORLDX];
      for(i=0; i<WORLDX; i++){
         cloudRow[i] = world[i][CLOUD2_LEVEL][WORLDZ-1];
         world[i][CLOUD2_LEVEL][WORLDZ-1] = 0;
      }
      /*Shift clouds*/
      for(i=WORLDX-1; i>=0; i--) {
         for(j=WORLDZ-2; j>=0; j--){
            if (world[i][CLOUD2_LEVEL][j] != 0){
               world[i][CLOUD2_LEVEL][j] = 0;
               world[i][CLOUD2_LEVEL][j+1] = 5;
            }
         }
      }
      for(i=0; i<WORLDX; i++)
         world[i][CLOUD2_LEVEL][0] = cloudRow[i];
   }
}

/*removeCube(): removes cube in front of players viewport, called if dig=TRUE */
void removeCube(){

   float xPos, yPos, zPos, xOri, yOri, zOri;

   /* find proper cube based on proper position and orientation */
   getViewPosition(&xPos, &yPos, &zPos);
   getViewOrientation(&xOri, &yOri, &zOri);
   int x,y,z;
   x=y=z=-1;

   /* facing down */
   if (((int)xOri%360 > 45) && ((int)xOri%360 <= 135)) {
      if (world[(int)(xPos*-1)][(int)(yPos*-1)-1][(int)(zPos*-1)] != 4){
         x=(int)(xPos*-1); y=(int)(yPos*-1)-1; z=(int)(zPos*-1);
         world[x][y][z] = 0;
      }
   }

   /* facing backwards */
   else if (((int)xOri%360 > 135) && ((int)xOri%360 <= 225)){
      if (((int)yOri%360 > 45) && ((int)yOri%360 <= 135)) {
         if (world[(int)(xPos*-1) -1][(int)(yPos*-1)][(int)(zPos*-1)] != 4){
            x=(int)(xPos*-1)-1; y=(int)(yPos*-1); z=(int)(zPos*-1);
            world[x][y][z] = 0;
         }
      }
      else if (((int)yOri%360 > 135) && ((int)yOri%360 <= 225)) {
         if (world[(int)(xPos*-1)][(int)(yPos*-1)][(int)(zPos*-1) -1] != 4){
            x=(int)(xPos*-1); y=(int)(yPos*-1); z=(int)(zPos*-1)-1;
            world[x][y][z] = 0;
         }
      }
      else if (((int)yOri%360 > 225) && ((int)yOri%360 <= 315)) {
         if (world[(int)(xPos*-1)+1][(int)(yPos*-1)][(int)(zPos*-1) ] != 4){
            x=(int)(xPos*-1)+1; y=(int)(yPos*-1); z=(int)(zPos*-1);
            world[x][y][z] = 0;
         }
      }
      else {
         if (world[(int)(xPos*-1)][(int)(yPos*-1)][(int)(zPos*-1) +1] != 4){
            x=(int)(xPos*-1); y=(int)(yPos*-1); z=(int)(zPos*-1)+1;
            world[x][y][z] = 0;
         }
      }
   }

   /* facing up */
   else if (((int)xOri%360 > 225) && ((int)xOri%360 <= 315)) {
      if (world[(int)(xPos*-1)][(int)(yPos*-1)+1][(int)(zPos*-1)] != 4){
         x=(int)(xPos*-1); y=(int)(yPos*-1)+1; z=(int)(zPos*-1);
         world[x][y][z] = 0;
      }
   }

   /* facing forwards */
   else {
      if (((int)yOri%360 > 45) && ((int)yOri%360 <= 135)) {
         if (world[(int)(xPos*-1) +1][(int)(yPos*-1)][(int)(zPos*-1)] != 4){
            x=(int)(xPos*-1)+1; y=(int)(yPos*-1); z=(int)(zPos*-1);
            world[x][y][z] = 0;
         }
      }
      else if (((int)yOri%360 > 135) && ((int)yOri%360 <= 225)) {
         if (world[(int)(xPos*-1)][(int)(yPos*-1)][(int)(zPos*-1) +1] != 4){
            x=(int)(xPos*-1); y=(int)(yPos*-1); z=(int)(zPos*-1) +1;
            world[x][y][z] = 0;
         }
      }
      else if (((int)yOri%360 > 225) && ((int)yOri%360 <= 315)) {
         if (world[(int)(xPos*-1)-1][(int)(yPos*-1)][(int)(zPos*-1) ] != 4){
            x=(int)(xPos*-1)-1; y=(int)(yPos*-1); z=(int)(zPos*-1);
            world[x][y][z] = 0;
         }
      }
      else {
         if (world[(int)(xPos*-1)][(int)(yPos*-1)][(int)(zPos*-1) -1] != 4){
            x=(int)(xPos*-1); y=(int)(yPos*-1); z=(int)(zPos*-1)-1;
            world[x][y][z] = 0;
         }
      }
   }
   if (netServer){
      write(client_sockfd, &bType, sizeof(bType));
      wChange[0]=x; wChange[1]=y; wChange[2]=z; wChange[3]=0;
      write(client_sockfd, &wChange, sizeof(wChange));
   }
   space = 0;
}

/*playerDeath(): performs required actions after a players death*/
void playerDeath(int death){

   player.deaths++;

   if (RADIO_ON == 1){
      printf("*...\n\n**********SCANNING FOR SIGNAL**********\n-");
      switch (death){
         case 0:
            printf("COMMANDER: Your Tank sunk... \n");
         break;
         case 1:
            printf("COMMANDER: Your Tank exploded... \n");
         break;
         case 2:
            printf("COMMANDER: Your Tank dropped into a void... \n");
         break;
         case 3:
            printf("COMMANDER: Welcome to your training...\n");
         break;
      }
      printf("-COMMANDER: Tank #%d is being deployed..\n", player.deaths);
      printf("********** NEW SIGNAL:  CH %d **********\n*\n", player.deaths);
   }
 
   setViewPosition(-50.0,-50.0,-50.0);
}

/*drawDash(): draws the UI for the Dashboard*/
void drawDash(){

   int mmCoords = MM_SIZE*100;
   int misNum, i, j;
   int hBuff = (screenHeight - (screenHeight/WORLDX)); 
   int wBuff = (screenWidth - (screenWidth/WORLDX)); 
   char string[20];
   memset(string,0,sizeof(string));
   GLfloat textColour[] = {1, 1, 1, 1};
   GLfloat backColour[] = {0, 0, 0, MM_TRANS};
   GLfloat backBarColour[] = {1, 1, 1, 0.8};
   GLfloat blackColour[] = {0, 0, 0, 1};
   GLfloat redColour[] = {1, 0.3, 0.3, 1};
   GLfloat blueColour[] = {0.3, 0.3, 1, 1};
   GLfloat greenColour[] = {0.3, 1, 0.3, 1};

   /* print current bullets */
   misNum=0;
   for (i=0; i<MISSILE_COUNT; i++)
      if (mNum[i][0] == 1)
         misNum++;
   set2Dcolour(textColour);
   glRasterPos2f(wBuff-mmCoords+5, ((hBuff-1)-mmCoords*1.05)-15 );
   sprintf(string, "AMMO:");
   for (i = 0; i < 20; i++) 
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
   memset(string,0,sizeof(string));
   set2Dcolour(redColour);
   for (i=0; i<MISSILE_COUNT; i++){
      if (i>=misNum)
         set2Dcolour(backBarColour);
      draw2Dbox(wBuff-mmCoords+72+(i*25), (hBuff-18) - mmCoords*1.05
         , wBuff-mmCoords+77+(i*25), (hBuff-5)-mmCoords*1.05);
   }
   set2Dcolour(textColour);

   /* print current velocity number*/
   set2Dcolour(blackColour);
   glRasterPos2f(wBuff-mmCoords+113, ((hBuff-1)-mmCoords*1.05)-36 );
   sprintf(string, "%.02f", player.vel);
   for (i = 0; i < 20; i++) 
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
   memset(string,0,sizeof(string));

   /* print current velocity */
   set2Dcolour(textColour);
   glRasterPos2f(wBuff-mmCoords+5, ((hBuff-1)-mmCoords*1.05)-35 );
   sprintf(string, "VELOC:");
   for (i = 0; i < 20; i++) 
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
   memset(string,0,sizeof(string));
   set2Dcolour(blueColour);
   draw2Dbox(wBuff-mmCoords+60, ((hBuff-38) - mmCoords*1.05)
         , wBuff-mmCoords+60+(player.vel*130), ((hBuff-28)-mmCoords*1.05));
   set2Dcolour(backBarColour);
   draw2Dbox(wBuff-mmCoords+60, ((hBuff-38) - mmCoords*1.05)
         , wBuff-10, ((hBuff-28)-mmCoords*1.05));

   /* print current velocity number*/
   set2Dcolour(blackColour);
   glRasterPos2f(wBuff-mmCoords+118, ((hBuff-1)-mmCoords*1.05)-56 );
   sprintf(string, "%d", player.ang);
   for (i = 0; i < 20; i++) 
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
   memset(string,0,sizeof(string));

   /* print current angle */
   set2Dcolour(textColour);
   glRasterPos2f(wBuff-mmCoords+5, ((hBuff-1)-mmCoords*1.05)-55 );
   sprintf(string, "ANGLE:");
   for (i = 0; i < 20; i++) 
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
   memset(string,0,sizeof(string));
   set2Dcolour(greenColour);
   draw2Dbox(wBuff-mmCoords+60, ((hBuff-58) - mmCoords*1.05)
         , wBuff-mmCoords+60+(((float)player.ang/90)*130)
         , ((hBuff-48)-mmCoords*1.05));
   set2Dcolour(backBarColour);
   draw2Dbox(wBuff-mmCoords+60, ((hBuff-58) - mmCoords*1.05)
         , wBuff-10, ((hBuff-48)-mmCoords*1.05));

   /* print current deaths & kills*/
   glRasterPos2f(wBuff-mmCoords+30, ((hBuff-1)-mmCoords*1.05)-75 );
   sprintf(string, "Deaths: %d", player.deaths);
   for (i = 0; i < 20; i++) 
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
   memset(string,0,sizeof(string));
   glRasterPos2f(wBuff-mmCoords+130, ((hBuff-1)-mmCoords*1.05)-75 );
   sprintf(string, "Kills: %d", player.kills);
   for (i = 0; i < 20; i++) 
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
   memset(string,0,sizeof(string));

   /* draw dash box */  
   set2Dcolour(backColour);
   draw2Dbox(wBuff-mmCoords-1, (hBuff-85) - mmCoords*1.05
      , wBuff +1, (hBuff-1)-mmCoords*1.05);

   /* print bot states */
   set2Dcolour(textColour);
   glRasterPos2f(wBuff-mmCoords-1+51, ((hBuff-1)-mmCoords*1.05)-105);
   sprintf(string, "Opponent  States:");
   for (i = 0; i < 20; i++) 
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
   memset(string,0,sizeof(string));
   for (j=0; j<BOT_COUNT; j++){
      glRasterPos2f(wBuff-mmCoords-1+57
         , ((hBuff-1)-mmCoords*1.05)-108-((j+1)*15));
      switch(bot[j].state){
         case BSEARCH:
            sprintf(string, "Bot %d: SEARCH", j+1);
         break;
         case BFIGHT:
            sprintf(string, "Bot %d:   FIGHT", j+1);
         break;
         case BSHOOT:
            sprintf(string, "Bot %d:  SHOOT", j+1);
         break;
         case BMOVE:
            sprintf(string, "Bot %d:  DODGE", j+1);
         break;
      }
      for (i = 0; i < 20; i++) 
         glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
      memset(string,0,sizeof(string));
   }

   /* draw dash box */  
   set2Dcolour(backColour);
   draw2Dbox(wBuff-mmCoords+40, (hBuff-90) - mmCoords*1.05
      , wBuff -40, (hBuff-115-(15*BOT_COUNT))-mmCoords*1.05);
}

/*drawDash(): draws the UI for Large MiniMap*/
void drawMiniMapLarge( int worldtop[WORLDX][WORLDZ][2]){

   /* initialize height and width buffer (to keep in corner & off sides)*/
   int i, j, k;
   float x,y,z;
   int mmCoords = MM_SIZE*100;
   int sizeBuff, centBuff;
   char string[20];

   /* colour definitions */
   GLfloat solidBlack[] = {0, 0, 0, 1};
   
   if (screenWidth > screenHeight)
      sizeBuff = (screenHeight/WORLDX);
   else
      sizeBuff = (screenWidth/WORLDZ);
   centBuff = floor(sizeBuff)*WORLDZ;

   /* print player coordinates */
   getViewPosition(&x,&y,&z);
   set2Dcolour(solidBlack);
   glRasterPos2f(sizeBuff + ((screenWidth-centBuff)/2)
      , sizeBuff + ((screenHeight-centBuff)/2));
   sprintf(string, "%d, %d, %d", (int)((x*-1))
      , (int)((y*-1)), (int)((z*-1)) );
   for (i = 0; i < 12; i++) 
      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
   memset(string,0,sizeof(string));

   /* draw player position */
   getViewPosition(&x,&y,&z);
   set2Dcolour(p1ColourSolid);
   draw2Dbox( ((int)WORLDX-(x*-1)) *sizeBuff 
      + ((screenWidth-centBuff)/2), ((int)(z*-1)) *sizeBuff
      + ((screenHeight-centBuff)/2), (((int)WORLDX-(x*-1))  
      *sizeBuff) +sizeBuff + ((screenWidth-centBuff)/2)
   , (((int)(z*-1))  *sizeBuff) +sizeBuff + ((screenHeight-centBuff)/2));

   /* draw bot position */
   for (i=0; i<BOT_COUNT; i++){
      GLfloat tempColour[] = {1.1-((double)(abs(bot[i].y))/WORLDY), 0
         ,((double)(abs(bot[i].y))/WORLDY), 1};
      set2Dcolour(tempColour);
      draw2Dbox( ((double)WORLDX-(bot[i].x)) *sizeBuff 
         + ((screenWidth-centBuff)/2), ((double)(bot[i].z)) *sizeBuff
         + ((screenHeight-centBuff)/2), (((double)WORLDX-(bot[i].x))  
         *sizeBuff) +sizeBuff + ((screenWidth-centBuff)/2)
         , (((double)(bot[i].z))  *sizeBuff) +sizeBuff + ((screenHeight
         -centBuff)/2));
   }

   /* draw missile position */
   for (i=0; i<MISSILE_COUNT+BOT_COUNT+1; i++){
      if (mNum[i][0]){
         GLfloat tempColour[] = {1, ((double)(abs(mNum[i][2]))
                  /WORLDY), 0, 1};
         set2Dcolour(tempColour);
         draw2Dbox( ((double)WORLDX-(mNum[i][1]*-1)) *sizeBuff 
            + ((screenWidth-centBuff)/2), ((double)(mNum[i][3]*-1)) *sizeBuff
            + ((screenHeight-centBuff)/2), (((double)WORLDX-(mNum[i][1]*-1))  
            *sizeBuff) +sizeBuff + ((screenWidth-centBuff)/2)
            , (((double)(mNum[i][3]*-1))  *sizeBuff) +sizeBuff + ((screenHeight
            -centBuff)/2));

      }
   }

   /* draw map features */
   for (i=0; i<WORLDX; i++){
      for (j=0; j<WORLDZ; j++){

         /*check type of tile and draw colour hue based on height */
         switch(worldtop[i][j][0]){
            case 1:{
               GLfloat tempColour[] = {0, ((double)worldtop[i][j][1]
                  /WORLDY)+0.05, 0, 1};
               set2Dcolour(tempColour);
            break;}
            case 2:{
               GLfloat tempColour[] = {0, 0, 0.4, 1};
               set2Dcolour(tempColour);
            break;}
            case 3:{
               GLfloat tempColour[] = {0.5, 0, 0, 1};
               set2Dcolour(tempColour);
            break;}
            default:
               set2Dcolour(solidBlack);
            break;
         }
         draw2Dbox( (WORLDX-i-1) *sizeBuff + ((screenWidth-centBuff)/2)
            , j *sizeBuff + ((screenHeight-centBuff)/2)
            , ((WORLDX-i-1) *sizeBuff) + sizeBuff + ((screenWidth-centBuff)/2)
            , (j *sizeBuff) +sizeBuff + ((screenHeight-centBuff)/2) );             
      }
   }

   /* draw map border */
   if (screenWidth>screenHeight){ 
      set2Dcolour(solidBlack);
      draw2Dbox((screenHeight - screenHeight/100) + ((screenWidth - 
         screenHeight)/2), (screenHeight - screenHeight/100), 
         (screenHeight/100) + ((screenWidth - screenHeight)/2), 
         (screenHeight/100));
   }
   else{
   set2Dcolour(solidBlack);
   draw2Dbox((screenWidth - screenWidth/100), (screenWidth 
      - screenWidth/100) + ((screenHeight - screenWidth)/2), 
      (screenWidth/100), (screenWidth/100) + ((screenHeight - 
      screenWidth)/2));
   }

   //drawDash();
}

/*drawDash(): draws the UI for the Small MiniMap*/
void drawMiniMapSmall( int worldtop[WORLDX][WORLDZ][2]){

   /* initialize height and width buffer (to keep in corner & off sides)*/
   int i, j, k;
   float x,y,z;
   int mmCoords = MM_SIZE*100;
   int hBuff = (screenHeight - (screenHeight/WORLDX)); 
   int wBuff = (screenWidth - (screenWidth/WORLDX)); 
   char string[20];

   /* colour definitions */
   GLfloat solidBlack[] = {0, 0, 0, 1};

   /* draw dashboard if not client*/
   if (!netClient)
      drawDash();

   /* print player coordinates */
   getViewPosition(&x,&y,&z);
   set2Dcolour(solidBlack);
   glRasterPos2f(wBuff-mmCoords+1, hBuff-mmCoords);
   sprintf(string, "%d, %d, %d", (int)((x*-1))
      , (int)((y*-1)), (int)((z*-1)) );
   for (i = 0; i < 12; i++) 
      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, string[i]);
   memset(string,0,sizeof(string));

   /* draw player position */  
   set2Dcolour(p1ColourTrans);
   draw2Dbox( ((((int)WORLDX-(x*-1) +1) *MM_SIZE) + wBuff) -mmCoords
      , ((((int)(z*-1)) *MM_SIZE) + hBuff) -mmCoords
      , ((((int)WORLDX-(x*-1) +1) *MM_SIZE) + wBuff) -mmCoords-MM_SIZE
      , ((((int)(z*-1)) *MM_SIZE) + hBuff) -mmCoords-MM_SIZE);

   /* draw bot position */
   for (i=0; i<BOT_COUNT; i++){
      GLfloat tempColour[] = {1.1-((double)(abs(bot[i].y))/WORLDY), 0
         ,((double)(abs(bot[i].y))/WORLDY), 1};
         set2Dcolour(tempColour);
      draw2Dbox( ((((int)WORLDX-(bot[i].x) +1) *MM_SIZE) + wBuff) -mmCoords
      , ((((int)(bot[i].z)) *MM_SIZE) + hBuff) -mmCoords
      , ((((int)WORLDX-(bot[i].x) +1) *MM_SIZE) + wBuff) -mmCoords-MM_SIZE
      , ((((int)(bot[i].z)) *MM_SIZE) + hBuff) -mmCoords-MM_SIZE);
   }

   /* draw missile position */
   for (i=0; i<MISSILE_COUNT+BOT_COUNT+1; i++){
      if (mNum[i][0]){
         GLfloat tempColour[] = {1, ((double)(abs(mNum[i][2]))
                  /WORLDY), 0, 1};
         set2Dcolour(tempColour);
         
         draw2Dbox( ((((double)WORLDX-(mNum[i][1]*-1)) *MM_SIZE) + wBuff)
            -mmCoords, ((((double)(mNum[i][3]*-1)) *MM_SIZE) + hBuff)
            -mmCoords, ((((double)WORLDX-(mNum[i][1]*-1)) *MM_SIZE) 
            + wBuff) -mmCoords-MM_SIZE, ((((double)(mNum[i][3]*-1)) *MM_SIZE)
            + hBuff) -mmCoords-MM_SIZE);
      }
   }

   /* draw map features */
   for (i=0; i<WORLDX; i++){
      for (j=0; j<WORLDZ; j++){

         /*check type of tile and draw colour hue based on height */
         switch(worldtop[i][j][0]){
            case 1:{
               GLfloat tempColour[] = {0, ((double)worldtop[i][j][1]
                  /WORLDY)+0.05, 0, MM_TRANS};
               set2Dcolour(tempColour);
            break;}
            case 2:{
               GLfloat tempColour[] = {0, 0, 0.4, MM_TRANS};
               set2Dcolour(tempColour);
            break;}
            case 3:{
               GLfloat tempColour[] = {0.5, 0, 0, MM_TRANS};
               set2Dcolour(tempColour);
            break;}
            default:{
               GLfloat tempColour[] = {0, 0, 0, MM_TRANS};
               set2Dcolour(tempColour);
            break;}
         }
         draw2Dbox( (((WORLDX-i) *MM_SIZE) + wBuff) -mmCoords
            , ((j *MM_SIZE) + hBuff) -mmCoords
            , (((WORLDX-i) *MM_SIZE) + wBuff) -mmCoords-MM_SIZE
            , ((j *MM_SIZE) + hBuff) -mmCoords-MM_SIZE); 
      }
   }

   /* draw map border */
   set2Dcolour(solidBlack);
   draw2Dline( wBuff, hBuff - MM_SIZE, wBuff - mmCoords
      , hBuff - MM_SIZE, 2);
   draw2Dline( wBuff, hBuff - MM_SIZE - mmCoords
      , wBuff - mmCoords, hBuff - MM_SIZE -mmCoords, 2);
   draw2Dline( wBuff, hBuff - MM_SIZE, wBuff
      , hBuff - MM_SIZE -mmCoords, 2);
   draw2Dline( wBuff -mmCoords, hBuff - MM_SIZE
      , wBuff - mmCoords, hBuff - MM_SIZE -mmCoords, 2);
}

/*Performs a spherical explosion at given coordinates*/
void explosionSphere(int x, int y, int z){

   int i, j, k, a;
   float tempX, tempY, tempZ;

   if (world [x][y][z] != 4){
      world[x][y][z] = 3;
      if (netServer){
         write(client_sockfd, &bType, sizeof(bType));
         wChange[0]=x; wChange[1]=y; wChange[2]=z; wChange[3]=3;
         write(client_sockfd, &wChange, sizeof(wChange));
      }
   }
   else{
      y++;
      world[x][y][z] = 3;
      if (netServer){
         write(client_sockfd, &bType, sizeof(bType));
         wChange[0]=x; wChange[1]=y; wChange[2]=z; wChange[3]=3;
         write(client_sockfd, &wChange, sizeof(wChange));
      }
   }
   /*Check for direct hits*/
   for (i=0; i<BOT_COUNT; i++){
      if (((int)bot[i].x==x) && ((int)bot[i].z==z) && (((int)bot[i].y==y)
       || ((int)bot[i].y==y+1))){
            botDeath(i);
            popUpFlag = 1;
            sprintf(popUpString, "Direct    Hit!");
            player.kills++;
      }
   }
   getViewPosition(&tempX, &tempY, &tempZ);
   if (((int)tempX*-1==x) && ((int)tempZ*-1==z) && (((int)tempY*-1==y)
    || ((int)tempY*-1==y+1))){
         playerDeath(2);
         popUpFlag = 1;
         sprintf(popUpString, "Direct    Hit!");
   }

   for (a=0; a<EXPLOSION_RADIUS; a++){
      for (i=x-EXPLOSION_RADIUS; i<=x+EXPLOSION_RADIUS; i++)
         for (j=y-EXPLOSION_RADIUS; j<=y+EXPLOSION_RADIUS; j++)
            for (k=z-EXPLOSION_RADIUS; k<=z+EXPLOSION_RADIUS; k++)
               if ((i>=0) && (i<WORLDX) && (j>=0) && (j<WORLDY)
                && (k>=0) && (k<WORLDZ))
                  if ((world[i+1][j][k] == 3) || (world[i-1][j][k] == 3) ||
                     (world[i][j+1][k] == 3) || (world[i][j-1][k] == 3)
                     || (world[i][j][k+1] == 3) || (world[i][j][k-1] == 3))
                     if ((world [i][j][k] != 4) && (world [i][j][k] != 5))
                        world[i][j][k] = 6;
      for (i=x-EXPLOSION_RADIUS; i<=x+EXPLOSION_RADIUS; i++){
         for (j=y-EXPLOSION_RADIUS; j<=y+EXPLOSION_RADIUS; j++) {
            for (k=z-EXPLOSION_RADIUS; k<=z+EXPLOSION_RADIUS; k++){ 
               if ((world[i][j][k] == 6) && (world [i][j][k] != 5)){
                  world[i][j][k] = 3;
                  missile_clock=clock();
                  if (netServer){
                     write(client_sockfd, &bType, sizeof(bType));
                     wChange[0]=i; wChange[1]=j; wChange[2]=k; wChange[3]=3;
                     write(client_sockfd, &wChange, sizeof(wChange));
                  }
               }
            }
         }
      }
   }
}

/*Performs a cube explosion at given coordinates*/
void explosionPyramid(int x, int y, int z){

   int i, a, k;
   for (a=0; a<=EXPLOSION_RADIUS; a++){
      for (i=x-EXPLOSION_RADIUS+a; i<=x+EXPLOSION_RADIUS-a; i++){
         for (k=z-EXPLOSION_RADIUS+a; k<=z+EXPLOSION_RADIUS-a; k++){
            if ((i>=0) && (i<WORLDX) && (y-a>=0) && (y+a<WORLDY)
                && (k>=0) && (k<WORLDZ)){
               if ((world [i][y-a][k] != 4) && (world [i][y+a][k] != 4)
                  && (world [i][y-a][k] != 5) && (world [i][y+a][k] != 5)) {
                  world[i][y-a][k] = 0;
                  world[i][y+a][k] = 0;
                  world[i][y-a][k] = 3;
                  world[i][y+a][k] = 3;
               }
            }
         }
      }
   }
}

/*fireMissile(): begin act of firing and calculating missile trajectory*/
int fireMissile( int missileNum, float* mobx, float* moby, float* mobz
   , float* mobry, float* mobrx, float* mobVel, float* mobAng, float* mobinc){

   /*Check missile's y location: if out of map toggle pseudo (negative) */
   if ((*moby*-1)<-50)
      *moby=(*moby)*-1;

   /*Missile Path in rapidfire mode: fire towards viewpoint*/
   if (RAPIDFIRE_MODE == 1){
      *moby += tan((((int)*mobrx+180)%360)*(PI/180));
      *mobz -= cos((((int)*mobry+180)%360)*(PI/180));
      *mobx += sin((((int)*mobry+180)%360)*(PI/180));
   }

   /*Missile Path: fire in projectile motion*/
   else {
      *moby -= *mobinc;
      *mobz -= (cos((((int)*mobry+180)%360)*(PI/180))
         * ((*mobVel*VELOCITY_CONSTRAINT)+0.1))* cos(*mobAng);
      *mobx += (sin((((int)*mobry+180)%360)*(PI/180))
       * ((*mobVel*VELOCITY_CONSTRAINT)+0.1))* cos(*mobAng);
      *mobinc -= 0.0169;
   }

   /*Check missile's y location: if out of map put in pseudo (negative) */
   if ((*moby*-1)>50)
      *moby=(*moby)*-1;
   else if ((((*moby*-1)>-50)) && (((*moby*-1)<0)))
      *moby=(*moby)*-1;

   /* If missile is not above the map*/
   if (*moby*-1 > 0){

      /*If a collision occurs (that is not a cloud)*/
      if ((world[(int)(*mobx*-1)][(int)(*moby*-1)][(int)(*mobz*-1)] != 0) &&
         (world[(int)(*mobx*-1)][(int)(*moby*-1)][(int)(*mobz*-1)] != 5)){
         hideMob(missileNum);
         if ((EXPLOSION_SHAPE == 0) && (!netClient))
            explosionPyramid((int)(*mobx*-1), (int)(*moby*-1), (int)(*mobz*-1));
         else if (!netClient)
            explosionSphere((int)(*mobx*-1), (int)(*moby*-1), (int)(*mobz*-1));
         return 0;
      }
   }
   setMobPosition(missileNum,(*mobx*-1),(*moby*-1),(*mobz*-1)
      ,((int)-*mobry-180));

   /* Set global variables to trace mobs */
   mNum[missileNum][1] = *mobx;
   mNum[missileNum][2] = *moby;
   mNum[missileNum][3] = *mobz;

   /*If missile goes out of bounds*/
   if (((int)(*mobx*-1) >= WORLDX-1) || ((int)(*mobx*-1) < 0)
      || ((int)(*moby*-1) < -500) || ((int)(*mobz*-1) >= WORLDZ-1)
      || ((int)(*mobz*-1) < 0)){
      hideMob(missileNum);
      return 0;
   }
   return 1;
}

void popUp(){
   int i;
   GLfloat solidBlack[] = {0, 0, 0, 1};

   /* print bot deaths */
   glRasterPos2f(screenWidth/2-48, screenHeight/2-5);
   set2Dcolour(solidBlack);
   for (i = 0; i < 16; i++) 
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, popUpString[i]);

   /* draw dash box */
   GLfloat backColour[4] = {1,1,1,0.4}; 

   if (displayMap == 2)
      backColour[3] = 1;
   set2Dcolour(backColour);
   draw2Dbox((screenWidth/2)-55, (screenHeight/2)-10
      , (screenWidth/2)+55, (screenHeight/2)+15);
}

void botController(int botNum){

   /* initialize variables */
   float x,y,z,r, tempX, tempY, tempZ;
   float oldy, oldx, oldz;
   int i;
   double waterAvoid = 0;
   x = bot[botNum].x; y = bot[botNum].y; z = bot[botNum].z; r= bot[botNum].r;
   oldy = y; oldx = x; oldz = z;

   /****************** BOT BEHAVIOUR ******************/

   /* if falling from sky, avoid water */
   i = 0;
   if ((world[(int)(x)][(int)((y)-1)][(int)(z)] == 0 )
      && (world[(int)(x)][(int)((y)-2)][(int)(z)] == 0 )){
      if (world[(int)(x)][WATER_DEPTH][(int)(z)] == 2){
         while (waterAvoid == 0){
            if (world[(int)(x)+i][WATER_DEPTH][(int)(z)] != 2)
               waterAvoid=1;
            else if (world[(int)(x)-i][WATER_DEPTH][(int)(z)] != 2)
               waterAvoid=-1;
            else
               i++;
         }
      }
      x=x+(waterAvoid/2);
      setPlayerPosition(botNum, x, y, z, r);
      bot[botNum].x = x+(waterAvoid/2); bot[botNum].y = y; bot[botNum].z = z;
      bot[botNum].wanderCount = 999999;
   }

   /* initialize state behaviour */
   else
      switch (bot[botNum].state){
         case BSEARCH:
            botSearch(botNum);
         break;
         case BFIGHT:
            botFight(botNum);
         break;
         case BSHOOT:
            botShoot(botNum);
         break;
         case BMOVE:
            botMove(botNum);
         break;
      }

   /******************** BOT RULES ********************/
   /* kill bot in water*/
   if ((world[(int)(x)][(int)(y)][(int)(z)] == 2)
      || (world[(int)(x)][(int)(y) -1][(int)(z)] == 2)){
      botDeath(botNum);
   }

   /* kill bot in fire*/
   else if ((world[(int)(x)][(int)(y)][(int)(z)] == 3)
      || (world[(int)(x)][(int)(y) -1][(int)(z)] == 3)){
      botDeath(botNum);
      popUpFlag = 1;
      sprintf(popUpString, "Indirect Hit!");
      player.kills++;
   }

   /* kill bots who dropped out bottoms of map*/
   else if ((y)<0)
      botDeath(botNum);

   /*Collision climbing: to allow bot to climb single blocks*/
   else if ((world[(int)(x)][(int)(y)][(int)(z)] != 0)
      && (world[(int)(x)][(int)((y) + 1)][(int)(z)] == 0)){
      setPlayerPosition(botNum, x, y+1, z, r);
      bot[botNum].x = x; bot[botNum].y = y+1; bot[botNum].z = z;
   }

   /*Collision climbing: to allow bot to climb double blocks*/
   else if ((world[(int)(x)][(int)(y)][(int)(z)] != 0)
      && (world[(int)(x)][(int)((y) + 2)][(int)(z)] == 0)){
      setPlayerPosition(botNum, x, y+2, z, r);
      bot[botNum].x = x; bot[botNum].y = y+2; bot[botNum].z = z;
   }

    /*Gravity: pulls bot towards ground if there is space beneath him
   /   -when fly control is set to true*/
   else if (((world[(int)(x)][(int)((y)-0.25)][(int)(z)]==0)
      || (world[(int)(x)][(int)((y))][(int)(z)]== 5))){
         setPlayerPosition(botNum, x, y-0.25, z, r);
         y=y-0.25;
         bot[botNum].x = x; bot[botNum].y = y; bot[botNum].z = z;
   }
}

void botDeath(int botNum){
   bot[botNum].deaths++;

   //Refresh Bot location
   bot[botNum].x = rand()%WORLDX;
   bot[botNum].y = CLOUD1_LEVEL-1;
   bot[botNum].z = rand()%WORLDZ;
   bot[botNum].state = BSEARCH;
   bot[botNum].wx=0; bot[botNum].wz=0; bot[botNum].wanderCount = 0;
   createPlayer(botNum, bot[botNum].x, bot[botNum].y, bot[botNum].z, 0.0);
}

void botSearch(int botNum){

   float x,y,z;

   /* Check for Change State */
   if ((botSightRange(botNum) && (botSight(botNum))))
      bot[botNum].state = BFIGHT;

   /* perform search */
   else {
      if (((clock()-bot[botNum].wanderCount)/CLOCKS_PER_SEC)>=3){
         bot[botNum].wx = (((double)(rand()%10))/40)-0.125;
         bot[botNum].wz = (((double)(rand()%10))/40)-0.125;
         bot[botNum].wanderCount = clock();
      }
      bot[botNum].x = bot[botNum].x + bot[botNum].wx;
      bot[botNum].z = bot[botNum].z + bot[botNum].wz;
      bot[botNum].r = atan2(bot[botNum].wx,bot[botNum].wz)*(180/PI);

      botRules(botNum);
      setPlayerPosition(botNum, bot[botNum].x, bot[botNum].y, bot[botNum].z
         , bot[botNum].r);
   }
}

void botFight(int botNum){

   float x, y, z, angle;
   int i,j,k;

   /* check for sight: begin searching */
   if ((!botSightRange(botNum) || (!botSight(botNum)))){
      
      /* wait 3 seconds before searching */
      if (((clock()-bot[botNum].sightCount)/CLOCKS_PER_SEC)>=3)
         bot[botNum].state = BSEARCH;
   }
   else{

      /* change rotation to follow player*/
      getViewPosition(&x,&y,&z);
      angle = atan2((int)bot[botNum].z - (z*-1)
         , (int)bot[botNum].x - (x*-1))*(180/PI);
      bot[botNum].r = 270-angle;
      setPlayerPosition(botNum, bot[botNum].x, bot[botNum].y, bot[botNum].z
         , bot[botNum].r);

      /* restart sight counter */
      bot[botNum].sightCount = clock(); 

      /* check for dodge */
      for (i=0; i<MISSILE_COUNT; i++){
         if ((mNum[i][0]) && (lengthTwoPoints(mNum[i][1]*-1, mNum[i][2]*-1
            , mNum[i][3]*-1,(int)bot[botNum].x,(int)bot[botNum].y
            ,(int)bot[botNum].z) < 20)){
            bot[botNum].state = BMOVE;
            bot[botNum].mAvoid = i;
         }
      }

      /* if no dodge then check for current bullet*/
      if (bot[botNum].state != BMOVE) {
         if (mNum[MISSILE_COUNT+botNum][0] == 0){
            bot[botNum].shotCount = clock();
            bot[botNum].state = BSHOOT;
         }
      }
   }
}

void botMove (int botNum){

   /* if missile is gone or too far*/
   if ((!mNum[bot[botNum].mAvoid][0])
      || (lengthTwoPoints(mNum[bot[botNum].mAvoid][1]*-1
      , mNum[bot[botNum].mAvoid][2]*-1, mNum[bot[botNum].mAvoid][3]*-1
      ,(int)bot[botNum].x,(int)bot[botNum].y,(int)bot[botNum].z) > 20)){
      bot[botNum].state = BFIGHT;
   }
   else {
      bot[botNum].r = atan2((int)bot[botNum].x-mNum[bot[botNum].mAvoid][1]*-1
         ,(int)bot[botNum].z-mNum[bot[botNum].mAvoid][3]*-1)*(180/PI);
      bot[botNum].wx = sin(bot[botNum].r/(180/PI))/8;
      bot[botNum].wz = cos(bot[botNum].r/(180/PI))/8;
      bot[botNum].x = bot[botNum].x + bot[botNum].wx;
      bot[botNum].z = bot[botNum].z + bot[botNum].wz;
      botRules(botNum);
      setPlayerPosition(botNum, bot[botNum].x, bot[botNum].y, bot[botNum].z
         , bot[botNum].r);
   }
}

void botShoot(int botNum){

   int i,angle;
   float x,y,z;

   /* check for dodge */
   for (i=0; i<MISSILE_COUNT; i++){
      if ((mNum[i][0]) && (lengthTwoPoints(mNum[i][1]*-1, mNum[i][2]*-1
         , mNum[i][3]*-1,(int)bot[botNum].x,(int)bot[botNum].y
         ,(int)bot[botNum].z) < 20)){
         bot[botNum].state = BFIGHT;
      }
   }

   /* change rotation to follow player*/
   if ((botSightRange(botNum) || (botSight(botNum)))){
      getViewPosition(&x,&y,&z);
      angle = atan2((int)bot[botNum].z - (z*-1)
         , (int)bot[botNum].x - (x*-1))*57.296;
      bot[botNum].r = 270-angle;
      setPlayerPosition(botNum, bot[botNum].x, bot[botNum].y, bot[botNum].z
         , bot[botNum].r);
      bot[botNum].sightCount = clock();
   } 

   /* wait 0.5 seconds and shoot bullet */
   if (((clock()-bot[botNum].shotCount)/CLOCKS_PER_SEC)>=0.5){
      BOT_MIS_FLAG[botNum] = 1;
      bot[botNum].shotCount = clock();
      bot[botNum].state = BFIGHT;
   }
}

void botRules (int botNum){

   float x,y,z;

   /* check for wall (over two blocks high)*/
   if ((world[(int)(bot[botNum].x)][(int)(bot[botNum].y)]
      [(int)(bot[botNum].z)] != 0) && (world[(int)(bot[botNum].x)]
      [(int)(bot[botNum].y) +2][(int)(bot[botNum].z)] != 0)) {
      bot[botNum].wx = 0-bot[botNum].wx;
      bot[botNum].wz = 0-bot[botNum].wz;
      bot[botNum].x = bot[botNum].x + bot[botNum].wx;
      bot[botNum].z = bot[botNum].z + bot[botNum].wz;
   }

   /* check for water*/
   if ((world[(int)(bot[botNum].x)][(int)(bot[botNum].y)]
      [(int)(bot[botNum].z)] == 2) || (world[(int)(bot[botNum].x)]
      [(int)(bot[botNum].y) -1][(int)(bot[botNum].z)] == 2)) {
      bot[botNum].wx = 0-bot[botNum].wx;
      bot[botNum].wz = 0-bot[botNum].wz;
      bot[botNum].x = bot[botNum].x + bot[botNum].wx;
      bot[botNum].z = bot[botNum].z + bot[botNum].wz;
   }

   /* check for worlds edge*/ 
   if (((bot[botNum].x)>=WORLDX) || ((bot[botNum].x)<0) || ((bot[botNum].y)<0)
      || ((bot[botNum].z)>=WORLDZ) || ((bot[botNum].z)<0)) {
      bot[botNum].wx = 0-bot[botNum].wx;
      bot[botNum].wz = 0-bot[botNum].wz;
      bot[botNum].x = bot[botNum].x + bot[botNum].wx;
      bot[botNum].z = bot[botNum].z + bot[botNum].wz;
   }

   /*Collision detection: to stop bots from walking into the player*/
   getViewPosition(&x, &y, &z);
   if (((int)x == (int)bot[botNum].x*-1) && ((int)y == (int)bot[botNum].y*-1)
      && ((int)z == (int)bot[botNum].z*-1)) {
      bot[botNum].wx = 0-bot[botNum].wx;
      bot[botNum].wz = 0-bot[botNum].wz;
      bot[botNum].x = bot[botNum].x + bot[botNum].wx;
      bot[botNum].z = bot[botNum].z + bot[botNum].wz;
   }
}

int botSight(int botNum){

   float x,y,z;
   int xmin,xmax,ymin,ymax,zmin,zmax;
   int i,j,k;


   /*cross product to find line? check for objects between line?*/
   getViewPosition(&x,&y,&z);

   for (i=1; i<BOT_SIGHT_RANGE; i++)
      if (world[(int)((x*-1)
         +(((float)i/BOT_SIGHT_RANGE)*((bot[botNum].x-(x*-1)))))]
         [(int)((y*-1)+((float)i/BOT_SIGHT_RANGE*((bot[botNum].y-(y*-1)))))]
         [(int)((z*-1)+((float)i/BOT_SIGHT_RANGE*((bot[botNum].z-(z*-1)))))]!=0)
               return 0;
   return 1;
}

int botSightRange(int botNum){

   float x,y,z;

   getViewPosition(&x,&y,&z);
   x=x*-1;y=y*-1;z=z*-1;

   /* check if player out of distance range */
   if (lengthTwoPoints(x, y, z,(int)bot[botNum].x,(int)bot[botNum].y
      ,(int)bot[botNum].z) > BOT_SIGHT_RANGE)
      return 0;

   /* check if player is in front of enemy */
      //DIDNT NOTICE TILL LAST MINUTE THAT I DIDNT IMPLEMENT THIS...
      //....BUT LOOK AT ALL THE OTHER COOL STUFF!

   return 1;
}

/* collisionResponse(): Performs collision detection and response
/  -Sets new xyz to position of the viewpoint after collision
/  -Also used to implement gravity by updated viewpoints coordinates
/  -Note that the world coordinates returned from getViewPosition()
/     will be the negative value of the array indices */
void collisionResponse() {

   int i;
   float x,y,z;

   /*Turn off CollisionsResponse when fly control is true*/
   if ( (flycontrol == 0) && !netClient){
      getViewPosition(&x,&y,&z);

      /*Kill player in water*/
      if ((world[(int)(x * -1)][(int)(y * -1)][(int)(z * -1)] == 2)
         || (world[(int)(x * -1)][(int)(y * -1) -1][(int)(z * -1)] == 2))
         playerDeath(0);

      /*Kill player in fire*/
      else if ((world[(int)(x * -1)][(int)(y * -1)][(int)(z * -1)] == 3)
         || (world[(int)(x * -1)][(int)(y * -1) -1][(int)(z * -1)] == 3)){
         popUpFlag = 1;
         sprintf(popUpString, "Indirect Hit!");
         playerDeath(1);
      }

      /*Kill players who dropped out bottoms of map*/
      else if ((y*-1)<0)
         playerDeath(2);

      /*Collision climbing: to allow player to climb single blocks*/
      else if ((world[(int)(x * -1)][(int)(y * -1)][(int)(z * -1)] != 0)
         && (world[(int)(x * -1)][(int)((y * -1) + 1)][(int)(z * -1)] == 0))
         setViewPosition(x,((int)y-1),z);

      /*Collision detection: to stop players from walking into blocks*/
      else if ((world[(int)((x * -1))][(int)((y * -1))][(int)((z * -1))] != 0)
         || (world[(int)((x * -1))][(int)((y * -1))][(int)((z * -1))] != 0)) {
         getOldViewPosition(&x,&y,&z);
         setViewPosition(x,y,z);
      }

      /*Collision boundaries: stop players from walking out of bounds*/
      else if (((x*-1)>=WORLDX) || ((x*-1)<0) || ((y*-1)<0)
         || ((z*-1)>=WORLDZ) || ((z*-1)<0)) {
         getOldViewPosition(&x,&y,&z);
         setViewPosition(x,y,z);
      }

      /*Gravity: pulls player towards ground if there is space beneath him*/
      if (player.jump_flag)
         setViewPosition(x,y-0.25,z);

      else if (((world[(int)(x*-1)][(int)((y*-1) -0.5)][(int)(z*-1)] == 0)
         || (world[(int)(x*-1)][(int)((y*-1) -0.5)][(int)(z*-1)] == 5)))
         setViewPosition(x,y+0.25,z);

      /*Collision detection: to stop players from walking into bots*/
      for (i=0; i<BOT_COUNT; i++){
         if (((int)bot[i].x == (int)x*-1) && ((int)bot[i].y == (int)y*-1)
            && ((int)bot[i].z == (int)z*-1)) {
            getOldViewPosition(&x,&y,&z);
            setViewPosition(x,y,z);
         }
      }
   }
}

/* draw2D(): draws 2D shapes on screen */
void draw2D() {

   /* variable initialization */
   int i, j, k;
   int worldtop[WORLDX][WORLDZ][2];
   static clock_t popUpTime = -(10*CLOCKS_PER_SEC);

   if (testWorld) {
      /* draw some sample 2d shapes */
      GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
      set2Dcolour(green);
      draw2Dline(0, 0, 500, 500, 15);
      draw2Dtriangle(0, 0, 200, 200, 0, 200);

      GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
      set2Dcolour(black);
      draw2Dbox(500, 380, 524, 388);
   } else {

      /* initialize top of world array */
      for (i=0; i<WORLDX; i++){
         for (j=0; j<WORLDZ; j++){
            worldtop[i][j][0] = 0;
            worldtop[i][j][1] = 0;
            for (k=0; k<CLOUD1_LEVEL; k++){
               if (world [i][k][j] != 0){
                  worldtop [i][j][0] = world [i][k][j];
                  worldtop [i][j][1] = k;
               }
            }
         }
      }

      /* show popup based on popupstring */
      if (popUpFlag){
         popUpTime = clock();
         popUpFlag = 0;
      }
      if ((clock()-popUpTime)/CLOCKS_PER_SEC < POP_COUNT)
         popUp();

      /* draw small transparent map in upper-right corner*/
      if (displayMap == 1)
         drawMiniMapSmall(worldtop);

      /* draw large solid map in center of screen and center screen based 
      * larger parameter; height or width */
      else if (displayMap == 2) 
         drawMiniMapLarge(worldtop);
   }
}

/* update(): background process, it is called when there are no other events.
/  -used to control animations and perform calculations while system's running
/  -gravity is also implemented here, duplicating collisionResponse */
void update() {

   float *la, x, y, z;
   int i, j, k;
   float velChange;
   static float mobx[MISSILE_COUNT+BOT_COUNT+1], moby[MISSILE_COUNT+BOT_COUNT+1];
   static float mobz[MISSILE_COUNT+BOT_COUNT+1], mobry[MISSILE_COUNT+BOT_COUNT+1];
   static float mobrx[MISSILE_COUNT+BOT_COUNT+1], mobVel[MISSILE_COUNT+BOT_COUNT+1];
   static float mobAng[MISSILE_COUNT+BOT_COUNT+1], mobinc[MISSILE_COUNT+BOT_COUNT+1];
   static clock_t jump_time, uTime;

   /* sample animation for the test world, don't remove this code */
   if (testWorld) {

      /* rotation and positioning of mob */
      /* coordinates for mob 0 */
      static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
      static float mob0ry = 0.0;
      static int increasingmob0 = 1;
      /* coordinates for mob 1 */
      static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
      static float mob1ry = 0.0;
      static int increasingmob1 = 1;

      /* move mob 0 and rotate */
      setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

      /* move mob 0 in the x axis */
      if (increasingmob0 == 1)
         mob0x += 0.2;
      else
         mob0x -= 0.2;
      if (mob0x > 50) increasingmob0 = 0;
      if (mob0x < 30) increasingmob0 = 1;

      /* rotate mob 0 around the y axis */
      mob0ry += 1.0;
      if (mob0ry > 360.0) mob0ry -= 360.0;

      /* move mob 1 and rotate */
      setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

      /* move mob 1 in the z axis */
      /* when mob is moving away it is visible, when moving back it is hidden */
      if (increasingmob1 == 1) {
         mob1z += 0.2;
         showMob(1);
      } else {
         mob1z -= 0.2;
         hideMob(1);
      }
      if (mob1z > 72) increasingmob1 = 0;
      if (mob1z < 52) increasingmob1 = 1;

      /* rotate mob 1 around the y axis */
      mob1ry += 1.0;
      if (mob1ry > 360.0) mob1ry -= 360.0;

      /* end testworld animation */
   } else {

      /* base update of real timing (hundredth of a second) then reset clock */
      if ((clock() - uTime) / (CLOCKS_PER_SEC/50) >= 1) {
         uTime = clock();

         /* if game is in client mode*/
         if (netClient){
            int changeType;

            /* read changes */
            do {
               read(sockfd, &changeType, sizeof(changeType));
               switch (changeType){
                  case 0:
                     read(sockfd, &wChange, sizeof(wChange));
                     world[wChange[0]][wChange[1]][wChange[2]]=wChange[3];
                  break;
                  case 1:
                     read(sockfd, &p1Change, sizeof(p1Change));
                     setViewPosition(p1Change[0] ,p1Change[1] ,p1Change[2]);
                     read(sockfd, &p1Change, sizeof(p1Change));
                     setViewOrientation(p1Change[0] ,p1Change[1] ,p1Change[2]);
                  break;
                  case 2:
                     read(sockfd, &mChange, sizeof(mChange));
                     MISSILE_FLAG = 1;
                  break;
               } 
            } while (changeType != 1);

            /* Fire missile*/
            if (MISSILE_FLAG > 0){
               for (i=0; i<MISSILE_COUNT; i++){
                  if (mNum[i][0] == 0) {
                     mNum[i][0] = 1;
                     mobAng[i] = (mChange[0] * (PI/180));
                     mobVel[i] = ((float)mChange[1])/100;
                     mobinc[i] = sin(mobAng[i]+0.1)
                        * ((*mobVel*VELOCITY_CONSTRAINT)+0.1);
                     getViewPosition(&mobx[i], &moby[i], &mobz[i]);
                     getViewOrientation(&mobrx[i], &mobry[i], &x);
                     createMob(i, (int)(mobx[i]*-1), (int)(moby[i]*-1)
                        , (int)(mobz[i]*-1), ((int)mobry[i]+180));

                     if (RADIO_ON == 1)
                     printf("-\t MISSILE #%d DEPLOYED\n-\t   VEL:%.2f  ANG:%d\n*\n"
                        , i+1 , player.vel, player.ang);
                     i = MISSILE_COUNT;
                  }
               }
               MISSILE_FLAG--;
            }

            /*Count missiles and animate missiles that are still in the air.*/
            for (i=0; i<MISSILE_COUNT+BOT_COUNT+1; i++){
               if (mNum[i][0] == 1) {
                  mNum[i][0] = fireMissile(i, &mobx[i], &moby[i], &mobz[i], &mobry[i]
                     , &mobrx[i], &mobVel[i], &mobAng[i], &mobinc[i]);
               }
            }
         } else {

            /* if in servermode - send player position changes */
            if (netServer){
               write(client_sockfd, &pType, sizeof(pType));
               getViewPosition(&x,&y,&z);
               p1Change[0]=x; p1Change[1]=y; p1Change[2]=z;
               write(client_sockfd, &p1Change, sizeof(p1Change));
               getViewOrientation(&x,&y,&z);
               p1Change[0]=0-player.ang; p1Change[1]=y; p1Change[2]=z;
               write(client_sockfd, &p1Change, sizeof(p1Change));
            }

            /* update counter, water clouds and position buffers */
            UPDATE_COUNTER++;
            updateWater();
            updateClouds();
            getViewPosition(&x,&y,&z);

            /* update bots */
            for (i=0; i<BOT_COUNT; i++)
               botController(i);

            /* kill player in water*/
            if ((world[(int)(x * -1)][(int)(y * -1)][(int)(z * -1)] == 2)
               || (world[(int)(x * -1)][(int)(y * -1) -1][(int)(z * -1)] == 2))
               playerDeath(0);

            /* kill player in fire*/
            else if ((world[(int)(x * -1)][(int)(y * -1)][(int)(z * -1)] == 3)
               || (world[(int)(x * -1)][(int)(y * -1) -1][(int)(z * -1)] == 3)){
               playerDeath(1);
               popUpFlag = 1;
               sprintf(popUpString, "Indirect Hit!");
            }

            /* kill players who dropped out bottoms of map*/
            else if ((y*-1)<0)
               playerDeath(2);

            /*Gravity: pulls player towards ground if there is space beneath him
            /   -when fly control is set to true*/
            if (flycontrol == 0){    
               if (player.jump_flag)
                  setViewPosition(x,y-0.25,z);
               else if (((world[(int)(x*-1)][(int)((y*-1) -0.5)][(int)(z*-1)]==0)
                  || (world[(int)(x*-1)][(int)((y*-1) -0.5)][(int)(z*-1)]== 5)))
                  setViewPosition(x,y+0.25,z);
            }

            /* count down jump flag*/
            if (player.jump_flag != 0)
               player.jump_flag--;

            /* check jump flag */
            if (space){
               if ((player.jump_flag == 0) && 
                  (world[(int)(x*-1)][(int)((y*-1) -0.5)][(int)(z*-1)] != 0)
                  && ((clock()-jump_time)/CLOCKS_PER_SEC >= JUMP_LAG)){
                  player.jump_flag = 10;
                  jump_time = clock();
               }
               space = 0;
            }

            /*check dig flag*/
            if (dig){
               removeCube();
               dig--;
            }

            /*check all keyboard ang/vel change flags*/
            if (velUp){
               if (player.vel < 1)
                  player.vel+=0.01;
               velUp--;
            }
            else if (velDown){
               if (player.vel > 0.00)
                  player.vel-=0.01;
               velDown--;
            }
            if (angUp){
               if (player.ang < 90)
                  player.ang+=1;
               angUp--;
            }
            else if (angDown){
               if (player.ang > 0)
                  player.ang-=1;
               angDown--;
            }

            /* fire missile*/
            if (MISSILE_FLAG > 0){
               for (i=0; i<MISSILE_COUNT; i++){
                  if (mNum[i][0] == 0) {
                     mNum[i][0] = 1;
                     mobAng[i] = (player.ang * (PI/180));
                     mobVel[i] = player.vel;
                     mobinc[i] = sin(mobAng[i]+0.1)
                        * ((mobVel[i]*VELOCITY_CONSTRAINT)+0.1);
                     getViewPosition(&mobx[i], &moby[i], &mobz[i]);
                     getViewOrientation(&mobrx[i], &mobry[i], &x);
                     if (netServer){
                        write(client_sockfd, &mType, sizeof(mType));
                        mChange[0]=player.ang; mChange[1]=player.vel*100;
                        write(client_sockfd, &mChange, sizeof(mChange));
                     }
                     createMob(i, (int)(mobx[i]*-1), (int)(moby[i]*-1)
                        , (int)(mobz[i]*-1), ((int)mobry[i]+180));
                     if (RADIO_ON == 1)
                     printf("-\t MISSILE #%d DEPLOYED\n-\t VEL:%.2f ANG:%d\n*\n"
                        , i+1 , player.vel, player.ang);
                     i = MISSILE_COUNT;
                  }
               }
               MISSILE_FLAG--;
            }
            //printf("1:%d,2:%d,3:%d,4:%d,5:%d,6:%d,7:%d\n"
            //            ,mNum[0][0], mNum[1][0], mNum[2][0], mNum[3][0], mNum[4][0]
            //            ,mNum[5][0], mNum[6][0]);

            /* Fire bot missile*/
            getViewPosition(&x,&y,&z);
            x=x*-1;y=y*-1;z=z*-1;
            for(i=0; i<BOT_COUNT; i++){
               if (BOT_MIS_FLAG[i]==1){
                  if (mNum[MISSILE_COUNT+i][0] == 0) {
                     mNum[MISSILE_COUNT+i][0] = 1;
                     mobAng[MISSILE_COUNT+i] = (BOT_ANG_LOCK * (PI/180));
                     mobVel[MISSILE_COUNT+i] = ((lengthTwoPoints(x, y
                        , z,(int)bot[i].x,(int)bot[i].y
                        ,(int)bot[i].z)/BOT_SIGHT_RANGE)+0.08)*0.58;
                     mobinc[MISSILE_COUNT+i] = sin(mobAng[MISSILE_COUNT+i]+0.1)
                        * ((mobVel[MISSILE_COUNT+i]*VELOCITY_CONSTRAINT)+0.1);

                     /* change velocity based on y difference */
                     velChange = (float)((int)y - (int)bot[i].y)/150;
                     mobVel[MISSILE_COUNT+i] += velChange; 
                     printf("%f, %d, %d ->%f\n",mobVel[MISSILE_COUNT+i]
                        ,(int)bot[i].y,(int)y, velChange);

                     mobx[MISSILE_COUNT+i] = bot[i].x*-1;
                     moby[MISSILE_COUNT+i] = bot[i].y*-1;
                     mobz[MISSILE_COUNT+i] = bot[i].z*-1;
                     mobry[MISSILE_COUNT+i] = (bot[i].r*-1)+180;
                     if (netServer){
                        write(client_sockfd, &mType, sizeof(mType));
                        mChange[0]=player.ang; mChange[1]=player.vel*100;
                        write(client_sockfd, &mChange, sizeof(mChange));
                     }
                     createMob(MISSILE_COUNT+i, (int)(mobx[MISSILE_COUNT+i]*-1)
                        , (int)(moby[MISSILE_COUNT+i]*-1)
                        , (int)(mobz[MISSILE_COUNT+i]*-1)
                        , ((int)mobry[MISSILE_COUNT+i]));
                  }
                  BOT_MIS_FLAG[i]--;
               }
            }

            /* count missiles and animate missiles that are still in the air.*/
            for (i=0; i<=MISSILE_COUNT+BOT_COUNT; i++){
               if (mNum[i][0] == 1) {
                  mNum[i][0] = fireMissile(i, &mobx[i], &moby[i], &mobz[i]
                     , &mobry[i], &mobrx[i], &mobVel[i], &mobAng[i], &mobinc[i]);
               }
            }

            /* remove explosion fires*/
            if ((clock()-missile_clock)/(CLOCKS_PER_SEC/100) >=25){
               for(i=0; i<WORLDX; i++){
                  for(j=0; j<WORLDY; j++){
                     for(k=0; k<WORLDZ; k++){
                        if (world[i][j][k] == 3){
                           world[i][j][k] = 0;
                           if (netServer){
                              write(client_sockfd, &bType, sizeof(bType));
                              wChange[0]=i;
                              wChange[1]=j;
                              wChange[2]=k;
                              wChange[3]=0;
                              write(client_sockfd, &wChange, sizeof(wChange));
                           }
                        }
                     }
                  }
               }
            }

            /* updates radio beeps*/
            if ((RADIO_BLIP == 1) && (RADIO_ON == 1))
               if (UPDATE_COUNTER % RADIO_SCAN_SPEED == 0)
                  printf("*\n");
         }
      }
   }
}

/* mouse(): Called by GLUT when a mouse button is pressed or released.
/  -button indicates which button was pressed or released
/	-state indicates a button down or button up event
/	-x,y are the screen coordinates when the mouse is pressed or released */
void mouse(int button, int state, int x, int y) {

   static int angBuf;
   static double velBuf;

   /* if the game instance is not the client*/
   if(!netClient){
   
      /*Fire missile on left button release*/
      if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
         MISSILE_FLAG++;

      /*If right button is pushed down, save coordinates*/
      if (RAPIDFIRE_MODE == 0){
         if (button == GLUT_RIGHT_BUTTON){
            if (state != GLUT_UP){
               velBuf = x;
               angBuf = y;
            }

            /*If right button is released, perform calculations */
            else {

               /*Round Velocity*/
               velBuf = roundf((x - velBuf)/1000*100)/100;
               if (player.vel+velBuf > 1)
                  velBuf = 1-player.vel;
               else if (player.vel+velBuf < 0){
                  velBuf = 0-player.vel;
               }
               player.vel += velBuf;

               /*Round Angle*/
               angBuf = (y - angBuf)/10;
               if (player.ang-angBuf > 90)
                  angBuf = player.ang-90;
               else if (player.ang-angBuf < 0){
                  angBuf = player.ang;
               }
               player.ang -= angBuf;


               /*Print radio message*/
               if (RADIO_ON == 1){
                  if (velBuf > 0)
                     printf("-\tVELOCITY INCREASE: %.2f\n", velBuf);
                  else if (velBuf < 0)
                     printf("-\tVELOCITY DECREASE: %.2f\n", velBuf*-1);
                  if (angBuf < 0)
                     printf("-\t   ANGLE INCREASE: %d\n", angBuf*-1);
                  else if (angBuf > 0)
                     printf("-\t   ANGLE DECREASE: %d\n", angBuf);
                  printf("- ANGLE: %d deg     VELOCITY: %.2f m/s\n*\n"
                     , player.ang, player.vel);
               }
           }
         }
      }
   }
}

int main(int argc, char** argv){

   /* variable initialization*/
   int i, j, k;

	/* initialize the graphics system */
   graphicsInit(&argc, argv);
   srand(time(NULL));

	/* build sample world, do not remove this code. The testworld is only
      guaranteed to work with a world of with dimensions of 100,50,100. */
   if (testWorld == 1) {

      /* initialize world to empty */
      for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDY; j++)
            for(k=0; k<WORLDZ; k++)
               world[i][j][k] = 0;

      /* build a red platform */
      for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDZ; j++)
            world[i][24][j] = 3;

      /* create some green and blue cubes */
      world[50][25][50] = 1;
      world[49][25][50] = 1;
      world[49][26][50] = 1;
      world[52][25][52] = 2;
      world[52][26][52] = 2;

      /* blue box shows xy bounds of the world */
      for(i=0; i<WORLDX-1; i++) {
         world[i][25][0] = 2;
         world[i][25][WORLDZ-1] = 2;
      }
      for(i=0; i<WORLDZ-1; i++) {
         world[0][25][i] = 2;
         world[WORLDX-1][25][i] = 2;
      }

      /* create two sample mobs, animated in the update() function */
      createMob(0, 50.0, 25.0, 52.0, 0.0);
      createMob(1, 50.0, 25.0, 52.0, 0.0);

      /* create sample player */
      createPlayer(0, 52.0, 27.0, 52.0, 0.0);
   } else {

      /* initialize world to empty & set fly control to 0*/
      for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDY; j++)
            for(k=0; k<WORLDZ; k++)
               world[i][j][k] = 0;
      flycontrol = 0;
      dig=0;

      /* build world surface using Perlin Noise and fill with water and rock*/
      for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDZ; j++)
            world[i][abs((int)((PerlinNoise_2D(i*0.01, j*0.01, LAND_PERS
               , LAND_OCT)) * -75)-8)][j] = 1;
      generateWorldFeatures();

      /* initialize server & client modes if flags are set*/
      if (netServer){
         netClient = 0;
         serverMode();
      }
      else if (netClient){
         clientMode();
      }

      /* initialize colours*/
      p1ColourTrans[0] = 0.9; p1ColourTrans[1] = 0.9; p1ColourTrans[2] = 1;
      p1ColourTrans[3] = 0.9;
      p1ColourSolid[0] = 0.9; p1ColourSolid[1] = 0.9; p1ColourSolid[2] = 1;
      p1ColourSolid[3] = 1;

      /* initialize velocity and angle */
      player.ang = 45;
      player.vel = 0.5;
      for (i=0; i<MISSILE_COUNT+BOT_COUNT+1; i++){
         mNum[i][0]=0;
      }

      /* initialize AI bots */
      for (i=0; i<BOT_COUNT; i++){
         bot[i].x = rand()%WORLDX;
         bot[i].y = CLOUD1_LEVEL-1;
         bot[i].z = rand()%WORLDZ;
         bot[i].state = BSEARCH;
         bot[i].wx=0; bot[i].wz=0; bot[i].wanderCount = clock();
         createPlayer(i, bot[i].x, bot[i].y, bot[i].z, 0.0);
      }
   }

	/* starts the graphics processing loop, code after this will not run until
      the program exits */
   glutMainLoop();



   /* close sockets */
   if (netServer)
      close(client_sockfd);
   else if (netClient)
      close(sockfd);

   return 0;
}
