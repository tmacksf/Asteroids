#include "asteroids.h"
#include "external/TSL/src/datastructures/dlinkedlist.h"
#include "external/TSL/src/datastructures/queue.h"
#include "external/raylib/src/raylib.h"
#include <stdio.h>

int runGame() {
  SetTargetFPS(60);

  InitWindow(WIDTH, HEIGHT, "Asteroids");
  const Vec2_I32 center = {.x = WIDTH / 2, .y = HEIGHT / 2};
  printf("Center: %d, %d\n", center.x, center.y);

  Ship *ship = makeShip(center.x, center.y);
  Queue *shot_queue = queue_init(sizeof(Shot));
  DLinkedList *asteroids = dlist_init(sizeof(Asteroid));

  for (int i = 0; i < 5; i++) {
    Asteroid a = makeAsteroid();
    dlist_append(asteroids, &a);
  }

  // time until new asteroid is spawned
  int spawn_countdown = 1000;
  int paused = 1;
  int debug = DEBUG;
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    if (IsKeyPressed(KEY_P)) {
      paused = 1 - paused;
    }

    bool boosting = false;
    if (IsKeyDown(KEY_UP)) {
      boosting = true;
    }
    // draw stuff
    drawShip(ship, boosting);
    drawShots(shot_queue);
    drawAsteroids(asteroids);

    if (IsKeyPressed(KEY_SPACE))
      shoot(ship, shot_queue);

    if (debug) {
      // output debug info
    }

    if (!paused) {
      updateShip(ship, boosting);
      updateAsteroids(asteroids);
      shotAsteroidsCollisions(asteroids, shot_queue);
      updateShots(shot_queue);
      if (shipAsteroidCollision(asteroids, ship))
        paused = 1 - paused;
    }

    EndDrawing();
  }
  CloseWindow();
  return 0;
}

Ship *makeShip(const int x, const int y) {
  Ship *ship = malloc(sizeof(Ship));
  // velocity
  ship->velocity.x = 0.0f;
  ship->velocity.y = 0.0f;
  // center
  ship->center.x = x;
  ship->center.y = y;
  ship->vertex_count = 3;
  ship->vertices = malloc(sizeof(Vec2) * ship->vertex_count);
  // vertices
  ship->vertices[0].x = cos(ship->direction) * SHIP_SIZE;
  ship->vertices[0].y = -sin(ship->direction) * SHIP_SIZE;
  ship->vertices[1].x = cos(ship->direction + 2.0f * PI / 3) * SHIP_SIZE;
  ship->vertices[1].y = -sin(ship->direction + 2.0f * PI / 3) * SHIP_SIZE;
  ship->vertices[2].x = cos(ship->direction - 2.0f * PI / 3) * SHIP_SIZE;
  ship->vertices[2].y = -sin(ship->direction - 2.0f * PI / 3) * SHIP_SIZE;

  ship->direction = PI / 2.0f;

  return ship;
}

void drawShip(Ship *ship, bool boosting) {
  // could probably load this into a texture but for now this is fine
  // have to draw 3 lines

  // need to flip y axis to make it intuitiva
  Vec2 top = {cos(ship->direction) * SHIP_SIZE,
              -sin(ship->direction) * SHIP_SIZE};
  Vec2 left = {cos(ship->direction + 2.0f * PI / 3) * SHIP_SIZE,
               -sin(ship->direction + 2.0f * PI / 3) * SHIP_SIZE};
  Vec2 right = {cos(ship->direction - 2.0f * PI / 3) * SHIP_SIZE,
                -sin(ship->direction - 2.0f * PI / 3) * SHIP_SIZE};

  // update values in ship
  ship->vertices[0].x = ship->center.x + top.x;
  ship->vertices[0].y = ship->center.y + top.y;
  ship->vertices[1].x = ship->center.x + left.x;
  ship->vertices[1].y = ship->center.y + left.y;
  ship->vertices[2].x = ship->center.x + right.x;
  ship->vertices[2].y = ship->center.y + right.y;

  DrawLine(ship->vertices[0].x, ship->vertices[0].y, ship->vertices[1].x,
           ship->vertices[1].y, RED);
  DrawLine(ship->vertices[0].x, ship->vertices[0].y, ship->vertices[2].x,
           ship->vertices[2].y, BLUE);
  DrawLine(ship->vertices[1].x, ship->vertices[1].y, ship->vertices[2].x,
           ship->vertices[2].y, WHITE);

  if (boosting)
    DrawLine(ship->center.x, ship->center.y,
             ship->center.x + cos(ship->direction - PI) * 20,
             ship->center.y + -sin(ship->direction - PI) * 20, RED);

  Vector2 c = {.x = (float)ship->center.x, .y = (float)ship->center.y};
  DrawPixelV(c, WHITE);

  // TODO: an additional two for the tail thingy
}

void updateShip(Ship *ship, bool boosting) {
  if (IsKeyDown(KEY_LEFT)) {
    if (ship->direction > PI * 2.0f)
      ship->direction = 0.0f;
    ship->direction += PI / 40.0f;
  }
  if (IsKeyDown(KEY_RIGHT)) {
    if (ship->direction < 0.0f)
      ship->direction = 2.0f * PI;
    ship->direction -= PI / 40.0f;
  }

  // accelerate here
  const float acceleration = 0.1f;
  if (boosting) {
    ship->velocity.x += acceleration * cos(ship->direction);
    ship->velocity.y -= acceleration * sin(ship->direction);
  }

  // keep velocity below max
  if (ship->velocity.x > MAX_VELOCITY)
    ship->velocity.x = MAX_VELOCITY;
  if (ship->velocity.y > MAX_VELOCITY)
    ship->velocity.y = MAX_VELOCITY;

  if (ship->velocity.x < -MAX_VELOCITY)
    ship->velocity.x = -MAX_VELOCITY;
  if (ship->velocity.y < -MAX_VELOCITY)
    ship->velocity.y = -MAX_VELOCITY;

  // update location
  ship->center.x = ship->center.x + ship->velocity.x;
  ship->center.y = ship->center.y + ship->velocity.y;

  // make sure in bounds
  if (ship->center.x > WIDTH)
    ship->center.x = 0.0f;
  if (ship->center.x < 0)
    ship->center.x = WIDTH;
  if (ship->center.y > HEIGHT)
    ship->center.y = 0.0f;
  if (ship->center.y < 0)
    ship->center.y = HEIGHT;
}

void shoot(const Ship *ship, Queue *q) {
  Shot s = {
      .timeLeft = SHOT_TIME,
      .x = ship->center.x + cos(ship->direction) * SHIP_SIZE,
      .y = ship->center.y + -sin(ship->direction) * SHIP_SIZE,
      .direction = ship->direction,
      .active = 1,
  };

  queue_push(q, &s);
}

void drawShots(Queue *q) {
  if (!q->size)
    return;
  QueueNode *node = queue_topNode(q);
  while (node) {
    Shot *s = node->data;
    if (!s->active) {
      node = node->next;
      continue;
    }
    DrawPixel(s->x, s->y, WHITE);
    // update position
    node = node->next;
  }
}

void updateShots(Queue *q) {
  if (!q->size)
    return;
  QueueNode *node = queue_topNode(q);
  if (((Shot *)node->data)->timeLeft < 0) {
    queue_pop(q);
    node = queue_topNode(q);
  }
  while (node) {
    Shot *s = node->data;
    if (!s->active) {
      node = node->next;
      s->timeLeft--;
      continue;
    }
    // update position
    s->x += 10 * cos(s->direction);
    s->y -= 10 * sin(s->direction);
    if (s->x > WIDTH) {
      s->x = 0;
    }
    if (s->x < 0) {
      s->x = WIDTH;
    }

    if (s->y > HEIGHT) {
      s->y = 0;
    }
    if (s->y < 0) {
      s->y = HEIGHT;
    }

    // subtract time and set to next asteroid
    s->timeLeft--;
    node = node->next;
  }
}

Asteroid makeAsteroid() {
  Asteroid a;
  a.vertex_count = 6 + rand() % 14;
  // a.vertex_count = 6;
  a.vertices = malloc(a.vertex_count * sizeof(Vec2));
  float angle_slice_size = 2 * PI / a.vertex_count;
  // TODO: For asteroid hitbox: keep an average of the distance from the
  // center of the asteroid and use that as a radius for the hitbox

  // determine location based on random
  // will be done by splitting the spawning loactions into 4 places
  int location = rand() % 4;
  switch (location) {
  // left
  case 1:
    a.center.x = 0;
    a.center.y = rand() % HEIGHT;
    break;
  // right
  case 2:
    a.center.x = WIDTH;
    a.center.y = rand() % HEIGHT;
    break;
  // top
  case 3:
    a.center.x = rand() % WIDTH;
    a.center.y = 0;
    break;
  // bottom
  default:
    a.center.x = rand() % WIDTH;
    a.center.y = HEIGHT;
    break;
  }
  a.velocity.x = rand() % 10 - 5;
  a.velocity.y = rand() % 10 - 5;
  for (int i = 0; i < a.vertex_count; i++) {
    // going to generate random polar coordinate for each point
    int len = 10 + rand() % 20;
    float angle = angle_slice_size * i +
                  (float)rand() / ((float)RAND_MAX / angle_slice_size);
    // adding random x and y to asteroid (generated from polar coordinates)
    a.vertices[i].x = (float)len * cos(angle);
    a.vertices[i].y = (float)len * -sin(angle);
  }
  return a;
}

void drawAsteroid(Asteroid *a) {
  for (int i = 1; i < a->vertex_count; i++) {
    DrawLine(a->center.x + a->vertices[i - 1].x,
             a->center.y + a->vertices[i - 1].y, a->center.x + a->vertices[i].x,
             a->center.y + a->vertices[i].y, WHITE);
  }
  DrawLine(a->center.x + a->vertices[0].x, a->center.y + a->vertices[0].y,
           a->center.x + a->vertices[a->vertex_count - 1].x,
           a->center.y + a->vertices[a->vertex_count - 1].y, WHITE);
}

void drawAsteroids(DLinkedList *asteroids) {
  if (!asteroids->size)
    return;
  DLLNode *node = asteroids->head;
  while (node) {
    Asteroid *temp = node->data;
    drawAsteroid(temp);
    node = node->next;
  }
}

void updateAsteroids(DLinkedList *asteroids) {
  if (!asteroids->size)
    return;
  DLLNode *node = asteroids->head;
  while (node) {
    Asteroid *temp = node->data;
    temp->center.x += temp->velocity.x;
    temp->center.y += temp->velocity.y;
    if (temp->center.x > WIDTH)
      temp->center.x = 0;
    if (temp->center.x < 0)
      temp->center.x = WIDTH;
    if (temp->center.y > HEIGHT)
      temp->center.y = 0;
    if (temp->center.y < 0)
      temp->center.y = HEIGHT;
    node = node->next;
  }
}

int lineIntersection(float p0_x, float p0_y, float p1_x, float p1_y, float p2_x,
                     float p2_y, float p3_x, float p3_y) {
  // https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
  float s1_x, s1_y, s2_x, s2_y;
  s1_x = p1_x - p0_x;
  s1_y = p1_y - p0_y;
  s2_x = p3_x - p2_x;
  s2_y = p3_y - p2_y;
  float denom = s1_x * s2_y - s1_y * s2_x;
  if (!denom)
    return 0;

  bool denomPositive = denom > 0;
  float s02_x = p0_x - p2_x;
  float s02_y = p0_y - p2_y;
  float s_numer = s1_x * s02_y - s1_y * s02_x;
  if ((s_numer < 0) == denomPositive)
    return 0; // No collision

  float t_numer = s2_x * s02_y - s2_y * s02_x;
  if ((t_numer < 0) == denomPositive)
    return 0; // No collision
  if (((s_numer > denom) == denomPositive) ||
      ((t_numer > denom) == denomPositive))
    return 0; // No collision

  return 1;
}

// returns 0 if there is no intersection and 1 if intersection

int shipIntersection(const Asteroid *a, const Ship *s) {
  // brute force find all intersections
  const int vc = a->vertex_count;
  // discard asteroids that are jtoo far
  float dist = sqrt(pow(a->center.x + s->center.x, 2.0f) +
                    pow(a->center.y + s->center.y, 2.0f));
  if (dist > 60.0f)
    return 0;

  printf("Dist %f", dist);
  float cx = a->center.x;
  float cy = a->center.y;
  // if they are too far to interact

  for (int i = 0; i < vc; i++) {
    // TODO: Remove 3 and replace with variable in the ship
    float ax_1 = a->vertices[i - 1].x + cx;
    float ay_1 = a->vertices[i - 1].y + cy;
    float ax_2 = a->vertices[i].x + cx;
    float ay_2 = a->vertices[i].y + cy;
    for (int j = 1; j < 3; j++) {
      if (lineIntersection(s->vertices[i - 1].x, s->vertices[i - 1].y,
                           s->vertices[i].x, s->vertices[i].y, ax_1, ay_1, ax_2,
                           ay_2))
        return 1;
    }
    // check first to last
    if (lineIntersection(s->vertices[0].x, s->vertices[0].y,
                         s->vertices[s->vertex_count - 1].x,
                         s->vertices[s->vertex_count - 1].y, ax_1, ay_1, ax_2,
                         ay_2))
      return 1;
  }
  return 0;
}

int shotIntersection(const Asteroid *a, const Shot *s) {
  // keeping track of intersections of both line segements to make sure its
  // inside
  int intersectionCountBelow = 0;
  int intersectionCountAbove = 0;
  const int vc = a->vertex_count;
  // line segments for the ray cast
  float xRay = s->x;
  float yRayBelow = (float)HEIGHT;
  float yRayAbove = 0.0f;
  // center of the asteroid for the offset
  float cx = a->center.x;
  float cy = a->center.y;
  for (int i = 1; i < a->vertex_count; i++) {
    // check shot line segment against single asteroid line
    // segment
    float ax_1 = a->vertices[i - 1].x + cx;
    float ay_1 = a->vertices[i - 1].y + cy;
    float ax_2 = a->vertices[i].x + cx;
    float ay_2 = a->vertices[i].y + cy;

    intersectionCountBelow +=
        lineIntersection(s->x, s->y, xRay, yRayBelow, ax_1, ay_1, ax_2, ay_2);
    intersectionCountAbove +=
        lineIntersection(s->x, s->y, xRay, yRayAbove, ax_1, ay_1, ax_2, ay_2);
  }
  // have to check the first and last for final line segment of asteroid
  intersectionCountBelow += lineIntersection(
      s->x, s->y, xRay, yRayBelow, a->vertices[0].x + cx, a->vertices[0].y + cy,
      a->vertices[vc - 1].x + cx, a->vertices[vc - 1].y + cy);
  intersectionCountAbove += lineIntersection(
      s->x, s->y, xRay, yRayAbove, a->vertices[0].x + cx, a->vertices[0].y + cy,
      a->vertices[vc - 1].x + cx, a->vertices[vc - 1].y + cy);

  return (intersectionCountAbove == 1 && intersectionCountBelow == 1) ? 1 : 0;
}

// TODO: Optimise the search away from brute force
int shotAsteroidsCollisions(DLinkedList *asteroids, Queue *shots) {
  // loop through asteroids. for each loop through all shots
  // to see if any intersect
  DLLNode *node = asteroids->head;
  for (int i = 0; i < asteroids->size; i++) {
    Asteroid *a = node->data;
    QueueNode *shot = shots->front;
    for (int j = 0; j < shots->size; j++) {
      Shot *s = shot->data;
      if (!s->active) {
        shot = shot->next;
        continue;
      }

      if (shotIntersection(a, s)) {
        // destroy asteroid
        s->active = 0;
        DrawLine(s->x, s->y, s->x, HEIGHT, RED);
        printf("Asteroid center: %f, %f, Shot Center: %f, %f\n", a->center.x,
               a->center.y, s->x, s->y);
        dlist_removeNode(asteroids, node);
        return 0; // TODO: Fix this so that we can remove more
                  // than 1 asteroid per frame
      } else {
        DrawLine(s->x, s->y, s->x, HEIGHT, BLUE);
      }
      shot = shot->next;
    }
    node = node->next;
  }
  return 0;
}

int shipAsteroidCollision(DLinkedList *asteroids, Ship *ship) {
  DLLNode *node = asteroids->head;
  for (int i = 0; i < asteroids->size; i++) {
    Asteroid *a = node->data;
    if (shipIntersection(a, ship)) {
      printf("Collision\n");
      return 1;
    }
    node = node->next;
  }
  return 0;
}

Asteroid customAsteroid(int vcount, int x, int y) {
  Asteroid a;
  a.vertex_count = vcount;
  a.center.x = (float)x;
  a.center.y = (float)y;
  a.velocity.x = 0.0f;
  a.velocity.y = 0.0f;
  a.vertices = malloc(sizeof(Vec2) * vcount);
  float angle_slice_size = 2 * PI / a.vertex_count;
  for (int i = 0; i < a.vertex_count; i++) {
    // going to generate random polar coordinate for each
    // point
    int len = 10 + rand() % 20;
    float angle = angle_slice_size * i +
                  (float)rand() / ((float)RAND_MAX / angle_slice_size);
    // still need to get the x and y and put them into
    // asteroid
    a.vertices[i].x = (float)len * cos(angle);
    a.vertices[i].y = (float)len * -sin(angle);
    printf("x: %f, y: %f\n", a.vertices[i].x, a.vertices[i].y);
  }
  return a;
}

Shot customShot(bool intersecting) {
  Shot s = {100000, WIDTH / 2.0f, HEIGHT / 2.0f, -PI / 2, 1};
  return s;
}
