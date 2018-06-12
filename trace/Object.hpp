// virtual class for any object
#ifndef OBJECT_HPP
#define OBJECT_HPP

// other classes we use DIRECTLY in our interface
#include "Appearance.hpp"
#include "Intersection.hpp"
#include "Vec3.hpp"

// classes we only use by pointer or reference
class World;
class Ray;

class Object {
protected: // data visible to children
    Appearance d_appearance;        // this object's appearance parameters

public: // constructor & destructor
    Object();
    Object(const Appearance &_appearance);
    virtual ~Object();


public: // computational members
    // return t for closest intersection with ray
    virtual const Intersection intersect(const Ray &ray) const = 0;

    // return color for intersection at t along ray r
    virtual const Vec3 appearance(const World &w, 
            const Ray &r, float t) const = 0;
};

#endif
