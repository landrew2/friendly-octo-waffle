// Everything we know about the world
#ifndef World_hpp
#define World_hpp

// other classes we use DIRECTLY in our interface
#include "Vec3.hpp"
#include "ObjectList.hpp"
#include <list>
#include <stdio.h>

struct Light {
    Vec3 pos;                   // light position
    Vec3 col;                   // light color
    Light() : pos(Vec3(0,0,0)), col(Vec3(1,1,1)) {}
};
typedef std::list<Light> LightList;

class World {
public: // public data
    enum Effects {                          // one bit for each shading effect
        DIFFUSE        = 0x001, 
        SPECULAR       = 0x002, 
        SHADOW         = 0x004, 
        REFLECT        = 0x008, 
        REFRACT        = 0x010,
        DEPTH_OF_FIELD = 0x020,
        ANTIALIAS      = 0x040,
        POLYGONS       = 0x080,
        SPHERES        = 0x100,
        CONES          = 0x200
    };
    static unsigned int effects;

    // image size
    int width, height;

    // background color
    Vec3 background;

    // near clipping plane distance
    float hither;

    // view origin and basis parameters
    Vec3 eye, w, u, v;
    float dist, left, right, bottom, top;


    // list of objects in the scene
    ObjectList objects;

    // list of lights
    LightList lights;

public:                                                     
    // read world data from a file
    World(FILE *f);
};

#endif
