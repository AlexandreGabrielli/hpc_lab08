/* 06.05.2020 - Sydney Hauke - HPC - REDS - HEIG-VD */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <xmmintrin.h>

#include "image.h"
#include "ifs.h"
#include "random.h"

#define TRANSFORM_SIZE      6

/* Sierpinski triangle */
#define NB_ST_TRANSFORMS    3
float ST_TRANSFORMS[NB_ST_TRANSFORMS][TRANSFORM_SIZE] = {
{
    0.5f,  0.0f,   0.0f,
    0.0f,  0.5f,   0.0f,
},
{
    0.5f,  0.0f,   0.5f,
    0.0f,  0.5f,   0.0f,
},
{
    0.5f,  0.0f,   0.25f,
    0.0f,  0.5f,   0.5f,
}};

/* Barnsley's fern */
#define NB_BF_TRANSFORMS    4
float BF_TRANSFORMS[NB_BF_TRANSFORMS][TRANSFORM_SIZE] = {
{
    0.0f,  0.0f,   0.0f,
    0.0f,  0.16f,   0.0f,
},
{
    0.2f,  -0.26f,   0.0f,
    0.23f,  0.22f,   1.6f,
},
{
    -0.15f,  0.28f,   0.0f,
    0.26f,  0.24f,   0.44f,
},
{
    0.85f,  0.04f,   0.0f,
    -0.04f,  0.85f,   1.6f,
}};

struct ifs_t {
    float width;
    float height;

    float center_x;
    float center_y;

    size_t nb_transforms;
    float *transforms;
};

/* Sierpinski triangle IFS */
const struct ifs_t st_ifs = {
    .width  = 1.0f,
    .height = 1.0f,

    .center_x = 0.5f,
    .center_y = 0.5f,

    .nb_transforms = NB_ST_TRANSFORMS,
    .transforms = (float*)ST_TRANSFORMS,
};

/* Barnley's fern IFS */
const struct ifs_t bf_ifs = {
    .width  = 6.0f,
    .height = 10.0f,

    .center_x = 0.0f,
    .center_y = 4.5f,

    .nb_transforms = NB_BF_TRANSFORMS,
    .transforms = (float*)BF_TRANSFORMS, 
};



static size_t __ifs(struct img_t *img, const struct ifs_t *ifs, size_t passes, size_t width, size_t height);

void ifs(char *pathname, size_t passes, size_t min_width)
{
    const struct ifs_t *i = &bf_ifs;
    struct img_t *fractal_img;
    size_t width, height;
    float aspect_ratio;

    /* Fractals have a specific aspect ratio. The resulting image
     * must have the same aspect ratio as well */
    aspect_ratio = i->height / i->width;
    width = min_width;
    height = width * aspect_ratio;

    fractal_img = allocate_image(width, height, COMPONENT_GRAYSCALE);

    /* Generate fractal image */
    size_t points_drawn = __ifs(fractal_img, i, passes, width, height);
    printf("Number of points drawn : %lu\n", points_drawn);

    save_image(pathname, fractal_img);
    printf("Fractal saved as %s (%lu, %lu)\n", pathname, width, height);

    free_image(fractal_img);
}

static size_t __ifs(struct img_t *img, const struct ifs_t *ifs, size_t passes, size_t width, size_t height)
{
    /* TODO : do multiple instances of this algorithm so that the number of points generated 
     * per second increases. Use SIMD instructions. */

    /* This is the real number of iterations we do to draw the fractal */
    size_t count_points = width*height*passes*4;

    /* We start from the origin point 
    float p_x = 0.0f;
    float p_y = 0.0f;*/
    __m128 points_x;
    __m128 points_y;


    points_x = _mm_set_ps(0,0,0,0);
    points_y  = _mm_set_ps(0,0,0,0);

   // struct xorshift32_state rand_state; // random number generator

    srand(time(NULL));

    /* Choose a random starting state for the random number generator
    rand_state.a = rand();*/
    __m128i rand_idxs;
   rand_idxs = _mm_set_epi32(rand(), rand(), rand(), rand());


    
    for (size_t iterations = count_points/4; iterations != 0; iterations--) {
        /* Randomly choose an affine transform 
        size_t rand_idx = xorshift32(&rand_state) % ifs->nb_transforms;*/

        xorshift128( & rand_idxs);

        uint32_t tab[4] = {};
        _mm_store_si128((__m128i *)tab,rand_idxs);

        int i;
        for ( i = 0 ; i< 4 ; i++){
            tab[i]%=ifs->nb_transforms;
            tab[i]*=TRANSFORM_SIZE;
            }

        /* Apply choosen affine transform 
        affine_transform(&p_x, &p_y, &ifs->transforms[rand_idx*TRANSFORM_SIZE]);*/


        // transform[0] * xtmp 
        __m128 a = _mm_set_ps(
    (ifs->transforms[tab[0]*6])
    ,(ifs->transforms[tab[1]*6])
    ,(ifs->transforms[tab[2]*6])
    ,(ifs->transforms[tab[3]*6])
    );
    __m128 transfo = _mm_mul_ps(points_x,a);  

        // transform[1] * ytmp 
        __m128 b = _mm_set_ps(
    (ifs->transforms[tab[0]*6+1])
    ,(ifs->transforms[tab[1]*6+1])
    ,(ifs->transforms[tab[2]*6+1])
    ,(ifs->transforms[tab[3]*6+1])
    );
    __m128 transfo2 = _mm_mul_ps(points_y,a);     // (transform[1] * ytmp )
    
    __m128 rx = _mm_add_ps(transfo ,transfo2); // (transform[0] * xtmp ) + (transform[1] * ytmp )
    
    // transform[2] 
     b = _mm_set_ps(
    (ifs->transforms[tab[0]*6+2])
    ,(ifs->transforms[tab[1]*6+2])
    ,(ifs->transforms[tab[2]*6+2])
    ,(ifs->transforms[tab[3]*6+2])
    );    

    rx = _mm_add_ps(rx , b); // (transform[0] * xtmp ) + (transform[1] * ytmp ) + transform[2] 
    
    
    // transform[3] * xtmp 
      a = _mm_set_ps(
    (ifs->transforms[tab[0]*6+3])
    ,(ifs->transforms[tab[1]*6+3])
    ,(ifs->transforms[tab[2]*6+3])
    ,(ifs->transforms[tab[3]*6+3])
    );
    transfo = _mm_mul_ps(points_x,a);     // (transform[3] * xtmp )



        // transform[4] * ytmp 
        b = _mm_set_ps(
    (ifs->transforms[tab[0]*6+4])
    ,(ifs->transforms[tab[0]*6+4])
    ,(ifs->transforms[tab[0]*6+4])
    ,(ifs->transforms[tab[0]*6+4])
    );
    transfo2 = _mm_mul_ps(points_y,a);     // (transform[4] * ytmp )
    
    __m128 ry = _mm_add_ps(transfo ,transfo2); // (transform[0] * xtmp ) + (transform[1] * ytmp )
    
    // transform[5] 
    b = _mm_set_ps(
    (ifs->transforms[tab[0]*6+5])
    ,(ifs->transforms[tab[1]*6+5])
    ,(ifs->transforms[tab[2]*6+5])
    ,(ifs->transforms[tab[3]*6+5])
    );    

    ry = _mm_add_ps(rx , b); // (transform[0] * xtmp ) + (transform[1] * ytmp ) + transform[2]

     
    points_x = rx;
    points_y = ry;


        /*let without smid*/
        float pointsX [4] ={};
        float pointsY [4] = {};
        _mm_store_ps(pointsX , points_x);
        _mm_store_ps(pointsY , points_y);
            
         for (i = 0 ; i< 4 ; i++){
               float p_x;
               float p_y;
                p_x = pointsX[i];
                p_y = pointsY[i];
            /* Check if point lies inside the boundaries of our image */
            if (p_x < ifs->center_x+ifs->width/2 && p_x > ifs->center_x-ifs->width/2 && 
                p_y < ifs->center_y+ifs->height/2 && p_y > ifs->center_y-ifs->height/2) 
            {
                float left = ifs->center_x-ifs->width/2;
                float bottom = ifs->center_y-ifs->height/2;

                /* If point lies in the image, save and colour it in white */
                img->data[(height-1-(int32_t)((p_y-bottom)/ifs->height*height))*width + 
                (int32_t)((p_x-left)/ifs->width*width)] = UINT8_MAX;
            }
        }
    }

    return count_points;
}





