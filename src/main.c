#include <raylib.h>
#include <immintrin.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

struct Vec2 {
  float *x;
  float *y;
};

static const int32_t WIDTH = 1280;
static const int32_t HEIGHT = 1280;
static const size_t BODIES = 5000; // multiples of 8
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
  __m256 mass_gs = _mm256_set1_ps(GRAVITY_CONST * MASS);
  __m256 zeroes = _mm256_setzero_ps();
  __m256 two_radiuses = _mm256_set1_ps(2 * RADIUS);

  for (size_t i = 0; i < BODIES; i += 8) {
    __m256 accel_x = zeroes;
    __m256 accel_y = zeroes;

    for (size_t j = 0; j < BODIES; ++j) {
      __m256 dx = _mm256_sub_ps(_mm256_loadu_ps(&positions->x[i]), _mm256_broadcast_ss(&positions->x[j]));
      __m256 dy = _mm256_sub_ps(_mm256_loadu_ps(&positions->y[i]), _mm256_broadcast_ss(&positions->y[j]));

      __m256 dist = _mm256_max_ps(_mm256_fmadd_ps(dx, dx, _mm256_mul_ps(dy, dy)), two_radiuses);

      dist = _mm256_rsqrt_ps(dist);

      __m256 force = _mm256_mul_ps(mass_gs, _mm256_mul_ps(dist, _mm256_mul_ps(dist, dist)));

      accel_x = _mm256_fnmadd_ps(force, dx, accel_x);
      accel_y = _mm256_fnmadd_ps(force, dy, accel_y);
    }

    _mm256_storeu_ps(&accelerations->x[i], accel_x);
    _mm256_storeu_ps(&accelerations->y[i], accel_y);
  }
}

void calculate_velocities(void) {
  for (size_t i = 0; i < BODIES; i += 8) {
    float dt = GetFrameTime();
    __m256 dts = _mm256_broadcast_ss(&dt);
    __m256 accel_x = _mm256_loadu_ps(&accelerations->x[i]);
    __m256 accel_y = _mm256_loadu_ps(&accelerations->y[i]);

    __m256 old_vel_x = _mm256_loadu_ps(&velocities->x[i]);
    __m256 old_vel_y = _mm256_loadu_ps(&velocities->y[i]);

    __m256 vel_x = _mm256_fmadd_ps(accel_x, dts, old_vel_x);
    __m256 vel_y = _mm256_fmadd_ps(accel_y, dts, old_vel_y);

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

    __m256 old_pos_x = _mm256_loadu_ps(&positions->x[i]);
    __m256 old_pos_y = _mm256_loadu_ps(&positions->y[i]);

    __m256 pos_x = _mm256_fmadd_ps(vel_x, dts, old_pos_x);
    __m256 pos_y = _mm256_fmadd_ps(vel_y, dts, old_pos_y);

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
  // SetTargetFPS(60);

  clock_t time;
  while (!WindowShouldClose()) {
    time = clock();
    simulate();
    time = clock() - time;
    printf("Took %f ms to calculate sim\n", (float)time * 1000 / CLOCKS_PER_SEC);
    
    BeginDrawing();
    ClearBackground(BLACK);

    draw_bodies();
    EndDrawing();
  }

  CloseWindow();
  
  deinit();
  return 0;
}
