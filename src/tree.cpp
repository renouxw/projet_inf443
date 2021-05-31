#include "tree.hpp"

using namespace vcl;


mesh create_tree_trunk_cylinder(float radius, float height)
{
    mesh m;

    int n = 9;
    m.uv.resize(2*n);

    for(int k=0; k<n; k++)
    {
        const vec3 pt = {radius*std::cos(k/float(n)*2*3.14f), radius*std::sin(k/float(n)*2*3.14f), 0.0f};
        m.position.push_back(pt);
        //m.uv.push_back({radius*std::cos(k/float(n)*2*3.14f), radius*std::sin(k/float(n)*2*3.14f)});
        m.position.push_back(pt + vec3{0, 0, height});

    }
    for(int k=0; k<2*n-1; k+=2)
    {
        m.uv[k] = {k,0};
        m.uv[k+1] = {k,1};
    }
    for(int k=0; k<2*n-2; ++k)
    {
        if(k%2 == 0){
        m.connectivity.push_back({k, k+1, k+2});
        }
        else {
            m.connectivity.push_back({k, k+2, k+1});
        }
    }


    m.connectivity.push_back({2*n-2, 2*n-1, 0});
    m.connectivity.push_back({2*n-1, 1, 0});

    m.fill_empty_field();
    return m;
}


mesh create_cone(float radius, float height, float z_offset)
{
    mesh m;

    // conical structure
    // *************************** //

    const size_t N = 20;

    // geometry
    for(size_t k=0; k<N; ++k)
    {
        const float u = k/float(N);
        const vec3 p = {radius*std::cos(2*3.14f*u), radius*std::sin(2*3.14f*u), 0.0f};
        m.position.push_back( p+vec3{0,0,z_offset} );
    }
    // apex
    m.position.push_back({0,0,height+z_offset});

    // connectivity
    const unsigned int Ns = N;
    for(unsigned int k=0; k<Ns; ++k) {
        m.connectivity.push_back( {k , (k+1)%Ns, Ns} );
    }

    // close the bottom of the cone
    // *************************** //

    // Geometry
    for(size_t k=0; k<N; ++k)
    {
        const float u = k/float(N);
        const vec3 p = {radius*std::cos(2*3.14f*u), radius*std::sin(2*3.14f*u), 0.0f};
        m.position.push_back( p+vec3{0,0,z_offset} );
    }
    // central position
    m.position.push_back( {0,0,z_offset} );

    // connectivity
    for(unsigned int k=0; k<Ns; ++k)
        m.connectivity.push_back( {k+Ns+1, (k+1)%Ns+Ns+1, 2*Ns+1} );

    m.fill_empty_field();
    return m;
}

mesh create_tree_trunk()
{
    float const h = 0.3f; // trunk height
    float const r = 0.1f; // trunk radius

    // Create a brown trunk
    mesh trunk = create_tree_trunk_cylinder(r, h);
    trunk.color.fill({0.4f, 0.3f, 0.3f});


    return trunk;
}

mesh create_tree_foliage()
{
    float const h = 0.3f; // trunk height
    float const r = 0.1f; // trunk radius

    // Create a green foliage from 3 cones
    mesh foliage = create_cone(6*r, 10*r, 0.0f);      // base-cone
    foliage.push_back(create_cone(5*r, 10*r, 3*r));   // middle-cone
    foliage.push_back(create_cone(4*r, 10*r, 6*r));   // top-cone
    foliage.position += vec3(0,0,h);                 // place foliage at the top of the trunk
    foliage.color.fill({0.4f, 0.6f, 0.3f});

    return foliage;
}

mesh create_champi()
{
    float const h = 0.03f; // champi height
    float const r = 0.01f; // champi radius

    // Create a brown trunk
    mesh trunk = create_tree_trunk_cylinder(r, h);
    trunk.color.fill({1.f, 1.f, 1.f});

    mesh foliage = create_cone(4*r, 6*r, 0.0f); // top-cone
    foliage.position += vec3(0,0,h);      // place foliage at the top of the trunk
    foliage.color.fill({1.f, 0.f, 0.f});

    // The tree is composted of the trunk and the foliage
    mesh champi = trunk;
    champi.push_back(foliage);

    return champi;
}

mesh create_street_lamp(){

    float height = 1.0f;

    //create basement
    mesh socle = create_tree_trunk_cylinder(0.08f,0.3f);
    socle.color.fill({0,0,0});

    //create pole
    mesh pole = create_tree_trunk_cylinder(0.05f,height);
    pole.color.fill({0,0,0});

    socle.push_back(pole);

    return socle;

}

mesh create_rectangle(vec3 const& center, float length, float width, float height, bool test){
    vec3 u = length*vec3{1,1,1};
    vec3 v = width*vec3{1,1,1};
    vec3 w = height*vec3{1,1,1};

    vec3 p000 = center;
    vec3 p100 = p000 + u*vec3{1,0,0};
    vec3 p110 = p000 + v*vec3{0,1,0};
    vec3 p010 = p000 + u*vec3{1,0,0} + v*vec3{0,1,0};
    vec3 p001 = p100 + w*vec3{0,0,1};
    vec3 p101 = p110 + w*vec3{0,0,1};
    vec3 p111 = p010 + w*vec3{0,0,1};
    vec3 p011 = p000 + w*vec3{0,0,1};

    mesh shape;
    if (test) shape.push_back(mesh_primitive_quadrangle(p011, p001, p111, p101));
    shape.push_back(mesh_primitive_quadrangle(p000, p100, p010, p110));
    shape.push_back(mesh_primitive_quadrangle(p000, p100, p001, p011));
    shape.push_back(mesh_primitive_quadrangle(p000, p110, p101, p011));
    shape.push_back(mesh_primitive_quadrangle(p010, p110, p101, p111));
    shape.push_back(mesh_primitive_quadrangle(p010, p100, p001, p111));

    return shape;
}

mesh create_fontaine(){
    mesh socle = create_rectangle({0,0,0}, 2, 2, 0.6, false);
    mesh fond = create_rectangle({0.001,0.001,0}, 1.99, 1.99, 0.35, true);
    mesh mur = create_rectangle({0.01,0.75,0}, 0.1, 0.4, 1.4, true);
    mesh robinet = mesh_primitive_cylinder(0.02,{0.1,1,1.3},{0.5,1,1.3},40,40,false);
    robinet.color.fill({0.8,1,1});

    socle.push_back(mur);
    socle.push_back(fond);
    socle.push_back(robinet);

    return socle;
}

