#include <raylib.h>
#include <immintrin.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

struct Vec2 {
  float *x;
  float *y;
};

static const int32_t WIDTH = 1280;
static const int32_t HEIGHT = 1280;
static const size_t BODIES = 240;
static const double MASS = 500000.;
static const double GRAVITY_CONST = .5;
static const float RADIUS = 5;

struct Vec2 *accelerations, *velocities, *positions;

int32_t rand_xy(int32_t x, int32_t y) {
  return (rand() % (y - 1)) + x;
}

float rand_real() {
  return (float)rand() / RAND_MAX;
}

void init(void) {
  accelerations = (struct Vec2 *)malloc(sizeof(struct Vec2));
  accelerations->x = (float *)calloc(BODIES, sizeof(float));
  accelerations->y = (float *)calloc(BODIES, sizeof(float));
  
  velocities = (struct Vec2 *)malloc(sizeof(struct Vec2));
  velocities->x = (float *)malloc(BODIES * sizeof(float));
  velocities->y = (float *)malloc(BODIES * sizeof(float));

  positions = (struct Vec2 *)malloc(sizeof(struct Vec2));
  positions->x = (float *)malloc(BODIES * sizeof(float));
  positions->y = (float *)malloc(BODIES * sizeof(float));

  for (size_t i = 0; i < BODIES; ++i) {
    positions->x[i] = rand_xy(10, WIDTH);
    positions->y[i] = rand_xy(10, WIDTH);

    velocities->x[i] = rand_real();
    velocities->y[i] = rand_real();
  }
}

void deinit(void) {
  free(accelerations);
  free(velocities);
  free(positions);
}

void calculate_accelerations(void) {
  for (size_t i = 0; i < BODIES; ++i) {
    accelerations->x[i] = 0;
    accelerations->y[i] = 0;

    for (size_t j = 0; j < BODIES; ++j) {
      if (i == j)
        continue;

      float dx = positions->x[i] - positions->x[j];
      float dy = positions->y[i] - positions->y[j];
      float len = sqrt(dx * dx + dy * dy);
      dx /= len;
      dy /= len;
      
      float force = GRAVITY_CONST * (MASS / (len * len));
      accelerations->x[i] -= force * dx;
      accelerations->y[i] -= force * dy;

    }
  }
}

void calculate_velocities(void) {
  for (size_t i = 0; i < BODIES; i += 8) {
    float dt = GetFrameTime();
    __m256 dts = _mm256_broadcast_ss(&dt);
    __m256 accel_x = _mm256_loadu_ps(&accelerations->x[i]);
    __m256 accel_y = _mm256_loadu_ps(&accelerations->y[i]);

    __m256 added_accel_x = _mm256_mul_ps(accel_x, dts);
    __m256 added_accel_y = _mm256_mul_ps(accel_y, dts);

    __m256 vel_x = _mm256_add_ps(_mm256_loadu_ps(&velocities->x[i]), added_accel_x);
    __m256 vel_y = _mm256_add_ps(_mm256_loadu_ps(&velocities->y[i]), added_accel_y);

    _mm256_storeu_ps(&velocities->x[i], vel_x);
    _mm256_storeu_ps(&velocities->y[i], vel_y);
  }
}

// Pretty much same calculation as velocities
void calculate_positions(void) {
  for (size_t i = 0; i < BODIES; i += 8) {
    float dt = GetFrameTime();
    __m256 dts = _mm256_broadcast_ss(&dt);
    __m256 vel_x = _mm256_loadu_ps(&velocities->x[i]);
    __m256 vel_y = _mm256_loadu_ps(&velocities->y[i]);

    __m256 added_vel_x = _mm256_mul_ps(vel_x, dts);
    __m256 added_vel_y = _mm256_mul_ps(vel_y, dts);

    __m256 pos_x = _mm256_add_ps(_mm256_loadu_ps(&positions->x[i]), added_vel_x);
    __m256 pos_y = _mm256_add_ps(_mm256_loadu_ps(&positions->y[i]), added_vel_y);

    _mm256_storeu_ps(&positions->x[i], pos_x);
    _mm256_storeu_ps(&positions->y[i], pos_y);
  }
}

void simulate(void) {
  calculate_accelerations();
  calculate_velocities();
  calculate_positions();
}

void draw_bodies(void) {
  for (size_t i = 0; i < BODIES; ++i) {
    DrawCircle(positions->x[i], positions->y[i], RADIUS, RED);
  }
}

int main(void) {
  init();
  InitWindow(WIDTH, HEIGHT, "N-Body");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    simulate();
    
    BeginDrawing();
    ClearBackground(BLACK);

    draw_bodies();
    EndDrawing();
  }
  
  deinit();
  return 0;
}
