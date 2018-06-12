// polygon objects
#ifndef POLYGON_HPP
#define POLYGON_HPP

// other classes we use DIRECTLY in our interface
#include "Object.hpp"
#include "Vec3.hpp"

// system includes necessary for the interface
#include <vector>

// classes we only use by pointer or reference
class Appearance;
class World;
class Ray;

class Polygon : public Object {
private: // private data
    struct PolyVert {
        Vec3 v;                     // vertex
        Vec3 n;                     // vertex normal
        // the rest are derived values used in intersection testing
        float v_t, v_b;             // v dot tangent and v dot bitangent
        PolyVert(const Vec3 &_v, const Vec3 &_n) { v = _v; n = _n; }
    };
    typedef std::vector<PolyVert> VertexList;

    VertexList d_vertex;            // list of vertices
    bool d_useVertexNormals;        // use vertex normals? (= 'pp' type)
    Vec3 d_normal;                  // face normal
    Vec3 d_tangent;                 // face tangent
    Vec3 d_bitangent;               // second face tangent

    // derived values for intersection testing
    float d_v0_n;                   // v0 dot d_normal

public: // constructors
    Polygon(int verts, const Appearance &_appearance, bool _useVertexNormals)
        : Object(_appearance) 
    { 
        d_vertex.reserve(verts);
        d_useVertexNormals = _useVertexNormals; 
    }

public: // manipulators
    // add a new vertex to the polygon
    // given vertex and per-vertex normal
    void addVertex(const Vec3 &v, const Vec3 &n);

    // close the polygon after the last vertex
    void closePolygon();

public: // object functions
    const Intersection intersect(const Ray &ray) const;
    const Vec3 appearance(const World &w, const Ray &r, float t) const;
};

#endif
