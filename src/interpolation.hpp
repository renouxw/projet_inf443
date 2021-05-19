#pragma once


#include "vcl/vcl.hpp"
#include <math.h>
#include <cstdlib>

// Compute the interpolated position p(t) given a time t and the set of key_positions and key_frame
vcl::vec3 const interpolation(float t, vcl::buffer<vcl::vec3> const& key_positions, vcl::buffer<float> const& key_times);
float const direction(float t, vcl::buffer<vcl::vec3> const& key_positions, vcl::buffer<float> const& key_times, const vcl::vec3& dir);
vcl::vec3 const new_direction(float t, vcl::buffer<vcl::vec3> const& key_positions, vcl::buffer<float> const& key_times);
