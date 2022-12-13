#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Filter.h"

using namespace std;

#include "rdtsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    short ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

class Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
      int value;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {

	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  } else {
    cerr << "Bad input in readFilter:" << filename << endl;
    exit(-1);
  }
}


double
applyFilter(class Filter *filter, cs1300bmp *input, cs1300bmp *output)
{

  long long cycStart, cycStop;

  cycStart = rdtscll();

  output -> width = input -> width;
  output -> height = input -> height;

   char f[3][3];
    for(int a = 0; a < 3; a++){
        for(int b = 0; b < 3; b++){
            f[a][b] = filter-> get(a, b);
        }
    }

    short w = input -> width;
    short h = input -> height;

    char divisor = filter -> getDivisor();

  for(int plane = 0; plane < 3; plane++) {
    for(int row = 1; row < h - 1 ; row += 1) {
      for(int col = 1; col < w - 1; col += 1) {
        short val_sum = 0;

          val_sum += input -> color[plane][row + - 1][col - 1] * f[0][0];
          val_sum += input -> color[plane][row + 0][col - 1] * f[1][0];
          val_sum += input -> color[plane][row + 1][col - 1] * f[2][0];
          val_sum += input -> color[plane][row + - 1][col] * f[0][1];
          val_sum += input -> color[plane][row + 0][col] * f[1][1];
          val_sum += input -> color[plane][row + 1][col] * f[2][1];
          val_sum += input -> color[plane][row + - 1][col + 1] * f[0][2];
          val_sum += input -> color[plane][row + 0][col + 1] * f[1][2];
          val_sum += input -> color[plane][row + 1][col + 1] * f[2][2];

          short value = val_sum / divisor;

          if(value < 0){value = 0;}
          if(value > 255) {value = 255;}
          output -> color[plane][row][col] = value;

      }
    }
  }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}