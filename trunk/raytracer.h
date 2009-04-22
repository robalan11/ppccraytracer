#ifndef _RAY_TRACER_
#define _RAY_TRACER_

#include <assert.h>
#include "argparser.h"
#include "ray.h"
#include "hit.h"
#include "camera.h"

#include <vector>
#include <math.h>

class Mesh;
class ArgParser;

// ====================================================================
// ====================================================================

class RayTracer {

public:

  // CONSTRUCTOR & DESTRUCTOR
  RayTracer(Mesh *m, ArgParser *a, Camera *c, int npes, int myrank) {
    mesh = m;
    args = a;
    camera = c;
    processor_number = myrank;
    num_pixels = int(ceil((args->width * args->height) / float(npes)));
    start_pixel = num_pixels * processor_number;
    raytracing_x = 0.5 + start_pixel % args->width;
    raytracing_y = 0.5 + start_pixel / args->width;
    raytracing_skip = 1;
    image = (double*)malloc(args->width * args->height * 3 * sizeof(double));
  }  
  ~RayTracer() {}
  
  unsigned long long TraceRays();
  Vec3f SetupRay(double i, double j);
  int DrawPixel();
  
  // casts a single ray through the scene geometry and finds the closest hit
  bool CastRay(Ray &ray, Hit &h, bool use_sphere_patches) const;

  // does the recursive work
  Vec3f TraceRay(Ray &ray, Hit &hit, int bounce_count = 0) const;

private:

  RayTracer() { assert(0); } // don't use this

  // REPRESENTATION
  Mesh *mesh;
  ArgParser *args;
  Camera* camera;
  double raytracing_x;
  double raytracing_y;
  int raytracing_skip;
  int start_pixel;
  int processor_number;
  int num_pixels;
  double* image;
};

// ====================================================================
// ====================================================================

#endif
