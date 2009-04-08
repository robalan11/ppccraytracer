#include "sphere.h"
#include "material.h"
#include "argparser.h"
#include "utils.h"
#include "math.h"

// Included files for OpenGL Rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

// ====================================================================
// ====================================================================

bool Sphere::intersect(const Ray &r, Hit &h) const {
    
    Vec3f R_O = r.getOrigin()-center;
    Vec3f R_D = r.getDirection();
    float a = R_D.Dot3(R_D);
    float b = 2*R_D.Dot3(R_O);
    float c = R_O.Dot3(R_O) - radius*radius;
    float d = b*b-4*a*c;
    
    if (b*b-4*a*c < 0) {
        return false;
    }
    
    float t = min((-b + sqrt(d)) / (2*a), (-b - sqrt(d)) / (2*a));
    if (t < 0) { t = max((-b + sqrt(d)) / (2*a), (-b - sqrt(d)) / (2*a)); }
    //cout << t << endl;
    if (t < 0.005) {
        return false;
    }
    
    Vec3f p = R_O + t*R_D;
    
    if (t < h.getT()) {
        h.set(t, material, p*(1/radius));
    }
    return true;
} 

// ====================================================================
// ====================================================================
