// implementation code for Polygon object class

// include this class include file FIRST to ensure that it has
// everything it needs for internal self-consistency
#include "Polygon.hpp"

// other classes used directly in the implementation
#include "World.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

void
Polygon::addVertex(const Vec3 &v, const Vec3 &n)
{
    d_vertex.push_back(PolyVert(v,n));
}

void
Polygon::closePolygon()
{
    // compute normal from first two edges
    VertexList::iterator v2 = d_vertex.begin(), v0 = v2++, v1 = v2++;
    Vec3 e1 = v1->v - v0->v, e2 = v2->v - v1->v;
    d_normal = normalize(e1 ^ e2);

    // tangent and bitangent (2nd tangent perpendicular to 1st)
    // use edge between first and last vertex
    // avoids having to test that edge, and makes loops easier
    d_tangent = normalize(d_vertex.front().v - d_vertex.back().v);
    d_bitangent = d_normal ^ d_tangent;

    // for each vertex, precompute dot product with tangent and bitangent
    for(VertexList::iterator v = d_vertex.begin(); v != d_vertex.end(); ++v) {
        if (!d_useVertexNormals) // replace normal for 'p'-type
            v->n = d_normal;
        v->v_t = dot(v->v, d_tangent);
        v->v_b = dot(v->v, d_bitangent);
    }

    // precompute dot product of first vertex with normal
    d_v0_n = dot(d_vertex.front().v, d_normal);
}

const Intersection
Polygon::intersect(const Ray &ray) const 
{
    // compute intersection point with plane
    float t = (d_v0_n - dot(d_normal, ray.start)) /
        dot(d_normal, ray.direction);

    if (t < ray.near || t > ray.far)
        return Intersection();  // not in ray bounds: no intersection

    Vec3 p = ray.start + ray.direction * t;

    // dot product of intersection with polygon tangent and bitangent
    float p_t = dot(p, d_tangent), p_b = dot(p, d_bitangent);

    // check if intersection is inside or outside
    // trace ray from p along a tangent vector and count even/odd intersections
    bool inside = false;
    VertexList::const_iterator v1 = d_vertex.begin(), v0 = v1++;
    for(; v1 != d_vertex.end(); v0 = v1, ++v1) {
        // does edge straddle test ray where q dot bitangent = p dot bitangent?
        float b0 = v1->v_b - p_b, b1 = p_b - v0->v_b;
        if ((b0 > 0) ^ (b1 < 0)) {
            // outbound on test ray?
            float q_t = (b0 * v0->v_t + b1 * v1->v_t)/(v1->v_b - v0->v_b);
            if (q_t > p_t)
                inside = !inside;
        }
    }

    if (inside) return Intersection(this,t);

    return Intersection();
}

const Vec3
Polygon::appearance(const World &w, const Ray &r, float t) const {
    Vec3 p = r.start + r.direction * t;

    if (! d_useVertexNormals)
        // per-polygon normal is easy and fast
        return d_appearance.eval(w, p, d_normal, r);
    else {
        // inefficiently re-test all edges to find vertex normals!
        VertexList::const_iterator v1 = d_vertex.begin(), v0 = v1++;

        // dot product of intersection with polygon tangent and bitangent
        float p_t = dot(p, d_tangent), p_b = dot(p, d_bitangent);

        float s0 = -INFINITY, s1 = INFINITY; // closest front/back intersection 
        float b00, b01, b10, b11;       // fraction of the way along each edge
        Vec3 n00, n01, n10, n11;        // normals at hit vertices

        // re-test each edge
        for(; v1 != d_vertex.end(); v0 = v1, ++v1) {
            // does edge straddle test ray?
            float b0 = v1->v_b - p_b, b1 = p_b - v0->v_b;
            if ((b0 > 0) ^ (b1 < 0)) {
                // convert to barycentric coordinates along edge
                b0 /= (v1->v_b - v0->v_b);
                b1 /= (v1->v_b - v0->v_b);
                float s = (b0 * v0->v_t + b1 * v1->v_t) - p_t;

                // closest behind test ray?
                if (s < 0 && s > s0) {
                    s0 = s; 
                    b00 = b0; b01 = b1;
                    n00 = v0->n; n01 = v1->n;
                }
                // closest outbound on test ray?
                if (s > 0 && s < s1) {
                    s1 = s; 
                    b10 = b0; b11 = b1;
                    n10 = v0->n; n11 = v1->n;
                }
            }
        }

        // interpolate between normals along test ray
        Vec3 n0 = b00*n00 + b01*n01;
        Vec3 n1 = b10*n10 + b11*n11;
        Vec3 n = normalize(s1*n0 - s0*n1);

        return d_appearance.eval(w, p, n, r);
    }
}
