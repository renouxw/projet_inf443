#include "birds.hpp"

using namespace vcl;
using namespace std;


hierarchy_mesh_drawable create_birds()
{
    hierarchy_mesh_drawable hierarchy;

    float const radius_head = 0.03f;
    float const radius_eye = 0.01f;
    float const radius_beak = 0.01f;

    vec3 const shape = {0.07f,0.19f,0.05f};

    // The geometry of the body is a sphere
    mesh_drawable body = mesh_drawable( mesh_primitive_ellipsoid(shape, {0,0,0}, 40, 40));
    body.shading.color = {0,0,0};

        // Geometry of the eyes: black spheres
    mesh_drawable eye = mesh_drawable(mesh_primitive_sphere(radius_eye, {0,0,0}, 20, 20));
    eye.shading.color = {0,1,0};

        //Geometry of the head : white sphere
    mesh_drawable head = mesh_drawable(mesh_primitive_sphere(radius_head, {0,0,0}, 40, 40));
    head.shading.color = {0,0,0};

        //Geometry of the beak
    mesh_drawable beak = mesh_drawable(mesh_primitive_cone(radius_beak, 0.05f, {0,0,0} , vec3(0,1,0),true, 40, 40));
    beak.shading.color = {202,20,0};

    //position shoulder_left
    vec3 const pt10 = vec3(0.05,-0.1,0);
    vec3 const pt11 = vec3(0.05,0.1,0);
    vec3 const pt12 = vec3(0.4,0.08,0);
    vec3 const pt13 = vec3(0.4,-0.08,0);
    //position arm_left

    vec3 const pt01 = vec3(0.15,-0.08,0);
    vec3 const pt1 = vec3(0.15,-0.081,0);
    vec3 const pt121 = vec3(0.0,0.08,0);
    vec3 const pt131 = vec3(0.0,-0.08,0);

    //position shoulder_right
    vec3 const pt20 = vec3(-0.05,-0.1,0);
    vec3 const pt21 = vec3(-0.05,0.1,0);
    vec3 const pt22 = vec3(-0.4,0.08,0);
    vec3 const pt23 = vec3(-0.4,-0.08,0);
    //position arm_right
    vec3 const pt02 = vec3(-0.15,-0.08,0);
    vec3 const pt2 = vec3(-0.15,-0.081,0);
    vec3 const pt221 = vec3(0.0,0.08,0);
    vec3 const pt231 = vec3(0.0,-0.08,0);


        // Shoulder part and arm are displayed as cylinder
    mesh_drawable shoulder_left = mesh_drawable(mesh_primitive_quadrangle(pt10,pt11,pt12,pt13));
    mesh_drawable arm_left = mesh_drawable(mesh_primitive_quadrangle(pt01,pt1,pt121,pt131));
    arm_left.shading.color = {0,0,0};
    shoulder_left.shading.color = {0,0,0};

    mesh_drawable shoulder_right = mesh_drawable(mesh_primitive_quadrangle(pt20,pt21,pt22,pt23));
    mesh_drawable arm_right = mesh_drawable(mesh_primitive_quadrangle(pt02,pt2,pt221,pt231));
    arm_right.shading.color = {0,0,0};
    shoulder_right.shading.color = {0,0,0};

        // Build the hierarchy:
        // ------------------------------------------- //
    // Syntax to add element
    //   hierarchy.add(visual_element, element_name, parent_name, (opt)[translation, rotation])

        // The root of the hierarchy is the body
        hierarchy.add(body, "body");

        //Head position
        vec3 const head_position = {0,0.18f,0.02f};

        hierarchy.add(head, "head", "body" , head_position);

        // Eyes positions are set with respect to some ratio of the body
        hierarchy.add(eye, "eye_left", "body" , head_position + radius_head * vec3( 1/3.0f, 1/2.0f, 1/1.5f));
        hierarchy.add(eye, "eye_right", "body", head_position + radius_head * vec3(-1/3.0f, 1/2.0f, 1/1.5f));

        //Beak
    hierarchy.add(beak, "beak", "body", head_position + radius_head*vec3(0,1,0));

        // Set the left part of the body arm: shoulder-elbow-arm
    hierarchy.add(shoulder_left, "shoulder_left", "body", {0,0,0}); // extremity of the spherical body
    hierarchy.add(arm_left, "arm_left", "shoulder_left", {0.4f,0,0});

        // Set the right part of the body arm: similar to the left part with a symmetry in x direction
    hierarchy.add(shoulder_right, "shoulder_right", "body", {0,0,0});
    hierarchy.add(arm_right, "arm_right", "shoulder_right", {-0.4f,0,0});

    return hierarchy;
}
