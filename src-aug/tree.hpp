#pragma once

#include "vcl/vcl.hpp"


vcl::mesh create_tree_trunk_cylinder(float radius, float height);
vcl::mesh create_cone(float radius, float height, float z_offset);
vcl::mesh create_tree_trunk();
vcl::mesh create_tree_foliage();
vcl::mesh create_champi();
vcl::mesh create_street_lamp();
