/***************************************a1.c*************************************
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

/* Noise(): Returns noise based on input coordinates*/
float Noise(int x, int y){

   int n;

   n = x + y * 57;
   n = (n << 13) ^ n;

   return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff)
    / 1073741824.0);
}

/* CosineInterpolate(): Performs a COS interpolation of input noise*/
float CosineInterpolate(float a, float b, float x) {

   float ft, f;

   ft = x * 3.1415927;
   f = (1 - cos(ft)) * 0.5;

   return  a*(1-f) + b*f;
}

/* SmoothedNoise(): Smooths out and returns input noise*/
float SmoothedNoise(int x, int y) {

   float corners, sides, center;

   corners = (Noise(x-1, y-1) + Noise(x+1, y-1) + Noise(x-1, y+1)
    + Noise(x+1, y+1) ) /16;
   sides   = (Noise(x-1, y)  + Noise(x+1, y)  + Noise(x, y-1)
    + Noise(x, y+1) ) /8;
   center  = Noise(x, y) /4;

   return (corners + sides + center);
}

/* InterpolateNoise(): Uses SmoothedNoise() and CosineInterpolate to
/   produce smooth and interpolated noise */
float InterpolateNoise(float x, float y){

   int integer_X, integer_Y;
   float fractional_X, fractional_Y;
   float v[4], i[2];

   integer_X = (int)x;
   integer_Y = (int)y;
   fractional_X = x - integer_X;
   fractional_Y = y - integer_Y;
   v[0] = SmoothedNoise(integer_X, integer_Y);
   v[1] = SmoothedNoise(integer_X +1, integer_Y);
   v[2] = SmoothedNoise(integer_X, integer_Y +1);
   v[3] = SmoothedNoise(integer_X +1, integer_Y +1);
   i[0] = CosineInterpolate(v[0], v[1], fractional_X);
   i[1] = CosineInterpolate(v[2], v[3], fractional_X);

   return CosineInterpolate(i[0], i[1], fractional_Y);
}

/* PerlinNoise_2D(): Based on persistance and octaves, interpolates
/   multiple noise functions and returns resulting noise between -1 and 1 */
float PerlinNoise_2D(float x, float y, float pers, int oct) {

   int i, freq;
   float total, ampl;

   total = 0.0;
   for (i=0; i<oct; i++){
      freq = pow(2, (float)i);
      ampl = pow(pers, (float)i);
      total = total + InterpolateNoise(x * (float)freq, y * (float)freq) * ampl;
   }

   return total;
}

/* generateWorldFeatures(): Function to fill world under surface*/
void generateWorldFeatures(){

   int i,j,k,l;

   /* fill land with dirt*/
   for (i=0; i<WORLDX; i++)
      for (j=0; j<WORLDY; j++)
         for(k=0; k<WORLDZ; k++)
            if (world[i][j][k] != 0)
               for (l=j-1; l>=BEDROCK_DEPTH; l--)
                  world[i][l][k] = 1;

   /* fill bedrock layer */
   for (i=0; i<WORLDX; i++)
      for (j=0; j<BEDROCK_DEPTH; j++)
         for (k=0; k<WORLDZ; k++)
            world[i][j][k] = 4;

   /* start water layer on sides */
   for (i=0; i<WORLDX; i++){
      for (j=BEDROCK_DEPTH; j<=WATER_DEPTH; j++)
            if (world[i][j][0] == 0)
               world[i][j][0] = 2;
      for (j=BEDROCK_DEPTH; j<=WATER_DEPTH; j++)
            if (world[i][j][WORLDZ-1] == 0)
               world[i][j][WORLDZ-1] = 2;
   }
   for (i=0; i<WORLDZ; i++){
      for (j=BEDROCK_DEPTH; j<=WATER_DEPTH; j++)
            if (world[0][j][i] == 0)
               world[0][j][i] = 2;
      for (j=BEDROCK_DEPTH; j<=WATER_DEPTH; j++)
            if (world[WORLDZ-1][j][i] == 0)
               world[WORLDZ-1][j][i] = 2;
   }

   /* draw cloud layer 1*/
   for(i=0; i<WORLDX; i++)
      for(j=0; j<WORLDZ; j++)
         if (PerlinNoise_2D(i*0.01, j*0.01, CLOUD1_PERS, CLOUD1_OCT) >= 0.2)
            world[i][CLOUD1_LEVEL][j] = 5;

   /* draw cloud layer 2*/
   for(i=0; i<WORLDX; i++)
      for(j=0; j<WORLDZ; j++)
         if (PerlinNoise_2D(i*0.01, j*0.01, CLOUD2_PERS, CLOUD2_OCT) >= 0.2)
            world[i][CLOUD2_LEVEL][j] = 5;
}