// implementation code for Appearance class

// include this class include file FIRST to ensure that it has
// everything it needs for internal self-consistency
#include "Appearance.hpp"

// other classes used directly in the implementation
#include "World.hpp"
#include "Ray.hpp"

// Color of this object
const Vec3
Appearance::eval(const World &world, const Vec3 &p, 
        const Vec3 &n, const Ray &r) const
{
    // base color
    Vec3 col = Vec3(0,0,0);

    // view ray
    Vec3 V = -normalize(r.direction);

    // diffuse and specular
    for (LightList::const_iterator li=world.lights.begin();
         li != world.lights.end(); ++li) {

        Vec3 L = li->pos - p;   // light vector

        // cast ray to see if it's in shadow
        if (! (World::effects & World::SHADOW) || 
            ! world.objects.probe(Ray(p,L,1e-4f,1.f))) {

            // normalized L and H
            L = normalize(L);
            Vec3 H = normalize(V+L);

            float diffuse = dot(n,L);
            if (diffuse > 0) {
                Vec3 dc = li->col * color;
                if (World::effects & World::DIFFUSE)
                    col = col + kd*diffuse*dc;

                if (ks > 0 && (World::effects & World::SPECULAR)) {
                    float specular = dot(n,H);
                    if (specular > 0)
                        col = col + ks*diffuse*pow(specular,e)*li->col;
                }
            }
        }
    }

    // reflected rays
    if ((World::effects & World::REFLECT) &&
         r.influence * ks > 1 && r.bounces > 0) {

        // reflect ray off surface
        Vec3 rv = r.direction - 2*dot(n, r.direction)*n;

        // new ray with one less bounce and influence reduced by kr
        Ray rr(p, rv, 1e-4f, INFINITY, r.bounces-1, r.influence*ks);
        Vec3 rc = world.objects.trace(rr).color(world,rr); // trace ray
        col = col + ks * rc;
    }

    // refracted rays
    if ((World::effects & World::REFRACT) &&
            r.influence * kt > 1 && r.bounces > 0) {

        // compute refracted ray
        float ci = dot(n,V);                // cosine of incident ray angle
        float tir = ci > 0 ? 1/ir : ir;     // ratio of air to object or object to air
        float ct2 = 1-(1-ci*ci)*tir*tir;    // cosine squared of refracted ray
        if (ct2 > 0) {                  // <=0 for total internal reflection
            // ray direction
            Vec3 td;
            if (ci>0)                   // into surface
                td = n*(ci*tir - sqrtf(ct2)) - V*tir;
            else                        // out of surface
                td = n*(ci*tir + sqrtf(ct2)) - V*tir;

            // new ray with one fewer bounce and influence reduced by kt
            Ray tr(p, td, 1e-4f, INFINITY, r.bounces-1, r.influence*kt);
            Vec3 tc = world.objects.trace(tr).color(world,tr); // trace ray
            col = col + kt * tc;
        }
    }

    return col;
}
