Parallel Raytracer

Authors:
Allan Pendergrast
Brandon Haines
Chris Ensor
Derek Pitman


Compiling Instructions:
type make bgfen to make on the blue gene
type make unix to make on a unix machine


Running Instructions:
./render.exe -size 400 300 -input reflective_spheres.obj -num_shadow_samples 1
-num_bounces 3 -background_color 0.5 0.6 0.9 -camera_position 5 3 10 -poi 0.5 0 -0.5

A different number of shadow samples, bounces, size, or background color can be
used for different visual appearance.


Notes on the code:

The files that were most modified from the original OpenGL raytracer were
main.cpp and raytracer.cpp.  Within the raytracer file, TraceRays, SetupRay,
and DrawPixel are the most highly modified functions for parallelization.