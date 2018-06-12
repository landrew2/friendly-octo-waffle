// Rays
#ifndef RAY_HPP
#define RAY_HPP

// other classes we use DIRECTLY in our interface
#include "Vec3.hpp"

// a ray consists of a starting point and direction
// ray is defined as start + t*direction, for t>=0
class Ray {
public: // public data
    Vec3 start;         // ray start point
    Vec3 direction;     // ray direction
    float near;         // closest t to ray origin to count as intersection
    float far;          // farthest t to count as intersection
    int bounces;        // number of bounces allowed for ray
    float influence;    // maximum contribution of this ray to the final image

public: // constructors
    Ray(const Vec3 &_start, const Vec3 &_direction, 
        float _near=1e-4, float _far=INFINITY,
        int _bounces=0, float _influence=0) 
    {
        start = _start;
        direction = _direction;
        bounces = _bounces;
        influence = _influence;
        near = _near;
        far = _far;
    }
};


#endif
