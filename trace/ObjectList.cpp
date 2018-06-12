// implementation code for ObjectList class
// list of objects in scene

// include this class include file FIRST to ensure that it has
// everything it needs for internal self-consistency
#include "ObjectList.hpp"
#include "Object.hpp"

// delete list and objects it contains
ObjectList::~ObjectList() {
    for(t_List::iterator i=d_list.begin(); i != d_list.end(); ++i) {
        delete *i;
    }
}

// trace ray r through all objects, returning first intersection
const Intersection
ObjectList::trace(Ray r) const
{
    Intersection closest;       // no object, t = infinity
    for(t_List::const_iterator i=d_list.begin(); i != d_list.end(); ++i) {
        Intersection current = (*i)->intersect(r);
        if (current < closest) {
            closest = current;
            r.far = current.t;
        }
    }
    return closest;
}

// trace ray r through all objects, returning true if there is any
// intersection between r.near and r.far
const bool
ObjectList::probe(Ray r) const
{
    for(t_List::const_iterator i=d_list.begin(); i != d_list.end(); ++i) {
        if ((*i)->intersect(r).t < r.far)
            return true;
    }
    return false;
}
