// sphere objects
#ifndef SPHERE_HPP
#define SPHERE_HPP

// other classes we use DIRECTLY in the interface
#include "Object.hpp"
#include "Vec3.hpp"

// classes we only use by pointer or reference
class Appearance;
class World;
class Ray;

// sphere objects
class Sphere : public Object {
private: // private data
    Vec3 d_center;
    float d_radius;

public: // constructors
    Sphere(const Appearance &_appearance, const Vec3 &_center, float _radius)
        : Object(_appearance)
    {
        d_center = _center;
        d_radius = _radius;
    }

public: // object functions
    const Intersection intersect(const Ray &ray) const;
    const Vec3 appearance(const World &w, const Ray &ray, float t) const;
};

#endif
