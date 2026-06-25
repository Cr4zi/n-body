#include <raylib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

struct Vec2 {
  double x;
  double y;
};

static const int32_t WIDTH = 1280;
static const int32_t HEIGHT = 1280;
static const size_t BODIES = 5000; // multiples of 8
static const double MASS = 500000.;
static const double GRAVITY_CONST = .5;
static const float RADIUS = 5;

struct Vec2 *accelerations, *velocities, *positions;

struct Vec2 vec2_add(struct Vec2 *vec1, struct Vec2 *vec2) {
  struct Vec2 vec = {.x = vec1->x + vec2->x, .y = vec1->y + vec2->y};
  return vec;
}

struct Vec2 vec2_sub(struct Vec2 *vec1, struct Vec2 *vec2) {
  struct Vec2 vec = {.x = vec1->x - vec2->x, .y = vec1->y - vec2->y};
  return vec;
}

double vec2_len(struct Vec2 *vec) {
  return sqrt(vec->x * vec->x + vec->y * vec->y);
}

struct Vec2 vec2_scalar(struct Vec2 *vec, double scalar) {
  struct Vec2 out = {.x = vec->x * scalar, .y = vec->y * scalar};
  return out;
}

int32_t rand_xy(int32_t x, int32_t y) {
  return (rand() % (y - 1)) + x;
}

double rand_real() {
  return (double)rand() / RAND_MAX;
}

void init(void) {
  accelerations = (struct Vec2 *)calloc(BODIES, sizeof(struct Vec2));
  velocities = (struct Vec2 *)malloc(BODIES * sizeof(struct Vec2));
  positions = (struct Vec2 *)malloc(BODIES * sizeof(struct Vec2));

  for (size_t i = 0; i < BODIES; ++i) {
    positions[i].x = rand_xy(10, WIDTH);
    positions[i].y = rand_xy(10, WIDTH);

    velocities[i].x = rand_real();
    velocities[i].y = rand_real();
  }
}

void deinit(void) {
  free(accelerations);
  free(velocities);
  free(positions);
}

void calculate_accelerations(void) {
  for (size_t i = 0; i < BODIES; ++i) {
    accelerations[i].x = 0;
    accelerations[i].y = 0;

    for (size_t j = 0; j < BODIES; ++j) {
      struct Vec2 r = vec2_sub(&positions[i], &positions[j]);
      double len = vec2_len(&r);
      len = len <= 2 * RADIUS ? RADIUS : len;
      r = vec2_scalar(&r, 1 / len);

      struct Vec2 added_accel = vec2_scalar(&r, GRAVITY_CONST * (MASS / (len * len)));
      accelerations[i] = vec2_sub(&accelerations[i], &added_accel);
    }
  }
}

void calculate_velocities(void) {
  for (size_t i = 0; i < BODIES; ++i) {
    struct Vec2 added_vel = vec2_scalar(&accelerations[i], GetFrameTime());

    velocities[i] = vec2_add(&velocities[i], &added_vel);
  }
}

void calculate_positions(void) {
  for (size_t i = 0; i < BODIES; ++i) {
    struct Vec2 added_pos = vec2_scalar(&velocities[i], GetFrameTime());

    positions[i] = vec2_add(&positions[i], &added_pos);
  }
}

void simulate(void) {
  calculate_accelerations();
  calculate_velocities();
  calculate_positions();
}

void draw_bodies(void) {
  for (size_t i = 0; i < BODIES; ++i) {
    DrawCircle(positions[i].x, positions[i].y, RADIUS, RED);
  }
}

int main(void) {
  init();
  InitWindow(WIDTH, HEIGHT, "N-Body");
  SetTargetFPS(60);

  clock_t time;
  while (!WindowShouldClose()) {
    time = clock();
    simulate();
    time = clock() - time;
    printf("Took %f seconds to calculate sim\n", (float)time / CLOCKS_PER_SEC);
    simulate();
    
    BeginDrawing();
    ClearBackground(BLACK);

    draw_bodies();
    EndDrawing();
  }

  CloseWindow();
  
  deinit();
  return 0;
}
