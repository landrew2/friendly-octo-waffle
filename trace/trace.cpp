// ray tracer main program
// includes input file parsing and spawning screen pixel rays

// classes used directly by this file
#include "ObjectList.hpp"
#include "Polygon.hpp"
#include "Sphere.hpp"
#include "Ray.hpp"
#include "World.hpp"
#include "Vec3.hpp"

// standard includes
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
// don't complain about MS-deprecated standard C functions
#pragma warning( disable: 4996 )
#endif

// conditional compilation for extra features
// so I can turn them off for baseline comparison
#define GAUSSIAN
#define DOF
//#define ANTIALIAS

// float in [0,1) distributed according to a Halton sequence with given base
float halton(int i, int base)
{
    int r = 0, scale=1;
    while(i != 0) {
        r = r * base + i % base;    // add digit to result
        i /= base;                  // update current value;
        scale *= base;              // increase total scale by one digit
    }
    return float(r)/float(scale);
}

// Box-Mueller transform to convert pair of random numbers on [-.5,.5)
// to pair of Gaussian-distributed random numbers with standard deviation 0.5
void gaussian(float &x, float &y)
{
    float radius = sqrt(-0.5f * log(0.5f - x));
    float theta = 2.f * M_PI * y;
    x = radius * cos(theta);
    y = radius * sin(theta);
}

// Transform a pair of random numbers on [.5,.5) to a disk of radius 0.5
void disk(float &x, float &y)
{
    float radius = 0.5 * sqrt(x + 0.5);
    float theta = 2.f * M_PI * y;
    x = radius * cos(theta);
    y = radius * sin(theta);
}

int main(int argc, char **argv)
{
    // defaults for command line arguments
    float aperture = 0;         // depth of field aperture
    int samples = 1;            // depth of field and antialiasing samples
    FILE *infile = stdin;       // input file

    // Default some things to off
    World::effects &= ~World::DEPTH_OF_FIELD;
    World::effects &= ~World::ANTIALIAS;
    
    // parse command line arguments
    char *progname = argv[0];
    ++argv; --argc;
    while(argc != 0) {
        // print usage on -h, -help, -?, --h, --help, etc.
        if (strncmp(argv[0], "-h", 2) == 0 || 
                strncmp(argv[0], "--h", 3) == 0 || 
                strcmp(argv[0], "-?") == 0)
            break;

        if (argc >= 2 && strcmp(argv[0], "-s") == 0) {
            sscanf(argv[1], "%d", &samples);
            argv += 2; argc -= 2;
            continue;
        }

        if (argc >= 2 && strcmp(argv[0], "-dof") == 0) {
            sscanf(argv[1], "%f", &aperture);
            World::effects |= World::DEPTH_OF_FIELD;
            argv += 2; argc -= 2;
            continue;
        }

        if (strcmp(argv[0], "-aa") == 0) {
            World::effects |= World::ANTIALIAS;
            argv += 1; argc -= 1;
            continue;
        }

        if (argc >= 2 && strcmp(argv[0], "-no") == 0) {
            if (strcmp(argv[1], "diffuse") == 0) 
                World::effects &= ~World::DIFFUSE;
            else if (strcmp(argv[1], "specular") == 0)
                World::effects &= ~World::SPECULAR;
            else if (strcmp(argv[1], "shadow") == 0)
                World::effects &= ~World::SHADOW;
            else if (strcmp(argv[1], "reflect") == 0)
                World::effects &= ~World::REFLECT;
            else if (strcmp(argv[1], "refract") == 0)
                World::effects &= ~World::REFRACT;
            else if (strcmp(argv[1], "polygons") == 0)
                World::effects &= ~World::POLYGONS;
            else if (strcmp(argv[1], "cones") == 0)
                World::effects &= ~World::CONES;
            else if (strcmp(argv[1], "spheres") == 0)
                World::effects &= ~World::SPHERES;
            else
                break;                  // leave unparsed, prints usage
            argv += 2; argc -= 2;
            continue;
        }

        if (argc == 1) {
            infile = fopen(argv[0], "r");
            if (!infile) {
                fprintf(stderr, "error opening %s\n", argv[1]);
                return 1;
            }
            argv += 1; argc -= 1;
            continue;
        }

        break;
    }

    // unparsed arguments? print usage and exit
    if (argc > 0) {
        printf("Usage: %s [options] [file.nff]\n", progname);
        printf("options:\n"
                "  -dof <aperture>\n"
                "    define a lens aperture for depth of field\n"
                "  -aa\n"
                "    enable antialiasing\n"
                "  -s <samples>\n"
                "    number of depth of field and antialiasing samples\n"
                "  -no diffuse, -no specular, -no shadow\n"
                "  -no reflect, -no refract\n"
                "  -no polygons, -no cones, -no spheres\n"
                "    turn off ray-tracing features\n");
        return 1;
    }

    // everything we know about the world
    // image parameters, camera parameters
    World world(infile);

    // array of image data in ppm-file order
    unsigned char (*pixels)[3] = new unsigned char[world.height*world.width][3];

    // spawn a ray for each pixel and place the result in the pixel
    for(int j=0; j<world.height; ++j) {

        if (j % 32 == 0) printf("line %d\n",j); // show current line
        for(int i=0; i<world.width; ++i) {

            // depth of field and antialiasing samples
            Vec3 col;
            for(int samp = 0; samp < samples; ++samp) {
                // Hammersley coordinate within pixel:
                //   (x bits | sample bits | reversed y bits)
                int ii = world.height*(i*samples + samp);
                ii += int(halton(j,2) * world.height);

                // new jittered eye position
                float dofX = 0, dofY = 0;
                if (World::effects & World::DEPTH_OF_FIELD) {
                    dofX = halton(ii, 3) - 0.5f, dofY = halton(ii, 5) - 0.5f;
                    disk(dofX, dofY);
                    dofX *= aperture; dofY *= aperture;
                }
                Vec3 eye = world.eye + dofX * world.u + dofY * world.v;

                // new ray center
                float aaX = 0, aaY = 0;
                if (World::effects & World::ANTIALIAS) {
                    aaX = halton(ii, 7) - 0.5f, aaY = halton(ii,11) - 0.5f;
                    gaussian(aaX, aaX);
                }
                float us = world.left + 
                    (world.right - world.left) * (i+aaX+0.5f)/world.width;
                float vs = world.top + 
                    (world.bottom - world.top) * (j+aaY+0.5f)/world.height;
                Vec3 pix = world.eye - world.dist * world.w 
                    + us * world.u + vs * world.v;

                // new ray allowing up to 5 bounces, ray contribution=255,
                // index of refraction=1, don't trace closer than hither plane
                Ray ray(eye, pix - eye, 
                        world.hither / world.dist, INFINITY,
                        5, 255);
                col = col + world.objects.trace(ray).color(world, ray);
            }
            col = col / float(samples);

            // assign color
            pixels[j*world.width + i][0] = col.r();
            pixels[j*world.width + i][1] = col.g();
            pixels[j*world.width + i][2] = col.b();
        }
    }
    printf("done\n");

    // write ppm file of pixels
    FILE *output = fopen("trace.ppm","wb");
    fprintf(output, "P6\n%d %d\n255\n", world.width, world.height);
    fwrite(pixels, world.height*world.width*3, 1, output);
    fclose(output);

    delete[] pixels;
    return 0;
}

