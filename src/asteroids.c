#include "asteroids.h"
#include "external/TSL/src/datastructures/dlinkedlist.h"
#include "external/TSL/src/datastructures/queue.h"

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
  DLLNode *n = asteroids->head;
  n = n->next;
  dlist_removeNodeAt(asteroids, 1);

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
      // check the collisions here
    }

    if (!paused) {
      updateShip(ship, boosting);
      updateAsteroids(asteroids);
      handleCollisions(ship, asteroids, shot_queue);
      updateShots(shot_queue);
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

  ship->dist = 10;
  ship->direction = PI / 2.0f;

  return ship;
}

void drawShip(Ship *ship, bool boosting) {
  // could probably load this into a texture but for now this is fine
  // have to draw 3 lines

  // need to flip y axis so make it negative
  Vec2 top = {cos(ship->direction) * SHIP_SIZE,
              -sin(ship->direction) * SHIP_SIZE};
  Vec2 left = {cos(ship->direction + 2.0f * PI / 3) * SHIP_SIZE,
               -sin(ship->direction + 2.0f * PI / 3) * SHIP_SIZE};
  Vec2 right = {cos(ship->direction - 2.0f * PI / 3) * SHIP_SIZE,
                -sin(ship->direction - 2.0f * PI / 3) * SHIP_SIZE};
  DrawLine(ship->center.x + top.x, ship->center.y + top.y,
           ship->center.x + left.x, ship->center.y + left.y, RED);
  DrawLine(ship->center.x + top.x, ship->center.y + top.y,
           ship->center.x + right.x, ship->center.y + right.y, BLUE);
  DrawLine(ship->center.x + left.x, ship->center.y + left.y,
           ship->center.x + right.x, ship->center.y + right.y, WHITE);

  if (boosting)
    DrawLine(ship->center.x, ship->center.y,
             ship->center.x + cos(ship->direction - PI) * 20,
             ship->center.y + -sin(ship->direction - PI) * 20, RED);

  Vector2 c = {.x = (float)ship->center.x, .y = (float)ship->center.y};
  DrawPixelV(c, WHITE);

  // and an additional two for the tail thingy
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
static int shipIntersection(const Ship *ship, const Asteroid *a) {
  // sweep line algorithm
  return 0;
}

int shotIntersection(const Asteroid *a, const Shot *s) {
  // going to do a ray cast and see how many times a shot intersects the
  // asteroid. If 1 then inside else outside

  // line segment for the ray cast
  int intersectionCount = 0;
  const int vc = a->vertex_count;
  float sx = s->x;
  float sy = (float)HEIGHT;
  // center of the asteroid for the offset
  float cx = a->center.x;
  float cy = a->center.y;
  for (int i = 1; i < a->vertex_count; i++) {
    // check shot line segment against single asteroid line segment
    float ax_1 = a->vertices[i - 1].x + cx;
    float ay_1 = a->vertices[i - 1].y + cy;
    float ax_2 = a->vertices[i].x + cx;
    float ay_2 = a->vertices[i].y + cy;
    if (lineIntersection(s->x, s->y, sx, sy, ax_1, ay_1, ax_2, ay_2))
      intersectionCount++;
  }
  // have to check the first and last for final line segment that closes it
  if (lineIntersection(s->x, s->y, sx, sy, a->vertices[0].x + cx,
                       a->vertices[0].y + cy, a->vertices[vc - 1].x + cx,
                       a->vertices[vc - 1].y + cy))
    intersectionCount++;

  return intersectionCount % 2 == 0 ? 0 : 1;
}

// TODO: Optimise the search away from brute force
int handleCollisions(const Ship *ship, DLinkedList *asteroids, Queue *shots) {
  // loop through asteroids. for each loop through all shots to see if any
  // intersect
  DLLNode *asteroid = asteroids->head;
  for (int i = 0; i < asteroids->size; i++) {
    Asteroid *a = asteroid->data;
    QueueNode *shot = shots->front;
    for (int j = 0; j < shots->size; j++) {

#if DEBUG
      if (!shot) {
        printf("QUEUE WRONG LENGTH\n Queue size: %d\n", shots->size);
        exit(1);
      }
      if (!asteroid) {
        printf("DLL WRONG LENGTH\n DLL size: %d\n", asteroids->size);
        exit(1);
      }
#endif

      Shot *s = shot->data;
      if (!s->active)
        continue;

      if (shotIntersection(a, s)) {
        // destroy asteroid
        dlist_removeNode(asteroids, asteroid);
        s->active = 0;
        DrawLine(s->x, s->y, s->x, HEIGHT, RED);
        break;
      } else {
        DrawLine(s->x, s->y, s->x, HEIGHT, BLUE);
      }
      shot = shot->next;
    }
    asteroid = asteroid->next;
  }

  // ship: if it has collided then return 1. No collision return 0
  for (int i = 0; i < asteroids->size; i++) {
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
    // going to generate random polar coordinate for each point
    int len = 10 + rand() % 20;
    float angle = angle_slice_size * i +
                  (float)rand() / ((float)RAND_MAX / angle_slice_size);
    // still need to get the x and y and put them into asteroid
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
