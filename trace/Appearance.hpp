// all info on an object's appearance
#ifndef APPEARANCE_HPP
#define APPEARANCE_HPP

// other classes we use DIRECTLY
#include "Vec3.hpp"

// classes we only use by pointer or reference
class World;
class Ray;

class Appearance {
public: // public data
    Vec3 color;     // diffuse color
    float kd;       // diffuse coefficient
    float ks, e;    // specular coefficient & exponent
    float kt, ir;   // transmission coefficient & index of refraction

public: // constructors
    Appearance() : color(Vec3(1,1,1)), kd(0), ks(0), e(0), kt(0), ir(1) {}
    // also allow default copy constructor and assignment operator

public: // computational members
    // return color for appearance of this surface for point at p, with
    // normal n and view ray r
    const Vec3 eval(const World &world, const Vec3 &p, 
            const Vec3 &n, const Ray &r) const;
};


#endif
