#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#include "argparser.h"
#include "mesh.h"
#include "raytracer.h"
#include "camera.h"

// =========================================
// =========================================

int main(int argc, char *argv[]) {
    srand((unsigned)time(0));

      int npes, myrank;
      unsigned long long my_time, min_time, max_time, avg_time;
      double my_usecs, min_usecs, max_usecs, avg_usecs;
//    int npes = 1;
//    int myrank = 0;
  
    MPI::Init(argc, argv);
    npes = MPI::COMM_WORLD.Get_size();
    myrank = MPI::COMM_WORLD.Get_rank();

    ArgParser *args = new ArgParser(argc, argv);
    
    Mesh *mesh = new Mesh();
    mesh->Load(args->input_file,args);
    Vec3f up = Vec3f(0,1,0);
    Camera *camera = new PerspectiveCamera(args->camera_position, args->poi, up, 20*3.14159/180.0);
    RayTracer *raytracer = new RayTracer(mesh, args, camera, npes, myrank);
    
    my_time = raytracer->TraceRays();
    MPI::COMM_WORLD.Barrier();

    my_usecs = my_time / 700.0; // BlueGene/L, 700 MHz processors
    MPI::COMM_WORLD.Allreduce(&my_usecs, &min_usecs, 1, MPI_DOUBLE, MPI_MIN);
    MPI::COMM_WORLD.Allreduce(&my_usecs, &max_usecs, 1, MPI_DOUBLE, MPI_MAX);
    MPI::COMM_WORLD.Allreduce(&my_usecs, &avg_usecs, 1, MPI_DOUBLE, MPI_SUM);

    avg_usecs /= npes;

    if(myrank == 0) {
       printf("Final Results:\nMin: %f\nMax: %f\nAvg: %f\n", min_usecs, max_usecs, avg_usecs);
    }
    
    delete args;
    MPI::Finalize();
    return 0;
}

// =========================================
// =========================================
