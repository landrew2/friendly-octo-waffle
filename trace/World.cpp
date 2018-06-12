// implementation code World object
// World holds everything we know about the world.
// Code here initializes it based on file input

// include this class include file FIRST to ensure that it has
// everything it needs for internal self-consistency
#include "World.hpp"

// local includes
#include "Polygon.hpp"
#include "Sphere.hpp"
#include "Cone.hpp"
#include "Appearance.hpp"

// system includes
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifdef _WIN32
#pragma warning( disable: 4996 )
#endif

// scoped global for what is enabled
unsigned int World::effects = ~0;

// report an error
static void err(int lineNum)
{
    fprintf(stderr, "NFF file error at line %d\n", lineNum);
    exit(1);
}

// read a line
static char *readLine(FILE *f, char line[1024], int &lineNumber)
{
    ++lineNumber;
    return fgets(line, 1024, f);
}

// read input file
World::World(FILE *f)
{
    char line[1024];                    // line of file
    int lineNumber = 0;                 // current line for error reporting
    Appearance app;                     // current object appearance

    while(readLine(f, line, lineNumber)) {
        switch(line[0]) {
            case ' ': case '\t':        // blank lines and comments
            case '\f': case '\r': case '\n':
            case '#': case '\0':
                break;

            case 'v':                   // view point
                {
                    // read view parameters
                    Vec3 vAt, vUp;
                    float angle;
                    readLine(f, line, lineNumber);
                    if (sscanf(line,"from %f %f %f", &eye[0], &eye[1], &eye[2]) != 3) 
                        err(lineNumber);

                    readLine(f, line, lineNumber);
                    if (sscanf(line,"at %f %f %f", &vAt[0], &vAt[1], &vAt[2]) != 3)
                        err(lineNumber);

                    readLine(f, line, lineNumber);
                    if (sscanf(line,"up %f %f %f", &vUp[0], &vUp[1], &vUp[2]) != 3) 
                        err(lineNumber);

                    readLine(f, line, lineNumber);
                    if (sscanf(line,"angle %f", &angle) != 1) 
                        err(lineNumber);

                    readLine(f, line, lineNumber);
                    if (sscanf(line,"hither %f", &hither) != 1) 
                        err(lineNumber);

                    readLine(f, line, lineNumber);
                    if (sscanf(line, "resolution %d %d", &width, &height) != 2) 
                        err(lineNumber);

                    // compute view basis
                    w = eye - vAt;
                    dist = length(w);
                    w = normalize(w);
                    u = normalize(vUp ^ w);
                    v = w ^ u;

                    // solve w/2d = tan(fov/2), where w=2 and fov must be in radians
                    float t = float(tan(angle * M_PI/360));
                    top = dist*t;
                    bottom = -top;
                    right = top * width / height;
                    left = -right;

                    break;
                }

            case 'b':                   // background
                {
                    if (sscanf(line,"b %f %f %f",
                                &background[0], &background[1], &background[2]) != 3) 
                        err(lineNumber);

                    break;
                }

            case 'l':                   // light
                {
                    Light light;
                    if (sscanf(line,"l %f %f %f %f %f %f",
                                &light.pos[0], &light.pos[1], &light.pos[2],
                                &light.col[0], &light.col[1], &light.col[2]) < 3)
                        err(lineNumber);
                    lights.push_back(light);

                    break;
                }

            case 'f':                   // fill/material properties
                {
                    if (sscanf(line,"f %f %f %f %f %f %f %f %f",
                                &app.color[0], &app.color[1], &app.color[2],
                                &app.kd, &app.ks, &app.e, &app.kt, &app.ir) != 8) 
                        err(lineNumber);

                    break;
                }

            case 'c':                   // cone or cylinder
                {
                    Vec3 base, apex;                      // location of base and apex
                    float rBase, rApex;                   // radius at base and apex

                    if (sscanf(line,"c %f %f %f %f %f %f %f %f",
                                &base[0], &base[1], &base[2], &rBase,
                                &apex[0], &apex[1], &apex[2], &rApex) != 8) 
                        err(lineNumber);

                    if (World::effects & World::CONES)
                        objects.addObject(new Cone(app, base, rBase, apex, rApex));

                    break;
                }

            case 's':                   // sphere
                {
                    Vec3 center;
                    float radius;
                    if (sscanf(line,"s %f %f %f %f",
                                &center[0], &center[1], &center[2], &radius) != 4) 
                        err(lineNumber);

                    if (World::effects & World::SPHERES)
                        objects.addObject(new Sphere(app, center, radius));

                    break;
                }

            case 'p':                   // p or pp polygon primitives
                {
                    bool pptype = line[1]=='p';    // 'pp' if next char is 'p'

                    // number of vertices, starting after "p " or "pp"
                    int nv;
                    if (sscanf(line+2, "%d", &nv) != 1 || nv < 3) 
                        err(lineNumber);

                    // polygon primitive w/ type
                    Polygon *poly = new Polygon(nv, app, pptype);

                    // read vertices
                    for(int i=0; i<nv; ++i) {
                        Vec3 v, n;
                        readLine(f, line, lineNumber);
                        if (sscanf(line,"%f %f %f %f %f %f", 
                                    &v[0], &v[1], &v[2],
                                    &n[0], &n[1], &n[2]) < 3)
                            err(lineNumber);
                        poly->addVertex(v, n);
                    }

                    poly->closePolygon();

                    if (World::effects & World::POLYGONS)
                        objects.addObject(poly);
                    else
                        delete poly;

                    break;
                }
            default:
                err(lineNumber);
        }
    }

    // rescale existing lights according to NFF expectation
    float lscale = 1/sqrtf(float(lights.size()));
    for(LightList::iterator li=lights.begin(); li!=lights.end(); ++li)
        li->col = li->col*lscale;
}
