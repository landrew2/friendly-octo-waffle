// implementation code for Cone object class

// include this class include file FIRST to ensure that it has
// everything it needs for internal self-consistency
#include "Cone.hpp"

// other classes used directly in the implementation
#include "World.hpp"
#include "Ray.hpp"

Cone::Cone(const Appearance &appearance, 
           const Vec3 &base, float base_radius,
           const Vec3 &apex, float apex_radius)
    : Object(appearance), 
    d_base(base), d_rBase(base_radius)
{
    // compute some derived values that don't change once cone is defined
    d_rDiff = apex_radius-base_radius;
    d_axis = apex-base;
    d_scaledAxis = d_axis/dot(d_axis,d_axis);
}

// cone-ray intersection
const Intersection
Cone::intersect(const Ray &r) const
{
    // equation for vector from base to ray: V = E + t D
    Vec3 E = r.start - d_base;
    Vec3 D = r.direction;

    // the cone is defined by
    //   B = base; b = radius at base
    //   A = apex; a = radius at apex
    //   C = center line = A-B; c = difference in radii = a-b
    //   Cs = scaled C = C/C.C
    float b = d_rBase, c = d_rDiff;
    Vec3 C = d_axis, Cs = d_scaledAxis;

    // for any vector V from B
    //   f = fraction between B and A = V.C/C.C = V.Cs
    //     = E.Cs + t D.Cs
    float E_Cs = dot(E, Cs), D_Cs = dot(D, Cs);

    //   Vp = component of V perpendicular to C = V - f C
    //     = E + t D - E.Cs C - t D.Cs C
    //     = (E - E.Cs C) + t (D - D.Cs C)
    //     = Ep + t Dp
    Vec3 Ep = E - E_Cs*C, Dp = D - D_Cs*C;

    //   u^2 = squared distance of V from C = Vp.Vp
    //     = Ep.Ep + 2 t Ep.Dp + t^2 Dp.Dp
    //
    //   r = interpolated radius from cone parameters = b + f c
    //     = b + E.Cs c + t D.Cs c
    //     = rb + t ra
    //   r^2 = rb^2 + 2 t rb ra + t^2 ra^2
    float rb = b + E_Cs*c, ra = D_Cs*c;

    // on cone if u^2 - r^2 == 0
    //   test = (Ep.Ep-rb^2) + 2 t (Ep.Dp - rb ra) + t^2 (Dp.Dp-ra^2)
    //     = qc + qb t + qa t^2
    float qa = dot(Dp, Dp) - ra*ra;
    float qb = 2*(dot(Ep, Dp) - rb*ra);
    float qc = dot(Ep, Ep) - rb*rb;

    // quadric term=0 if parallel to one side
    if (qa == 0) {
        float t = qc/qb;        // => linear solution for other side
        if (t < r.near || t > r.far) // outside ray extent
            return Intersection();
        float u = E_Cs + t*D_Cs;
        if (u<0 || u>=1) return Intersection(); // outside cone limits
        return Intersection(this,t);
    }

    // discriminant of quadratic equation
    float discriminant = qb*qb - 4*qa*qc;
    if (discriminant < 0)       // no intersection with extended cone
        return Intersection();

    float dsq = sqrtf(discriminant);

    // check if intersections are front of ray start
    // or between base and apex
    float t1 = (-qb - dsq) / (2*qa);
    float u1 = E_Cs + t1*D_Cs;
    if (t1 < r.near) t1 = INFINITY;
    if (u1<0 || u1 >= 1) t1 = INFINITY;

    float t2 = (-qb + dsq) / (2*qa);
    float u2 = E_Cs + t2*D_Cs;
    if (t2 < r.near) t2 = INFINITY;
    if (u2<0 || u2 >= 1) t2 = INFINITY;

    if (t1<t2) return Intersection(this,t1);

    return Intersection(this,t2);
}


// appearance of sphere at position t on ray r
const Vec3
Cone::appearance(const World &w, const Ray &r, float t) const
{
    Vec3 p = r.start + r.direction * t; // intersection point

    // find normal (perhaps not the most efficient way)
    Vec3 V = p-d_base;          // vector from p to base
    Vec3 Vp = V - d_axis*dot(V,d_scaledAxis); // component perpendicular to axis
    Vec3 E = d_axis+d_rDiff*normalize(Vp); // vector parallel to edge

    // normal = component of V perpendicular to edge
    Vec3 n = V - E*(dot(V,E)/dot(E,E));

    return d_appearance.eval(w, p, normalize(n), r);
}
