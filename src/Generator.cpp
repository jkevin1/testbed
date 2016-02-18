#include "LEAN.hpp"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  if (argc != 3 && argc != 4) {
    printf("Usage: lean <infile> <outfile> {scale=1}\n");
    return -1;
  }
  float scale = 1.0f;
  if (argc == 4) scale = atof(argv[3]);
  printf("Generating LEAN map from %s with scale %f\n", argv[1], scale);
  LEANMap lean = LEANMap::generate(argv[1], scale);
  printf("Writing to %s\n", argv[2]);
  lean.write(argv[2]);
}
