#include <iostream>
#include <stdlib.h>
#include <climits>
#include <time.h>
#include <cstdio>

namespace {

typedef unsigned char data_t;
#define DATA_MAX 256

#ifndef NUM_REPETITIONS
#define NUM_REPETITIONS 1
#endif

class StopWatch {
public:
  void tic() {
    tic_ = clock();
  }

  // Returns time elapsed in ms.
  int toc(const char* msg) {
    clock_t t = clock();
    int elapsed = int((t - tic_) * 1000 / CLOCKS_PER_SEC);
    printf("%s\telapsed: %d ms.\n", msg, elapsed);
    tic_ = t;
    return elapsed;
  }

private:
  clock_t tic_;
};

template<typename T>
void print_matrix(const T* M, int w, int h) {
  int i = 0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++, i++)
      printf("%.1f ", float(M[i]));
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void print_usage(std::ostream& stream) {
  stream << "Usage: gradient  <width> <height>" << std::endl;
  stream << "  width, height: positive integers, and width * height <= INT_MAX." << std::endl; 
}

void init_random(data_t* M, int size) {
  for (int i = 0; i < size; i++)
    M[i] = rand() % DATA_MAX;
}

void minmax(const short* d, int n, short* min, short* max) {
  *max = -DATA_MAX;
  *min = DATA_MAX;
  for (int i = 0; i < n; i++) {
    *max = d[i] > *max ? d[i] : *max;
    *min = d[i] < *min ? d[i] : *min;
  }
}

// Computes dx and dy in separate loops.
// This seems to be faster.
int gradient2(data_t* M, int w, int h) {
  float ry, rx;
  data_t *My, *My0, *My1, *Mx0, *Mx1; // indices
  short *dx, *dy, *dxp, *dyp;
  short dx_max, dy_max, dx_min, dy_min;
  StopWatch sw;

  sw.tic();
  
  int n = w * h;
  dx = dxp = new short[n];
  dy = dyp = new short[n];

  My = M; // My is the center index.
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      if (x == 0)     { Mx0 = My;     Mx1 = My + 1; rx = 1; } 
      if (x == 1)     { Mx0 = My - 1; Mx1 = My + 1; rx = .5; }
      if (x == w - 1) {               Mx1 = My;     rx = 1; }
      *(dxp++) = (*Mx1 - *Mx0);
      Mx0++; Mx1++; My++;
    }
  }
  minmax(dx, n, &dx_min, &dx_max);

  My = M;
  for (int y = 0; y < h; y++) {
    if (y == 0)     { My0 = My;     My1 = My + w; ry = 1; }
    if (y == 1)     { My0 = My - w; My1 = My + w; ry = .5; }
    if (y == h - 1) {               My1 = My;     ry = 1; }
    for (int x = 0; x < w; x++) {
      *(dyp++) = (*My1 - *My0);
      My0++; My1++; My++;
    }
  }
  
  minmax(dy, n, &dy_min, &dy_max);

  printf("min dx = %.1f\n", dx_min * .5);
  printf("max dx = %.1f\n", dx_max * .5);
  printf("min dy = %.1f\n", dy_min * .5);
  printf("max dy = %.1f\n", dy_max * .5);

#ifdef DEBUG
  print_matrix(M, w, h);
  print_matrix(dx, w, h);
  print_matrix(dy, w, h);
#endif

  delete [] dx;
  delete [] dy;
  return sw.toc("Total");
}

// w * h should be <= INT_MAX
int compute(int w, int h) {
  int n = w * h;
  data_t* M = new data_t[n];
  init_random(M, n);
  int elapsed = gradient2(M, w, h);
  delete [] M;
  return elapsed;
}

bool is_valid_range(long int size) {
  return size > 0 && size <= INT_MAX;
}
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    print_usage(std::cerr);
    return 1;
  }

  long int width = strtol(argv[1], NULL, 10);
  long int height = strtol(argv[2], NULL, 10);
  
  if (!is_valid_range(width) || !is_valid_range(height) ||
      !is_valid_range(width * height)) {
    print_usage(std::cerr);
    return 1;
  }

  srand(time(NULL));

  int elapsed = 0;
  for (int i = 0; i < NUM_REPETITIONS; i++) {
    elapsed += ::compute(width, height);
  }

  if (NUM_REPETITIONS > 1)
    printf("Average elapsed: %d ms\n", elapsed / NUM_REPETITIONS);

  return 0;
}
