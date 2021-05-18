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

    int n = 15;

    m.position.push_back({0.f, 0.f, z_offset});

    for(int k=0; k<n; k++)
    {
        const vec3 pt = {radius*std::cos(k/float(n)*2*3.14f), radius*std::sin(k/float(n)*2*3.14f), z_offset};
        m.position.push_back(pt);
    }

    m.position.push_back({0.f, 0.f, height + z_offset});

    for(int k=1; k<n; ++k)
    {

        m.connectivity.push_back({0.f, k, k+1});
        m.connectivity.push_back({k, n+1, k+1});

    }

    m.connectivity.push_back({0.f, n-1, 1.f});
    m.connectivity.push_back({n-1, n+1, 1.f});


    m.fill_empty_field();
    return m;
}

mesh create_tree_trunk()
{
    float const h = 1.0f; // trunk height
    float const r = 0.1f; // trunk radius

    // Create a brown trunk
    mesh trunk = create_tree_trunk_cylinder(r, h);
    trunk.color.fill({0.4f, 0.3f, 0.3f});


    return trunk;
}

mesh create_tree_foliage()
{
    float const h = 1.0f; // trunk height
    float const r = 0.1f; // trunk radius

    // Create a green foliage from 3 cones
    mesh foliage = create_cone(4*r, 6*r, 0.0f);      // base-cone
    foliage.push_back(create_cone(4*r, 6*r, 2*r));   // middle-cone
    foliage.push_back(create_cone(4*r, 6*r, 4*r));   // top-cone
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
    foliage.position += vec3(0,0,h);                 // place foliage at the top of the trunk
    foliage.color.fill({1.f, 0.f, 0.f});

    // The tree is composted of the trunk and the foliage
    mesh champi = trunk;
    champi.push_back(foliage);

    return champi;
}

