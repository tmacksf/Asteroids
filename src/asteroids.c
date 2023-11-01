#include "asteroids.h"
#include "external/TSL/src/base.h"
#include "external/TSL/src/datastructures/dlinkedlist.h"
#include "external/TSL/src/datastructures/queue.h"
#include "external/raylib/src/raylib.h"

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

  printf("Game\n");
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    bool boosting = false;
    if (IsKeyDown(KEY_UP)) {
      boosting = true;
    }

    if (IsKeyPressed(KEY_SPACE))
      shoot(ship, shot_queue);

    moveShip(ship, boosting);
    drawShip(ship, boosting);
    drawShots(shot_queue);
    drawAsteroids(asteroids);

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

void moveShip(Ship *ship, bool boosting) {
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
  Shot s = {.timeLeft = SHOT_TIME,
            .x = ship->center.x + cos(ship->direction) * SHIP_SIZE,
            .y = ship->center.y + -sin(ship->direction) * SHIP_SIZE,
            ship->direction};

  queue_push(q, &s);
}

void drawShots(Queue *q) {
  if (!q->size)
    return;
  QueueNode *node = queue_topNode(q);
  if (((Shot *)node->data)->timeLeft < 0) {
    queue_pop(q);
    node = queue_topNode(q);
  }
  int i = 0;
  while (node) {
    Shot *s = node->data;
    DrawPixel(s->x, s->y, WHITE);
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

    s->timeLeft--;
    node = node->next;
    i++;
  }
}

Asteroid makeAsteroid() {
  Asteroid a;
  // a->vertex_count = 6 + rand() % 14;
  a.vertex_count = 6;
  a.vertices = malloc(a.vertex_count * sizeof(Vec2_I32));
  float angle_slice_size = 2 * PI / a.vertex_count;
  // determine location
  // will be done by splitting the
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
  a.velocity.x = rand() % 40 - 20;
  a.velocity.y = rand() % 40 - 20;
  for (int i = 0; i < a.vertex_count; i++) {
    // going to generate random polar coordinate for each point
    int len = 5 + rand() % 15;
    float angle = angle_slice_size * i +
                  (float)rand() / ((float)RAND_MAX / angle_slice_size);
    // still need to get the x and y and put them into asteroid
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
    drawAsteroid(node->data);
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
