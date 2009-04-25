#include "raytracer.h"
#include "material.h"
#include "vectors.h"
#include "argparser.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "sphere.h"
#include "rdtsc.h"

#include "math.h"
#include <iostream>
#include <fstream>

unsigned long long RayTracer::TraceRays() {
    unsigned long long start_time, end_time, total_time;
    //cout << "Node " << processor_number << " number of pixels: " << num_pixels << endl;
    //cout << "Node " << processor_number << " start pixel: " << start_pixel << endl;
    start_time = rdtsc();
    while(DrawPixel()) ;
    end_time = rdtsc();
    total_time = end_time - start_time;
    ofstream output;
    char* filename = (char*)malloc(16*sizeof(char));
    char* buffer = (char*)malloc(num_pixels * 3 * 4 * sizeof(char));
    sprintf(filename, "out%04d.ppm", num_processors-processor_number);
    output.open(filename);
    if (processor_number == num_processors-1) {
        output << "P3" << endl << args->width << " " << args->height << endl << "255" << endl;
    }
    cout << "Start: " << (args->width*args->height-start_pixel-num_pixels)*3 << "\nEnd: " << (args->width*args->height-start_pixel)*3 << endl;
    for (int i = (args->width*args->height-start_pixel-num_pixels)*3; i < (args->width*args->height-start_pixel)*3; i++) {
        sprintf(buffer + (i-((args->width*args->height)-start_pixel-num_pixels)*3)*4, "%03d\n", int(image[i]));
    }
    output << buffer;
    output.close();
    free(filename);
    free(buffer);
    return total_time;
}

Vec3f RayTracer::SetupRay(double i, double j) {
  // compute and set the pixel color
  int max_d = max2(args->width,args->height);
  double x = (i+0.5-args->width/2.0)/double(max_d)+0.5;
  double y = (j+0.5-args->height/2.0)/double(max_d)+0.5;
  Vec2f point(x,y);
  Ray r = camera->generateRay(Vec2f(x,y));
  Hit hit;
  Vec3f color = TraceRay(r,hit);
  return color;
}

int RayTracer::DrawPixel() {
  if (raytracing_x > args->width) {
    raytracing_x = raytracing_skip/2;
    raytracing_y += raytracing_skip;
  }
  int pos = int(raytracing_y-0.5)*args->width + int(raytracing_x-0.5);
  if (pos - start_pixel > num_pixels) {
    return 0;
  }
  if (raytracing_y > args->height) {
    num_pixels = pos - start_pixel;
    return 0;
  }

  // compute the color and position of intersection
  Vec3f color = SetupRay(raytracing_x, raytracing_y);
  int x = int(raytracing_x - 0.5);
  int y = int(raytracing_y - 0.5);
  image[3*x + 3*args->width*(args->height-y-1) + 0] = color.x()*255;
  image[3*x + 3*args->width*(args->height-y-1) + 1] = color.y()*255;
  image[3*x + 3*args->width*(args->height-y-1) + 2] = color.z()*255;
  raytracing_x += raytracing_skip;
  return 1;
}

// casts a single ray through the scene geometry and finds the closest hit
bool RayTracer::CastRay(Ray &ray, Hit &h, bool use_sphere_patches) const {
  bool answer = false;

  // intersect each of the quads
  for (int i = 0; i < mesh->numQuadFaces(); i++) {
    Face *f = mesh->getFace(i);
    if (f->intersect(ray,h,args->intersect_backfacing)) answer = true;
  }

  // intersect each of the spheres (either the patches, or the original spheres)
  if (use_sphere_patches) {
    for (int i = mesh->numQuadFaces(); i < mesh->numFaces(); i++) {
      Face *f = mesh->getFace(i);
      if (f->intersect(ray,h,args->intersect_backfacing)) answer = true;
    }
  } else {
    const vector<Sphere*> &spheres = mesh->getSpheres();
    for (unsigned int i = 0; i < spheres.size(); i++) {
      if (spheres[i]->intersect(ray,h)) answer = true;
    }
  }
  return answer;
}



// does the recursive (shadow rays & recursive/glossy rays) work
Vec3f RayTracer::TraceRay(Ray &ray, Hit &hit, int bounce_count) const {

    hit = Hit();
    bool intersect = CastRay(ray,hit,false);
    
    Vec3f answer = args->background_color_linear;
    
    if (intersect == true) {
        Material *m = hit.getMaterial();
        assert (m != NULL);
        
        // rays coming from the light source are set to white, don't bother to ray trace further.
        if (m->getEmittedColor().Length() > 0.001) {
            answer = Vec3f(1,1,1);
        } 
        
        else {
        
            Vec3f normal = hit.getNormal();
            Vec3f point = ray.pointAtParameter(hit.getT());
            
            // ----------------------------------------------
            // ambient light
            answer = args->ambient_light_linear * 
            m->getDiffuseColor(hit.get_s(),hit.get_t());
            
            // ----------------------------------------------
            // add contributions from each light that is not in shadow
            int num_lights = mesh->getLights().size();
            for (int i = 0; i < num_lights; i++) {
                
                Face *f = mesh->getLights()[i];
                Vec3f pointOnLight = f->computeCentroid();
                Vec3f dirToLight = pointOnLight - point;
                dirToLight.Normalize();
                int numhits = 0;
                
                for (int j = 0; j < args->num_shadow_samples; j++) {
                    Vertex* p1 = (*f)[0];
                    Vertex* p2 = (*f)[1];
                    Vertex* p3 = (*f)[2];
                    Vertex* p4 = (*f)[3];
                    
                    /*if (args->num_shadow_samples > 8) {
                        Vec3f cd1 = p1->get()-point;
                        Vec3f cd2 = p2->get()-point;
                        Vec3f cd3 = p3->get()-point;
                        Vec3f cd4 = p4->get()-point;
                    }*/
                    
                    int squares = (int)sqrt(args->num_shadow_samples);
                    Vec3f step = (p1->get()-p3->get()) * (1.0/squares);
                    Vec3f dx = step.x()==0 ? Vec3f(0,fabs(step.y()),0) : Vec3f(fabs(step.x()),0,0);
                    Vec3f dy = step.x()==0 ? Vec3f(0,0,fabs(step.z())) : step.y()==0 ? Vec3f(0,0,fabs(step.z())) : Vec3f(0,fabs(step.y()),0);
                    
                    Vec3f topLeft = Vec3f(min(min(p1->x(), p2->x()), min(p3->x(), p4->x())),
                                          min(min(p1->y(), p2->y()), min(p3->y(), p4->y())),
                                          min(min(p1->z(), p2->z()), min(p3->z(), p4->z())));
                    Vec3f shadowPointOnLight;
                    if (args->num_shadow_samples > 1) {
                        shadowPointOnLight = topLeft + dx*(j%squares) + ((rand()%10000)/10000.0)*dx + dy*((j/squares)%squares) + ((rand()%10000)/10000.0)*dy;
                    } else {
                        shadowPointOnLight = pointOnLight;
                    }
                    Vec3f shadowDirToLight = shadowPointOnLight - point;
                    shadowDirToLight.Normalize();
                    Ray shadowRay(point, shadowDirToLight);
                    Hit shadowHit;
                    
                    CastRay(shadowRay, shadowHit, false);
                    if (shadowHit.getMaterial()->getEmittedColor().Length() > 0.001) numhits++;
                }
                
                if (normal.Dot3(dirToLight) > 0) {
                    Vec3f lightColor = 0.2 * f->getMaterial()->getEmittedColor() * f->getArea();
                    answer += (m->Shade(ray,hit,dirToLight,lightColor,args)) * (args->num_shadow_samples ? ((float)numhits/args->num_shadow_samples) : 1);
                }
                
            }
                
            // ----------------------------------------------
            // add contribution from reflection, if the surface is shiny
            Vec3f reflectiveColor = m->getReflectiveColor();
            double roughness = m->getRoughness();
            
            if (bounce_count < args->num_bounces && reflectiveColor.Length() > 0.001) {
                Vec3f refdir = ray.getDirection() - 2*ray.getDirection().Dot3(hit.getNormal())*hit.getNormal();
                Ray refray(point, refdir);
                Hit refhit;
                Vec3f refcolor(0,0,0);
                if (args->num_glossy_samples && roughness) {
                    for (int i = 0; i < args->num_glossy_samples; i++) {
                        float theta = (rand()%628318)/100000.0;                 //random angle between 0 and 2pi
                        float phi = roughness*(rand()%157079)/100000.0;  //random angle between 0 and (pi/2)*roughness
                        Vec3f newdir(cos(theta)*sin(phi), sin(theta)*sin(phi), cos(phi));
                        Vec3f perturb = Vec3f(0,0,1) - newdir;
                        Vec3f glosdir = refdir + perturb;
                        Ray glosray(point, glosdir);
                        Hit gloshit;
                        refcolor += TraceRay(glosray, gloshit, bounce_count+1);
                    }
                    refcolor *= (1.0/args->num_glossy_samples);
                } else {
                    refcolor = TraceRay(refray, refhit, bounce_count+1);
                }
                answer += refcolor*reflectiveColor;
            }
        }
    }

    return answer;
}
