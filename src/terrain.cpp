
#include "terrain.hpp"

using namespace vcl;
using namespace std;

perlin_noise_parameters parameters;

// Evaluate 3D position of the terrain for any (u,v) \in [0,1]
vec3 evaluate_terrain(float u, float v)
{
    float const x = 20*(u-0.5f);
    float const y = 20*(v-0.5f);

    std::array<vec2, 4> const p = {vec2{0.f,0.f},vec2{1.5,0.5},vec2{0.2,0.4},vec2{0.8,0.7}};
    vcl::buffer_stack<float, 4> const h = {3,-1.5,1,2};
    vcl::buffer_stack<float, 4> const sigma = {0.2,0.15,0.1,0.2};

    float z = 0;

    for (int n=3; n>=0; n--)
      {
        float d = norm(vec2(u,v)-p[n])/sigma[n];
        z += h[n]*std::exp(-d*d);

        // Compute the Perlin noise
        float const noise = noise_perlin({u, v}, parameters.octave, parameters.persistency, parameters.frequency_gain);
        z += parameters.terrain_height*noise;
      }
    return {x,y,z};
}

mesh create_terrain()
{
    // Number of samples of the terrain is N x N
    const unsigned int N = 100;

    mesh terrain; // temporary terrain storage (CPU only)
    terrain.position.resize(N*N);
    terrain.uv.resize(N*N);

    // Fill terrain geometry
    for(unsigned int ku=0; ku<N; ++ku)
    {
        for(unsigned int kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(N-1.0f);
            const float v = kv/(N-1.0f);

            // Compute the local surface function
            vec3 const p = evaluate_terrain(u,v);

            // Store vertex coordinates
            terrain.position[kv+N*ku] = p;
            terrain.uv[kv+N*ku] = {10*u,10*v};
        }
    }

    // Generate triangle organization
    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    for(size_t ku=0; ku<N-1; ++ku)
    {
        for(size_t kv=0; kv<N-1; ++kv)
        {
            const unsigned int idx = kv + N*ku; // current vertex offset

            const uint3 triangle_1 = {idx, idx+1+N, idx+1};
            const uint3 triangle_2 = {idx, idx+N, idx+1+N};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

	terrain.fill_empty_field(); // need to call this function to fill the other buffer with default values (normal, color, etc)
    return terrain;
}

std::vector<vcl::vec3> generate_positions_on_terrain(int N){

    std::vector<vcl::vec3> pos;

    for (int k=0; k<N; k++){
        pos.push_back(vec3(evaluate_terrain(rand_interval(0,1),rand_interval(0,1))));
    }
    return pos;
}
