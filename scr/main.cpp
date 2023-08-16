//-------------------------------------------------------------------------------------------------
// testpt
// yoyolon - August 2022
// 
// Development
// Microsoft Visual Studio 2019
// C++17
// 
// External Library
// stb: https://github.com/nothings/stb
// 
// Reference(this software is based on listed below.)
// Raytracing in one weekend: https://raytracing.github.io/
// pbrt-v3: https://pbrt.org/
// 
// More information on these licenses can be found in NOTICE.txt
//-------------------------------------------------------------------------------------------------


#include "Renderer.h"
#include "Scene.h"
#include "Camera.h"
#include "MakeScene.h"

/**
* @brief mainä÷êî
*/
int main(int argc, char** argv) {
    Renderer renderer(4, Sampling::MIS);
    // ÉVÅ[Éì
    Scene world;
    Camera cam;
    //make_scene_simple(world, cam);
    //make_scene_simple2(world, cam);
    make_scene_cornell_box(world, cam);
    //make_scene_box_with_sphere(world, cam);
    //make_scene_vase(world, cam);
    //make_scene_thinfilm(world, cam);
    renderer.render(world, cam);
    return 0;
}