# Hi guys #

This is (a draft of) my part of the paper; see you all at 11 in the Rathskellar.


# Ray Tracer Implementation #

The ray tracer implementation is very straightforward.  The program first applies a transformation matrix to the scene to project it onto the projection plane with the specified resolution.  The program then casts a ray through the center of every nth pixel, where n is the number of processors being used.  Each ray checks against each object in the scene (sphere or quad) to detect whether or not it intersects it.  It then determines which object is the closest and calculates the diffuse color of the surface.

If the num\_shadow\_samples argument has been provided in the command line, the program next casts a number of shadow rays towards the light.  The light is discretized into a number of square sections, and the shadow rays are cast towards random points in these sections.  The discretization helps avoid unnecessary noise in the shadows, while the jittering prevents banding.  This shadow value is then multiplied into the diffuse color.

If the surface is reflective, a reflection ray is cast out reflected across the normal.  This reflection ray is traced just as the initial ray was, then the returned value is multiplied by the diffuse color.  If necessary, the reflection ray will cast further reflection rays recursively until a purely diffuse surface is hit or the objects in the scene are missed entirely.  If the num\_glossy\_samples argument has been provided, these reflection rays are scattered in a cone around the reflection based on the roughness of the surface.  The scene used in this experiment did not have any glossy surfaces, but this functionality is implemented.

Once the value of the pixel is calculated (diffuse color times shadow value time reflected ray if applicable), the color is put into an array.  Since each processor only calculates 1/nth of the pixels in the image, these arrays must be compiled by some MPI code-1-.  The compiled array is then converted from floats to chars, and the chars are printed into a string, which is then output to a portable pixmap file.  The portable pixmap file type is a rather inefficient, but human-readable image file type which can be easily diffed against other results to validate parallel results.


-1-:   See MPI implementation section for more information