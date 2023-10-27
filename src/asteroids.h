#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include "external/TSL/src/base.h"
#include "external/TSL/src/datastructures/queue.h"
#include "external/raylib/src/raylib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 1200
#define HEIGHT 800

#define SHIP_SIZE 30

#define MAX_VELOCITY 20

#define SHOT_TIME 300
#define SHOT_SIZE 5

typedef struct Ship {
  Vec2 center;
  int dist;
  double direction; // in radians
  Vec2 velocity;
} Ship;

typedef struct Asteroid {
  Vec2_I32 *vertices;
} Asteroid;

typedef struct Shot {
  I32 timeLeft;
  float x;
  float y;
  double direction; // radians
} Shot;

// Game stuff

// Runs the game
int runGame();

// Ship stuff

// make ship
Ship *makeShip(int x, int y);
// drawship
void drawShip(Ship *ship, bool boosting);
// ship movement
void moveShip(Ship *ship, bool boosting);
// shoot
void shoot(const Ship *ship, Queue *q);
// draw shots
void drawShots(Queue *q);

// Asteroid stuff

// makes a new asteroid with a random number of vertices (between 4 and 10)
Asteroid *makeAsteroid();

#endif
