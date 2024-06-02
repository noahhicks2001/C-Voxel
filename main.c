#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <math.h>
#include <stdbool.h>

typedef struct Vec4 {
    double x, y, z, w;
}Vec4;

typedef struct Mat4x4 {
    Vec4 cols[4];
}Mat4x4;

typedef struct Vertices {
    Mat4x4 front;
    Mat4x4 back;
} Vertices;

typedef struct Transformation {
    Vec4 scale;
    Vec4 rotation;
    Vec4 translation;
} Transformation;

typedef struct Voxel {
    int length;
    Vertices vertices;
    Transformation transformation;
    bool transformed;
} Voxel;

typedef struct State {
    bool running;
    int voxel_count;
    int selected_voxel;
} State;


// VEC4 FUNCTIONS //
void print_vec4(const Vec4* v) {
    printf("%f\n%f\n%f\n%f\n", v->x, v->y, v->z, v->w);
    printf("\n");
}


void set_vec4(Vec4* v, int x, int y, int z, int w) {
    v->x = x;
    v->y = y;
    v->z = z;
    v->w = w;
}

void vec4_copy(Vec4* u, Vec4* v) {
    /*
    * Computes U <- V
    */
    u->x = v->x;
    u->y = v->y;
    u->z = v->z;
    u->w = v->w;
}

void vec4_addition(Vec4* u, Vec4* v, Vec4* w) {
    /*
    * Computes W <- U + V
    */
    w->x = u->x + v->x;
    w->y = u->y + v->y;
    w->z = u->z + v->z;
    w->w = u->w + v->w;
}


void vec4_subtraction(Vec4* u, Vec4* v, Vec4* w) {
    /*
    * Computes W <- U - V
    */
    w->x = u->x - v->x;
    w->y = u->y - v->y;
    w->z = u->z - v->z;
    w->w = u->w - v->w;
}

void vec4_scalar_mult(Vec4* u, double t, Vec4* w) {
    /*
    * Computes W <- tU
    */
    w->x = u->x * t;
    w->y = u->y * t;
    w->z = u->z * t;
    w->w = u->w * t;
}

double vec4_dot_product(const Vec4* u, const Vec4* v) {
    return (u->x * v->x) + (u->y * v->y) + (u->z * v->z) + (u->w * v->w);
}


double vec4_magnitude(const Vec4* u) {
    return sqrt(pow(u->x, 2) + pow(u->y, 2) + pow(u->z, 2));
}

void vec4_normalization(Vec4* u, Vec4* w) {
    double mag = vec4_magnitude(u);
    w->x = u->x / mag;
    w->y = u->y / mag;
    w->z = u->z / mag;
    w->w = u->w / mag;
}


// MAT4x4 FUNCTIONS //
void print_mat4x4(Mat4x4* a) {
    printf("%f %f %f %f\n", a->cols[0].x, a->cols[1].x, a->cols[2].x, a->cols[3].x);
    printf("%f %f %f %f\n", a->cols[0].y, a->cols[1].y, a->cols[2].y, a->cols[3].y);
    printf("%f %f %f %f\n", a->cols[0].z, a->cols[1].z, a->cols[2].z, a->cols[3].z);
    printf("%f %f %f %f\n", a->cols[0].w, a->cols[1].w, a->cols[2].w, a->cols[3].w);
    printf("\n");

}

void copy_mat4x4(Mat4x4* a, Mat4x4* b) {
    /*
    * Computes A <- B
    */
    for (int i = 0; i < 4; i++) {
        vec4_copy(&a->cols[i], &b->cols[i]);
    }
}

void mat4x4_addition(Mat4x4* a, Mat4x4* b, Mat4x4* c) {
    /*
    * Computes C <- A + B
    */
    for (int i = 0; i < 4; i++) {
        vec4_addition(&a->cols[i], &b->cols[i], &c->cols[i]);
    }
}


void mat4x4_vec4_mult(Mat4x4* a, Vec4* x, Vec4* b) {
    /*
    * Computes b <- Ax
    */

    // transpose row vectors for dot product
    b->x = vec4_dot_product(&(Vec4) { a->cols[0].x, a->cols[1].x, a->cols[2].x, a->cols[3].x }, x);
    b->y = vec4_dot_product(&(Vec4) { a->cols[0].y, a->cols[1].y, a->cols[2].y, a->cols[3].y }, x);
    b->z = vec4_dot_product(&(Vec4) { a->cols[0].z, a->cols[1].z, a->cols[2].z, a->cols[3].z }, x);
    b->w = vec4_dot_product(&(Vec4) { a->cols[0].w, a->cols[1].w, a->cols[2].w, a->cols[3].w }, x);

}
void mat4x4_mat4x4_mult(Mat4x4* a, Mat4x4* b, Mat4x4* c) {
    /*
    * Computes C <- AB
    */
    for (int i = 0; i < 4; i++) {
        // mult each row of A by each column of B
        mat4x4_vec4_mult(a, &b->cols[i], &c->cols[i]);
    }
}

void mat4x4_scalar_mult(Mat4x4* a, double t, Mat4x4* b) {
    /*
    * Computes B <- tA
    */
    for (int i = 0; i < 4; i++) {
        vec4_scalar_mult(&a->cols[i], t, &b->cols[i]);
    }
}




// VOXEL FUNCTIONS //

void print_voxel(Voxel* voxel) {


    printf("LENGTH \n");
    printf("%i \n", voxel->length);

    printf("FRONT VECTICES \n");
    print_mat4x4(&voxel->vertices.front);

    printf("BACK VERTICES \n");
    print_mat4x4(&voxel->vertices.back);

    printf("SCALE VEC \n");
    print_vec4(&voxel->transformation.scale);

    printf("ROTATION VEC\n");
    print_vec4(&voxel->transformation.rotation);

    printf("TRANSLATION VEC \n");
    print_vec4(&voxel->transformation.translation);

    printf("\n\n");
}


// SDL FUNCTIONS //

void clear_screen(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}


void draw_point(SDL_Renderer* renderer, int x, int y) {
    // set render color to white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawPoint(renderer, x, y);
}


void interpolate(SDL_Renderer* renderer, const Vec4* u, const Vec4* v) {
    /*
    * Draw line from U->V using vector definition of line
    */

    Vec4 line;
    vec4_subtraction(v, u, &line);

    Vec4 line_norm;
    vec4_normalization(&line, &line_norm);

    for (int i = 0; i < vec4_magnitude(&line); i++) {
        draw_point(renderer, u->x + (i * line_norm.x), u->y + (i * line_norm.y));
    }

}

void set_rot_mat4x4(Mat4x4* rot, Vec4* values) {

    // temporary matrix for intermediate values
    Mat4x4 temp;

    // create axis rotation matrices
    Mat4x4 x_rot = { 1, 0, 0, 0,
               0, cos(values->x * M_PI / 180), -sin(values->x * M_PI / 180), 0,
               0, sin(values->x * M_PI / 180), cos(values->x * M_PI / 180), 0,
               0,0,0,1
    };

    Mat4x4 y_rot = { cos(values->y * M_PI / 180), 0, sin(values->y * M_PI / 180), 0,
                  0, 1, 0, 0,
                  -sin(values->y * M_PI / 180), 0, cos(values->y * M_PI / 180), 0,
                  0, 0, 0, 1

    };

    Mat4x4 z_rot = { cos(values->z * M_PI / 180), sin(values->z * M_PI / 180), 0, 0,
                    -sin(values->z * M_PI / 180), cos(values->z * M_PI / 180), 0, 0,
                    0, 0, 1,0,
                    0, 0, 0, 1
    };

  
    // compute rotations
    mat4x4_mat4x4_mult(&x_rot, &z_rot, &temp);
    mat4x4_mat4x4_mult(&y_rot, &temp, rot);

}

void set_scale_mat4x4(Mat4x4* scale, Vec4* values) {
    // assign values to scale matrix
    *scale =  (Mat4x4){values->x,  0,   0, 0,
                        0,  values->y, 0, 0,
                        0, 0, values->z, 0,
                        0, 0 ,   0,      1, };
}

void set_translation_mat4x4(Mat4x4* trans, Vec4* values) {
    // assign values to translation matrix
    *trans = (Mat4x4){1,0,0,0,
                      0,1,0,0,
                      0,0,1,0,
                      values->x,values->y,values->z,1 };

}

void reset_voxel_vertices(Voxel* voxel) {
    // reset vertices to local space for voxel
    voxel->vertices.front = (Mat4x4){ -1,-1,-1,1,  1,-1,-1,1,  1, 1, -1, 1,  -1, 1, -1, 1 };
    voxel->vertices.back = (Mat4x4){ -1,-1,1,1,  1,-1,1,1,  1, 1, 1, 1,  -1, 1, 1, 1 };
}
void scale_voxel(Voxel* voxel) {

    // create temporary matrices for intermediate values
    Mat4x4 front;
    Mat4x4 back;

    // create scale matrix
    Mat4x4 scale;
    set_scale_mat4x4(&scale, &voxel->transformation.scale);
   
    // scale front
    mat4x4_mat4x4_mult(&scale, &voxel->vertices.front, &front);

    // scale back
    mat4x4_mat4x4_mult(&scale, &voxel->vertices.back, &back);

    // copy results to voxel vertices
    copy_mat4x4(&voxel->vertices.front, &front);
    copy_mat4x4(&voxel->vertices.back, &back);

}

void rotate_voxel(Voxel* voxel) {
    // create temporary matrices for intermediate values
    Mat4x4 front;
    Mat4x4 back;

    // create rotation matrix
    Mat4x4 rot;
    set_rot_mat4x4(&rot, &voxel->transformation.rotation);
  

    // rotate front
    mat4x4_mat4x4_mult(&rot, &voxel->vertices.front, &front);
    // rotate back
    mat4x4_mat4x4_mult(&rot, &voxel->vertices.back, &back);

    // copy results to voxel world space vertices
    copy_mat4x4(&voxel->vertices.front, &front);
    copy_mat4x4(&voxel->vertices.back, &back);

}



void translate_voxel(Voxel* voxel) {
    // create temporary matrices for intermediate values
    Mat4x4 front;
    Mat4x4 back;

    // create translation matrix
    Mat4x4 trans;
    set_translation_mat4x4(&trans, &voxel->transformation.translation);

    // translate world space front/back
    mat4x4_mat4x4_mult(&trans, &voxel->vertices.front, &front);
    mat4x4_mat4x4_mult(&trans, &voxel->vertices.back, &back);

    // copy results to voxel world space vertices
    copy_mat4x4(&voxel->vertices.front, &front);
    copy_mat4x4(&voxel->vertices.back, &back);

}

void transform_voxel(Voxel* voxel) {
    reset_voxel_vertices(voxel);
    scale_voxel(voxel);
    rotate_voxel(voxel);
    translate_voxel(voxel);
    voxel->transformed = false;
}


void draw_voxel(SDL_Renderer* renderer, Voxel* voxel) {

    // FRONT
    interpolate(renderer, &voxel->vertices.front.cols[0], &voxel->vertices.front.cols[1]);
    interpolate(renderer, &voxel->vertices.front.cols[1], &voxel->vertices.front.cols[2]);
    interpolate(renderer, &voxel->vertices.front.cols[2], &voxel->vertices.front.cols[3]);
    interpolate(renderer, &voxel->vertices.front.cols[3], &voxel->vertices.front.cols[0]);

    
    // BACK
    interpolate(renderer, &voxel->vertices.back.cols[0], &voxel->vertices.back.cols[1]);
    interpolate(renderer, &voxel->vertices.back.cols[1], &voxel->vertices.back.cols[2]);
    interpolate(renderer, &voxel->vertices.back.cols[2], &voxel->vertices.back.cols[3]);
    interpolate(renderer, &voxel->vertices.back.cols[3], &voxel->vertices.back.cols[0]);


    // SIDES
    interpolate(renderer, &voxel->vertices.front.cols[0], &voxel->vertices.back.cols[0]);
    interpolate(renderer, &voxel->vertices.front.cols[1], &voxel->vertices.back.cols[1]);
    interpolate(renderer, &voxel->vertices.front.cols[2], &voxel->vertices.back.cols[2]);
    interpolate(renderer, &voxel->vertices.front.cols[3], &voxel->vertices.back.cols[3]);
    

}


void handle_screen(SDL_Renderer* renderer, State* state, Voxel* voxels) {

    clear_screen(renderer);
    for (int i = 0; i < state->voxel_count; i++) {
        if (voxels[i].transformed) {
            transform_voxel(&voxels[i]);
        }
        draw_voxel(renderer, &voxels[i]);
    }
    SDL_RenderPresent(renderer);
}


void update_selection(SDL_Event* event, State* state) {
    if (event->key.repeat == 0) {
        switch (event->key.keysym.sym) {
        case SDLK_LEFT:
            state->selected_voxel -= 1;
            state->selected_voxel = (state->selected_voxel + state->voxel_count) % state->voxel_count;
            printf("VOXEL %i\n", state->selected_voxel);
            break;

        case SDLK_RIGHT:
            state->selected_voxel += 1;
            state->selected_voxel = (state->selected_voxel + state->voxel_count) % state->voxel_count;
            printf("VOXEL %i\n", state->selected_voxel);
            break;

        }
    }
}


void move_voxel(SDL_Event* event, Voxel* voxel) {
    switch (event->key.keysym.sym) {

    case SDLK_w:
        voxel->transformation.translation.y -= 10;
        voxel->transformed = true;
        break;

    case SDLK_a:
        voxel->transformation.translation.x -= 10;
        voxel->transformed = true;
        break;

    case SDLK_s:
        voxel->transformation.translation.y += 10;
        voxel->transformed = true;
        break;

    case SDLK_d:
        voxel->transformation.translation.x += 10;
        voxel->transformed = true;
        break;

    case SDLK_u:
        voxel->transformation.rotation.x -= 1;
        voxel->transformed = true;
        break;

    case SDLK_j:
        voxel->transformation.rotation.x += 1;
        voxel->transformed = true;
        break;

    case SDLK_y:
        voxel->transformation.rotation.y -= 1;
        voxel->transformed = true;
        break;

    case SDLK_i:
        voxel->transformation.rotation.y += 1;
        voxel->transformed = true;
        break;

    case SDLK_h:
        voxel->transformation.rotation.z -= 1;
        voxel->transformed = true;
        break;

    case SDLK_k:
        voxel->transformation.rotation.z += 1;
        voxel->transformed = true;
        break;

    case SDLK_r:
        voxel->length -= 1;
        voxel->transformation.scale.x -= 1;
        voxel->transformation.scale.y -= 1;
        voxel->transformation.scale.z -= 1;
        voxel->transformed = true;
        printf("LENGTH %i\n", voxel->length);
        break;

    case SDLK_t:
        voxel->length += 1;
        voxel->transformation.scale.x += 1;
        voxel->transformation.scale.y += 1;
        voxel->transformation.scale.z += 1;
        voxel->transformed = true;
        printf("LENGTH %i\n", voxel->length);
        break;

    }
}


void handle_input(SDL_Event* event, State* state, Voxel* voxels) {

    SDL_PollEvent(event);
    if (event->type == SDL_QUIT) {
        state->running = false;
    }
    else if (event->type == SDL_KEYDOWN) {
        update_selection(event, state);
        move_voxel(event, &voxels[state->selected_voxel]);
    }
 
}
   
/*
init code referenced from 
https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php
*/
void init_sdl(SDL_Renderer** renderer, SDL_Window** window, int screen_width,
    int screen_height) {

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("INIT ERROR\n");
    }

    *window = SDL_CreateWindow("Voxel", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_OPENGL);

    if (*window == NULL) {
        printf("WINDOW ERROR\n");
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);

    if (*renderer == NULL) {
        printf("RENDERER ERROR\n");
    }
}


void init_voxel(Voxel* voxel) {

    // set length
    voxel->length = 50;

    // set vertices
    voxel->vertices.front = (Mat4x4){ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
    voxel->vertices.back = (Mat4x4){ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };

    // set transformation vectors
    voxel->transformation.scale = (Vec4){ 50,50,50,0 };        // scale by initial value for visibility
    voxel->transformation.rotation = (Vec4){ 0,0,0,0 }; 
    voxel->transformation.translation = (Vec4){ 50,50,50,0 };  // places voxel initially at top left corner

    // set transform status
    voxel->transformed = true;
}
  
void init_state(State *state) {
    state->running = true;
    state->selected_voxel = 0;
    state->voxel_count = 0;
}
  

int main(int argc, char* args[]) {


    // SDL INIT //
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
    SDL_Event event;

    init_sdl(&renderer, &window, 640, 480);

    // INIT VOXELS //
    Voxel voxels[3];

    for (int i = 0; i < 3; i++) {
        init_voxel(&voxels[i]);
    }

    // INIT STATE //
    State state;
    init_state(&state);
    state.voxel_count = 3;


    while(state.running) {
        handle_input(&event, &state, &voxels);
        handle_screen(renderer, &state, &voxels);

    }
   
   
    
}


