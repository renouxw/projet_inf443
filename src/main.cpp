 #include "vcl/vcl.hpp"
#include <iostream>
#include "terrain.hpp"
#include "birds.hpp"
#include "tree.hpp"
#include "interpolation.hpp"
#include <list>


using namespace vcl;

struct picking_structure {
        bool active;
        int index;
};

struct gui_parameters {
        bool display_frame = true;
        bool add_sphere = true;
        bool display_polygon = true;
        bool display_keyposition = true;
        bool display_trajectory = true;
        int trajectory_storage = 100;
        bool display_surface = true;
        bool display_wireframe = false;
};

struct user_interaction_parameters {
        vec2 mouse_prev;
        timer_fps fps_record;
        mesh_drawable global_frame;
        gui_parameters gui;
        bool cursor_on_gui;
        picking_structure picking;
};
user_interaction_parameters user;


// Structure of a particle
struct particle_structure
{
    vcl::vec3 p; // Position
    vcl::vec3 v; // Speed
};



struct scene_environment
{
        camera_around_center camera;
        mat4 projection;
        vec3 light;
        // Consider a set of spotlight defined by their position and color
        std::array<vec3,5> spotlight_position;
        std::array<vec3,5> spotlight_color;
        float spotlight_falloff = 0.5;
        float fog_falloff = false;
        float t;
};
scene_environment scene;

buffer<vec3> key_positions;
buffer<float> key_times;
timer_interval timer;

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
void display_scene();
void display_interface();

mesh terrain_visual;

mesh_drawable billboard_grass;
mesh_drawable billboard_flower;
mesh_drawable terrain;
mesh_drawable trunk;
mesh_drawable street_lamp;
mesh_drawable tore;
mesh_drawable foliage;
mesh_drawable champi;
mesh_drawable fontaine;
std::vector<vcl::vec3> tree_position;
std::vector<vcl::vec3> champi_position;
std::vector<vcl::vec3> street_lamp_position;
std::vector<vcl::vec3> grass_position;

mesh_drawable sphere_current;    // sphere used to display the interpolated value
mesh_drawable sphere_keyframe;   // sphere used to display the key positions
curve_drawable polygon_keyframe; // Display the segment between key positions
trajectory_drawable trajectory;  // Temporary storage and display of the interpolated trajectory

hierarchy_mesh_drawable hierarchy1;
hierarchy_mesh_drawable hierarchy2;

std::list<particle_structure> particles; // Storage of all currently active particles
std::list<particle_structure> birds; // Storage of all currently active particles
std::list<particle_structure> neiges; // Storage of all currently active particles
mesh_drawable sphere;
mesh_drawable snow;

mesh_drawable moon;

vec3 dir = vec3(0,-1,0);

mesh_drawable cube;
mesh_drawable sphere_spotlight;
mesh_drawable ground;


mesh_drawable grid;
mesh_drawable grid2;

mesh_drawable trunk2;
mesh_drawable branches;
mesh_drawable foliage2;

mesh_drawable rock;

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

        int const width = 3280, height = 1524;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
	std::cout << opengl_info_display() << std::endl;;

	imgui_init(window);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	
	std::cout<<"Initialize data ..."<<std::endl;
	initialize_data();

	std::cout<<"Start animation loop ..."<<std::endl;
	user.fps_record.start();
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
                //scene.light = scene.camera.position();
		user.fps_record.update();
		
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
                //user.cursor_on_gui = ImGui::IsAnyWindowFocused();
                user.cursor_on_gui = ImGui::GetIO().WantCaptureMouse;

		if(user.gui.display_frame) draw(user.global_frame, scene);

		display_interface();
                display_scene();

		ImGui::End();
		imgui_render_frame(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	imgui_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void initialize_data()
{
        // Read the specific shader and generate a uniform grid
        GLuint const shader_deform = opengl_create_shader_program( read_text_file("shader/shader_deform.vert.glsl"), read_text_file("shader/shader_deform.frag.glsl"));

        grid = mesh_drawable( mesh_primitive_grid({-1,-1,0},{1,-1,0},{1,1,0},{-1,1,0},400,400) );
        grid.transform.translate = {-1.5,2.0f, 0.7};
        grid.shader = shader_deform;
        grid.shading.color = {0.0f, 0.94f, 1.0f};
        grid.texture = opengl_texture_to_gpu(image_load_png("assets/water.png"));

        GLuint const shader_mesh = opengl_create_shader_program(read_text_file("shader/mesh_lights.vert.glsl"),read_text_file("shader/mesh_lights.frag.glsl"));
	GLuint const shader_uniform_color = opengl_create_shader_program(opengl_shader_preset("single_color_vertex"), opengl_shader_preset("single_color_fragment"));
	GLuint const texture_white = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});
        mesh_drawable::default_texture = texture_white;

        /** Load a shader that makes fully transparent fragments when alpha-channel of the texture is small */
        GLuint const shader_with_transparency = opengl_create_shader_program( read_text_file("shader/transparency.vert.glsl"), read_text_file("shader/transparency.frag.glsl"));

        foliage2 = mesh_drawable( mesh_load_file_obj("assets/foliage.obj") );
        foliage2.texture = opengl_texture_to_gpu( image_load_png("assets/pine.png") );
        foliage2.shader = shader_with_transparency; // set the shader handling transparency for the foliage
        foliage2.shading.phong = {0.4f, 0.6f, 0, 1};     // remove specular effect for the billboard

        mesh_drawable::default_shader = shader_mesh;

        trunk2 = mesh_drawable( mesh_load_file_obj("assets/trunk.obj"));
        trunk2.texture = opengl_texture_to_gpu( image_load_png("assets/trunk.png") );

        branches = mesh_drawable( mesh_load_file_obj("assets/branches.obj"));
        branches.shading.color = {0.45f, 0.41f, 0.34f}; // branches do not have textures

	user.global_frame = mesh_drawable(mesh_primitive_frame());
	user.gui.display_frame = false;
        scene.camera.distance_to_center = 10.0f;
        scene.camera.look_at({4,3,2}, {0,0,1}, {0,0,2});

        /** *************************************************************  **/
    /** Terrain  **/
    /** *************************************************************  **/

    // Create visual terrain surface
        terrain_visual = create_terrain();
        terrain = mesh_drawable(terrain_visual);

    terrain.shading.color = {1.0f, 1.0f, 1.0f};
    terrain.shading.phong.specular = 0.0f; // non-specular terrain material

        /** *************************************************************  **/
    /** Motionless Objects  **/
    /** *************************************************************  **/

    billboard_grass = mesh_drawable(mesh_primitive_quadrangle({-0.5,0,0},{0.5,0,0},{0.5,0,1},{-0.5,0,1}));
            billboard_grass.transform.scale = 0.4f;
            billboard_grass.transform.translate = {0.5f, 0.5f, 0.0f};
            billboard_grass.texture = opengl_texture_to_gpu(image_load_png("assets/grass.png"));

    trunk = mesh_drawable(create_tree_trunk());
    foliage = mesh_drawable(create_tree_foliage());

    street_lamp = mesh_drawable(create_street_lamp());
    mesh torus = mesh_primitive_torus(0.08f, 0.02f, {0,0,0}, {0,0,1}, 20,20);
    torus.color.fill({0,0,0});
    tore = mesh_drawable(torus);

    fontaine = mesh_drawable(create_fontaine());
    fontaine.texture = opengl_texture_to_gpu(image_load_png("assets/rock.png"));

    moon = mesh_drawable(mesh_primitive_sphere(1.0f));
    moon.shading.color = {1.0f,1.0f,1.0f};
    moon.transform.translate = {15,40,15};
    moon.texture = opengl_texture_to_gpu(image_load_png("assets/moon.png"));
    moon.shading.phong = {0.4f, 0.6f, 0, 1};

    image_raw const im = image_load_png("assets/texture_grass.png");
    GLuint const texture_image_id = opengl_texture_to_gpu(im,
            GL_MIRRORED_REPEAT /**GL_TEXTURE_WRAP_S*/,
            GL_MIRRORED_REPEAT /**GL_TEXTURE_WRAP_T*/);
    terrain.texture = texture_image_id;

    image_raw const im2 = image_load_png("assets/trunk.png");
    GLuint const texture_image_id2 = opengl_texture_to_gpu(im2,
            GL_MIRRORED_REPEAT /**GL_TEXTURE_WRAP_S*/,
            GL_MIRRORED_REPEAT /**GL_TEXTURE_WRAP_T*/);
    trunk.texture = texture_image_id2;

    // create a billboard of flower
    billboard_flower = mesh_drawable(mesh_primitive_quadrangle({-1,0,0},{1,0,0},{1,0,2},{-1,0,2}));
    billboard_flower.transform.scale = 0.2f;
    billboard_flower.transform.translate = {0.5f, 0.5f, 0.0f};
    billboard_flower.texture = opengl_texture_to_gpu(image_load_png("assets/redflowers.png"));

        tree_position = generate_positions_on_terrain(14);
        champi_position = generate_positions_on_terrain(5);
        grass_position = generate_positions_on_terrain(30);
        street_lamp_position = generate_positions_on_terrain(3);

    /** *************************************************************  **/
    /** Trajectoire oiseau  **/
    /** *************************************************************  **/

    // Definition of the initial data
    //--------------------------------------//
    // Key positions
    key_positions = {{-1,1,6}, {0,1,6}, {1,3,8}, {1,6,6}, {2,-4,6}, {-1,1,6}, {2,2,7}, {2,2,6}};
    // Key times
    key_times = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f};

    // Set timer bounds
    //  You should adapt these extremal values to the type of interpolation
    timer.t_min = key_times[1];    // Start the timer at the first time of the keyframe
    timer.t_max = key_times[6];  // Ends the timer at the last time of the keyframe
    timer.t = timer.t_min;

        /** *************************************************************  **/
    /** Oiseau  **/
    /** *************************************************************  **/

    hierarchy1 = create_birds();
    hierarchy2 = create_birds();


        /** *************************************************************  **/
    /** Goutte à goutte **/
    /** *************************************************************  **/

    float const r = 0.01f; // radius of the sphere
    sphere = mesh_drawable( mesh_primitive_sphere(r));
    sphere.texture = opengl_texture_to_gpu(image_load_png("assets/water.png"));

        /** *************************************************************  **/
        /** Flocons de neige**/
        /** *************************************************************  **/

        float const rayon = 0.02f; // radius of the sphere
        snow = mesh_drawable( mesh_primitive_sphere(rayon));
        snow.shading.color = {1.0f,1.0f,1.0f};

        /** *************************************************************  **/
    /** Boules lumineuses  **/
    /** *************************************************************  **/

    // Load a new custom shader that take into account spotlights (note the new shader file in shader/ directory)
    //GLuint const shader_mesh = opengl_create_shader_program(read_text_file("shader/mesh_lights.vert.glsl"),read_text_file("shader/mesh_lights.frag.glsl"));

    //initialize the meshes

    mesh sphere_spotlight_mesh = mesh_primitive_sphere(0.05f);
    sphere_spotlight_mesh.flip_connectivity();
    sphere_spotlight = mesh_drawable( sphere_spotlight_mesh );

    /** *************************************************************  **/

}


void display_scene()
{

        draw(terrain, scene);
        // Update the current time
        float const dt = timer.update();
        float t = timer.t;


        /** *************************************************************  **/
    /** Boules lumineuses  **/
    /** *************************************************************  **/


    // set the values for the spotlights (possibly varying in time)
        for (size_t k = 0; k < street_lamp_position.size(); ++k)
        {
            scene.spotlight_color[k] = {0.8, 0.8, 0.8};
            scene.spotlight_position[k] = {street_lamp_position[k].x, street_lamp_position[k].y, street_lamp_position[k].z + 1.1f};
        }
        //scene.spotlight_color[street_lamp_position.size()-1] = {std::sin(3.14f*(t-0.8f)), std::sin(3.14f*(t-0.15f)), std::sin(3.14f*(t-0.45f))};
        //scene.spotlight_position[street_lamp_position.size()-1] = {street_lamp_position[street_lamp_position.size()-1].x, street_lamp_position[street_lamp_position.size()-1].y, street_lamp_position[street_lamp_position.size()-1].z + 1.1f};

    // display the spotlights as small spheres
    for (size_t k = 0; k < scene.spotlight_position.size(); ++k)
    {
            sphere_spotlight.transform.translate = scene.spotlight_position[k];
            sphere_spotlight.shading.color = scene.spotlight_color[k];
            draw(sphere_spotlight, scene);
    }

    draw(moon,scene);

        /** *************************************************************  **/
    /** Arbres, champignons, fontaine et lampadaires  **/
    /** *************************************************************  **/

        for (vec3 pi : tree_position){
            int a = 0;

            for (vec3 p : tree_position){
                float d = norm(pi-p);
                if (d<0.2f) a++;
            }
            if(a==1){
            trunk2.transform.rotate = rotation(vec3{1,0,0}, 3.14f/2);
            branches.transform.rotate = rotation(vec3{1,0,0}, 3.14f/2);
            foliage2.transform.rotate = rotation(vec3{1,0,0}, 3.14f/2);

            trunk2.transform.translate = pi;
            branches.transform.translate = pi;
            foliage2.transform.translate = pi;

            draw(trunk2, scene);
            draw(branches, scene);
            }
        }

        for (vec3 pi : champi_position){
            trunk2.transform.rotate = rotation(vec3{1,0,0}, 3.14f/2);
            branches.transform.rotate = rotation(vec3{1,0,0}, 3.14f/2);
            foliage2.transform.rotate = rotation(vec3{1,0,0}, 3.14f/2);

            trunk2.transform.translate = pi;
            branches.transform.translate = pi;
            foliage2.transform.translate = pi;

            //draw(trunk2, scene);
            //draw(branches, scene);
            draw(rock, scene);
        }

        for (vec3 pi : street_lamp_position){
            street_lamp.transform.translate = pi;
            tore.transform.translate = {pi.x, pi.y, pi.z + 1.1f};
            draw(street_lamp, scene);
            draw(tore, scene);
        }

        //Draw fontaine
        fontaine.transform.translate = {-2.5,1.0f, 0.2};
        draw(fontaine,scene);

        // Sanity check
        assert_vcl( key_times.size()==key_positions.size(), "key_time and key_positions should have the same size");

        if( t<timer.t_min+0.1f ) // clear trajectory when the timer restart
        trajectory.clear();


        // Compute the interpolated position
        /*if (t > key_times[5]){

            vec3 intermediaire = 2*key_positions[6]-key_positions[5];
            key_positions[0] = key_positions[5];
            key_positions[1] = key_positions[6];
            key_positions[2] = intermediaire;
            float u = rand_interval(0.1);
            float v = rand_interval(0,1);
            key_positions[3] = vec3(evaluate_terrain(u,v).x, evaluate_terrain(u,v).y, evaluate_terrain(u,v).z+
                                    7.5f + rand_interval(0,2));
        }
        else if (t < key_times[2]){

            float u4 = rand_interval(0,1);
            float v4 = rand_interval(0,1);
            vec3 pos4 = evaluate_terrain(u4,v4);
            key_positions[4] = vec3(pos4.x, pos4.y, pos4.z + 7.5f + rand_interval(0,2));

            u4 = rand_interval(0.1);
            v4 = rand_interval(0,1);
            pos4 = evaluate_terrain(u4,v4);
            key_positions[5] = vec3(pos4.x, pos4.y, pos4.z + 7.5f + rand_interval(0,2));

            u4 = rand_interval(0.1);
            v4 = rand_interval(0,1);
            pos4 = evaluate_terrain(u4,v4);
            key_positions[6] = vec3(pos4.x, pos4.y, pos4.z + 7.5f + rand_interval(0,2));

            key_positions[7] = key_positions[6];
        }*/
        vec3 const p = interpolation(t, key_positions, key_times);
        //Find the direction of trajectory
        float const ankl = direction(t, key_positions, key_times, dir);
        //dir = new_direction(t, key_positions, key_times);

        /** *************************************************************  **/
    /** Compute the (animated) transformations applied to the elements **/
    /** *************************************************************  **/

        // Bird trajectory
        hierarchy1["body"].transform.translate = p;
        hierarchy1["body"].transform.rotate = rotation({0,0,1}, ankl);

        // Rotation of wings
   hierarchy1["shoulder_left"].transform.rotate = rotation({0,0.7,0}, 0.8*std::sin(-5.5*3.14f*(t-0.15f)) );
   hierarchy1["arm_left"].transform.rotate = rotation({0,1,0}, 0.8*std::sin(-5.5*3.14f*(t-0.15f)) );
   hierarchy1["shoulder_right"].transform.rotate = rotation({0,0.7,0}, 0.8*std::sin(5.5*3.14f*(t-0.15f)));
   hierarchy1["arm_right"].transform.rotate = rotation({0,1,0}, std::sin(5.5*3.14f*(t-0.6f)) );

   hierarchy2["shoulder_left"].transform.rotate = rotation({0,0.7,0}, 0.8*std::sin(-5.5*3.14f*(t-0.15f)) );
   hierarchy2["arm_left"].transform.rotate = rotation({0,1,0}, 0.8*std::sin(-5.5*3.14f*(t-0.15f)) );
   hierarchy2["shoulder_right"].transform.rotate = rotation({0,0.7,0}, 0.8*std::sin(5.5*3.14f*(t-0.15f)));
   hierarchy2["arm_right"].transform.rotate = rotation({0,1,0}, std::sin(5.5*3.14f*(t-0.6f)) );

   // update the global coordinates
        hierarchy1.update_local_to_global_coordinates();
        hierarchy2.update_local_to_global_coordinates();

        // display the hierarchy
        if(user.gui.display_surface)
                draw(hierarchy1, scene);
        if(user.gui.display_wireframe)
                draw_wireframe(hierarchy1, scene);

        /** *************************************************************  **/
    /** Goutte à goutte  **/
    /** *************************************************************  **/

        if (t<timer.t_min+0.1f) {
                particles.push_back({vec3({-2.0f,2.0f, 1.5}),vec3(0,0,0)});
        }

        // Evolve position of particles
    const vec3 g = {0.0f,0.0f,-9.81f};
    for(particle_structure& particle : particles)
    {
        const float m = 0.01f; // particle mass

        vec3& p = particle.p;
        vec3& v = particle.v;

        const vec3 F = m*g;

        // Numerical integration
        if (p[2] < evaluate_terrain((p[0]/20)+0.5f, (p[1]/20)+0.5f).z + 0.05f){
            v = vec3(0.9*v.x, 0.9*v.y, - 0.7*v.z);
            //vec3 u = shader_mesh.fragment.normal;
            p[2] = evaluate_terrain((p[0]/20)+0.5f, (p[1]/20)+0.5f).z +0.05f;
        }
        else {
            v = v + dt*F/m;
            p = p + dt*v;
        }
    }

        // Remove particles that are too low
    for(auto it = particles.begin(); it!=particles.end(); ){
        if( it->p.z < -3)
            it = particles.erase(it);
                if(it!=particles.end())
                        ++it;
         if (it -> p.y == -6 && p.z<0){
             it = particles.erase(it);
                 if(it!=particles.end())
                         ++it;
         }
        }

        // Display particles
    for(particle_structure& particle : particles)
    {
        sphere.transform.translate = particle.p;
        draw(sphere, scene);
    }

        /** *************************************************************  **/
    /** Flocons de neige  **/
    /** *************************************************************  **/

        if (t<timer.t_max) {
                    // Initial random velocity (x,y) components are uniformly distributed along a circle.
                    const float alpha = rand_interval(0,2*pi);
                    const float theta = rand_interval(0,2*pi);
                    const vec3 v0 = vec3( std::sin(alpha)*1.0f, std::cos(alpha)*1.0f, -0.5f);
                    const float range = rand_interval(0,11.0);
                    const vec3 p0 = vec3(0.5f + range*std::cos(theta)*1.0f, 0.5f + range*std::sin(theta)*1.0f, 10.0f);
                    neiges.push_back({p0,v0});
            }

    for(particle_structure& particle : neiges)
        {
            vec3& p = particle.p;
            vec3& v = particle.v;

            const vec3 a = vec3(0.0f, 0.0f, -0.1f);

            //Neige qui tombe
            v = v + dt*(a+10*cross(a,v));
            //On considère une accélération vers le bas (pesanteur) avec une trajectoire hélicoïdale,
            p = p + dt*v;

        }
            // Remove particles that are too low
        for(auto it = neiges.begin(); it!=neiges.end(); ){
            if( it->p.x > 10 || it->p.x < -10 || it->p.y > 10 || it->p.y < -10 || it->p.z < evaluate_terrain(p.x,p.y).z-0.05f )
                it = neiges.erase(it);
                    if(it!=particles.end())
                            ++it;
            }
            // Display particles
        for(particle_structure& particle : neiges)
        {
            snow.transform.translate = particle.p;
            draw(snow, scene);
        }


        /** *************************************************************  **/
    /** Fontaine  **/
    /** *************************************************************  **/

    scene.t = timer.t; // send the current time to the shader as a uniform parameter
    draw(grid, scene);

        /** *************************************************************  **/
    /** Touffes d'herbe  **/
    /** *************************************************************  **/


    glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDepthMask(false);
        for (vec3 pi : grass_position){
            pi = pi - vec3(0.0f,0.0f,0.15f);
            billboard_grass.transform.translate = pi;
            billboard_grass.transform.rotate = rotation();
            draw(billboard_grass, scene);
            billboard_grass.transform.rotate = rotation(vec3{0,0,1}, 3.14f/2);
            draw(billboard_grass, scene);
        }

        glDepthMask(true);
    /** *************************************************************  **/


}


void display_interface()
{
    ImGui::SliderFloat("Time", &timer.t, timer.t_min, timer.t_max);
    ImGui::SliderFloat("Time scale", &timer.scale, 0.0f, 2.0f);
    ImGui::Checkbox("Frame", &user.gui.display_frame);
    ImGui::SliderFloat("Fog falloff", &scene.fog_falloff, 0, 0.05f, "%0.5f", 2.0f);


}


void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	scene.projection = projection_perspective(50.0f*pi/180.0f, aspect, 0.1f, 100.0f);
}


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	auto& camera = scene.camera;
	if(!user.cursor_on_gui){
		if(state.mouse_click_left && !state.key_ctrl)
			scene.camera.manipulator_rotate_trackball(p0, p1);
		if(state.mouse_click_left && state.key_ctrl)
			camera.manipulator_translate_in_plane(p1-p0);
		if(state.mouse_click_right)
			camera.manipulator_scale_distance_to_center( (p1-p0).y );
	}

	user.mouse_prev = p1;
}

void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection);
        opengl_uniform(shader, "view", scene.camera.matrix_view());
        opengl_uniform(shader, "light", scene.light, false);
        opengl_uniform(shader, "time", scene.t, false); // add this parameter as uniform to the shader
        //opengl_uniform(shader, "fog_falloff", current_scene.fog_falloff);


        // Adapt the uniform values send to the shader
        int const N_spotlight = current_scene.spotlight_color.size();
        GLint const location_color    = glGetUniformLocation(shader, "spotlight_color");
        GLint const location_position = glGetUniformLocation(shader, "spotlight_position");
        glUniform3fv(location_color, N_spotlight, ptr(current_scene.spotlight_color[0]));
        glUniform3fv(location_position, N_spotlight, ptr(current_scene.spotlight_position[0]));

        /** Note: Here we use the raw OpenGL call to glUniform3fv allowing us to pass a vector of data (here an array of 5 positions and 5 colors) */

        //opengl_uniform(shader, "spotlight_falloff", current_scene.spotlight_falloff);
}



