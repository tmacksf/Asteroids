#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include "external/TSL/src/base.h"
#include "external/TSL/src/datastructures/dlinkedlist.h"
#include "external/TSL/src/datastructures/queue.h"
#include "external/raylib/src/raylib.h"
#include <math.h>
#include <string.h>

#define WIDTH 1200
#define HEIGHT 800

#define DEBUG 1

#define SHIP_SIZE 30

#define MAX_VELOCITY 5

#define SHOT_TIME 300
#define SHOT_SIZE 5

// asteroid constants
#define ASTEROID_MIN_VERTEX_COUNT 5
#define ASTEROID_MAX_VERTEX_COUNT 20

typedef struct Ship {
  Vec2 center;
  int dist;
  double direction; // in radians
  Vec2 velocity;
} Ship;

typedef struct Asteroid {
  U16 vertex_count;
  Vec2 *vertices;
  Vec2 center;
  Vec2 velocity;
} Asteroid;

typedef struct Shot {
  I32 timeLeft;
  float x;
  float y;
  double direction; // radians
  char active;
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
void updateShip(Ship *ship, bool boosting);
// shoot
void shoot(const Ship *ship, Queue *q);
// draw shots
void drawShots(Queue *q);
// updates the position of the shots
void updateShots(Queue *q);

// makes a new asteroid with a random number of vertices (between 4 and 10)
Asteroid makeAsteroid();
// draws and individual asteroid
void drawAsteroid(Asteroid *a);

// draws asteroids
void drawAsteroids(DLinkedList *asteroids);
// updates asteroids
void updateAsteroids(DLinkedList *asteroids);

// collisions
int handleCollisions(const Ship *ship, DLinkedList *asteroids, Queue *shots);

int lineIntersection(float p0_x, float p0_y, float p1_x, float p1_y, float p2_x,
                     float p2_y, float p3_x, float p3_y);

int shotIntersection(const Asteroid *a, const Shot *s);

#if DEBUG
// TODO: Add a way to store debug info: Asteroid count, shot count, number of
// intersections, if ship is intersecting asteroid, etc

// make a custom asteroid
Asteroid customAsteroid(int vcount, int x, int y);

Shot customShot(bool intersecting);

#endif

#endif
