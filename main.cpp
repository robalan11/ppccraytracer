#include <stdio.h>
#include <stdlib.h>

#include "argparser.h"
#include "mesh.h"
#include "raytracer.h"
#include "camera.h"

// =========================================
// =========================================

int main(int argc, char *argv[]) {
    srand((unsigned)time(0));

    int npes = 1;
    int myrank = 0;
  
    ArgParser *args = new ArgParser(argc, argv);
    
    Mesh *mesh = new Mesh();
    mesh->Load(args->input_file,args);
    Vec3f up = Vec3f(0,1,0);
    Camera *camera = new PerspectiveCamera(args->camera_position, args->poi, up, 20*3.14159/180.0);
    RayTracer *raytracer = new RayTracer(mesh, args, camera, npes, myrank);
    
    raytracer->TraceRays();
    
    delete args;
    return 0;
}

// =========================================
// =========================================
