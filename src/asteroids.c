#include "asteroids.h"
#include "external/TSL/src/datastructures/queue.h"
#include "external/raylib/src/raylib.h"

int runGame() {
  SetTargetFPS(60);

  InitWindow(WIDTH, HEIGHT, "Asteroids");
  const Vec2_I32 center = {.x = WIDTH / 2, .y = HEIGHT / 2};
  printf("Center: %d, %d\n", center.x, center.y);

  Ship *ship = makeShip(center.x, center.y);
  Queue *shot_queue = queue_init(sizeof(Shot));

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
  if (boosting) {
    ship->velocity.x += 2.0f * cos(ship->direction);
    ship->velocity.y += 2.0f * -sin(ship->direction);
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
            .y = ship->center.y + -sin(ship->direction) * SHIP_SIZE};

  // queue_push(q, &s);
  printf("%d, %lu\n", q->datasize, sizeof(Shot));
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
  /*
  while (node) {
    Shot *s = node->data;
    DrawPixel(s->x, s->y, WHITE);
    // update position
    s->x += 1;
    s->y -= 1;

    s->timeLeft--;
    node = node->next;
    i++;
  }
  if (i != q->size)
    printf("Error moment\n");
  */
}
