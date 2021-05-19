#include "vcl/vcl.hpp"
#include <iostream>
#include "terrain.hpp"
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
        float fog_falloff = 0.005f;
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

mesh_drawable billboard_flower;
mesh_drawable terrain;
mesh_drawable trunk;
mesh_drawable street_lamp;
mesh_drawable tore;
mesh_drawable foliage;
mesh_drawable champi;
std::vector<vcl::vec3> tree_position;
std::vector<vcl::vec3> champi_position;
std::vector<vcl::vec3> street_lamp_position;

mesh_drawable sphere_current;    // sphere used to display the interpolated value
mesh_drawable sphere_keyframe;   // sphere used to display the key positions
curve_drawable polygon_keyframe; // Display the segment between key positions
trajectory_drawable trajectory;  // Temporary storage and display of the interpolated trajectory

hierarchy_mesh_drawable hierarchy;

std::list<particle_structure> particles; // Storage of all currently active particles
mesh_drawable sphere;
mesh_drawable disc;


vec3 dir = vec3(0,-1,0);

mesh_drawable cube;
mesh_drawable sphere_spotlight;
mesh_drawable ground;

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
		
                glClearColor(0.4f, 0.7f, 0.9f, 1.0f);
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
        //GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
        GLuint const shader_mesh = opengl_create_shader_program(read_text_file("shader/mesh_lights.vert.glsl"),read_text_file("shader/mesh_lights.frag.glsl"));
	GLuint const shader_uniform_color = opengl_create_shader_program(opengl_shader_preset("single_color_vertex"), opengl_shader_preset("single_color_fragment"));
	GLuint const texture_white = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = texture_white;
	curve_drawable::default_shader = shader_uniform_color;
	segments_drawable::default_shader = shader_uniform_color;
	
	user.global_frame = mesh_drawable(mesh_primitive_frame());
	user.gui.display_frame = false;
        scene.camera.distance_to_center = 10.0f;
        scene.camera.look_at({4,3,2}, {0,0,1}, {0,0,2});


        /** *************************************************************  **/
    /** Ground  **/
    /** *************************************************************  **/

    // Create visual terrain surface
        terrain_visual = create_terrain();
        terrain = mesh_drawable(terrain_visual);

    terrain.shading.color = {0.6f,0.85f,0.5f};
    terrain.shading.phong.specular = 0.0f; // non-specular terrain material

        /** *************************************************************  **/
    /** Motionless Objects  **/
    /** *************************************************************  **/

    trunk = mesh_drawable(create_tree_trunk());
    foliage = mesh_drawable(create_tree_foliage());
    champi = mesh_drawable(create_champi());
    street_lamp = mesh_drawable(create_street_lamp());
    mesh torus = mesh_primitive_torus(0.08f, 0.02f, {0,0,0}, {0,0,1}, 20,20);
    torus.color.fill({0,0,0});
    tore = mesh_drawable(torus);

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

    tree_position = generate_positions_on_terrain(15);
    champi_position = generate_positions_on_terrain(250);
    street_lamp_position = generate_positions_on_terrain(4);
        /** *************************************************************  **/
    /** Trajectoire oiseau  **/
    /** *************************************************************  **/

    // Definition of the initial data
    //--------------------------------------//
// Key positions
    key_positions = { {-1,1,3}, {0,1,3}, {1,1,3}, {1,2,3}, {2,2,3}, {2,2,3}, {2,0,3}, {1.5,-1,3}, {1.5,-1,3}, {1,-1,3}, {0,-0.5,3}, {-1,-0.5,3}, {0,1,3}, {0,1,3},};
    // Key times
    key_times = {0.0f, 1.0f, 2.0f, 2.5f, 3.0f, 3.5f, 3.75f, 4.5f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 9.9f};

    // Set timer bounds
    //  You should adapt these extremal values to the type of interpolation
    size_t const N = key_times.size();
    timer.t_min = key_times[1];    // Start the timer at the first time of the keyframe
    timer.t_max = key_times[N-2];  // Ends the timer at the last time of the keyframe
    timer.t = timer.t_min;


    // Initialize drawable structures
    sphere_keyframe = mesh_drawable( mesh_primitive_sphere(0.05f) );
    sphere_current  = mesh_drawable( mesh_primitive_sphere(0.06f) );
    sphere_keyframe.shading.color = {1,0,1};
    sphere_current.shading.color  = {1,1,0};

    polygon_keyframe = curve_drawable( key_positions );
    polygon_keyframe.color = {0,0,0};

        /** *************************************************************  **/
    /** Oiseau  **/
    /** *************************************************************  **/

    // Definition of the elements of the hierarchy
    // ------------------------------------------- //

    float const radius_head = 0.1f;
    float const radius_eye = 0.02f;
    float const radius_beak = 0.04f;

    vec3 const shape = {0.14f,0.27f,0.13f};

    // The geometry of the body is a sphere
    mesh_drawable body = mesh_drawable( mesh_primitive_ellipsoid(shape, {0,0,0}, 40, 40));

        // Geometry of the eyes: black spheres
    mesh_drawable eye = mesh_drawable(mesh_primitive_sphere(radius_eye, {0,0,0}, 20, 20));
    eye.shading.color = {0,0,0};

        //Geometry of the head : white sphere
    mesh_drawable head = mesh_drawable(mesh_primitive_sphere(radius_head, {0,0,0}, 40, 40));

        //Geometry of the beak
    mesh_drawable beak = mesh_drawable(mesh_primitive_cone(radius_beak, 0.11f, {0,0,0} , vec3(0,1,0),true, 40, 40));
    beak.shading.color = {202,20,0};

    //position shoulder_left
    vec3 const pt10 = vec3(0.05,-0.15,0);
    vec3 const pt11 = vec3(0.05,0.15,0);
    vec3 const pt12 = vec3(0.55,0.08,0);
    vec3 const pt13 = vec3(0.55,-0.08,0);
    //position arm_left
    vec3 const pt01 = vec3(0.7,-0.08,0);
    vec3 const pt1 = vec3(0.7,-0.081,0);

    //position shoulder_right
    vec3 const pt20 = vec3(-0.05,-0.15,0);
    vec3 const pt21 = vec3(-0.05,0.15,0);
    vec3 const pt22 = vec3(-0.55,0.08,0);
    vec3 const pt23 = vec3(-0.55,-0.08,0);
    //position arm_right
    vec3 const pt02 = vec3(-0.7,-0.08,0);
    vec3 const pt2 = vec3(-0.7,-0.081,0);


        // Shoulder part and arm are displayed as cylinder
    mesh_drawable shoulder_left = mesh_drawable(mesh_primitive_quadrangle(pt10,pt11,pt12,pt13));
    mesh_drawable arm_left = mesh_drawable(mesh_primitive_quadrangle(pt01,pt1,pt12,pt13));

    mesh_drawable shoulder_right = mesh_drawable(mesh_primitive_quadrangle(pt20,pt21,pt22,pt23));
    mesh_drawable arm_right = mesh_drawable(mesh_primitive_quadrangle(pt02,pt2,pt22,pt23));

        // An elbow displayed as a sphere
    mesh_drawable elbow = mesh_drawable(mesh_primitive_sphere(0.055f));


        // Build the hierarchy:
        // ------------------------------------------- //
    // Syntax to add element
    //   hierarchy.add(visual_element, element_name, parent_name, (opt)[translation, rotation])

        // The root of the hierarchy is the body
        hierarchy.add(body, "body");

        //Head position
        vec3 const head_position = {0,0.25f,0.08f};

        hierarchy.add(head, "head", "body" , head_position);

        // Eyes positions are set with respect to some ratio of the body
        hierarchy.add(eye, "eye_left", "body" , head_position + radius_head * vec3( 1/3.0f, 1/2.0f, 1/1.5f));
        hierarchy.add(eye, "eye_right", "body", head_position + radius_head * vec3(-1/3.0f, 1/2.0f, 1/1.5f));

        //Beak
    hierarchy.add(beak, "beak", "body", head_position + radius_head*vec3(0,1,0));

        // Set the left part of the body arm: shoulder-elbow-arm
    hierarchy.add(shoulder_left, "shoulder_left", "body", {0,0,0}); // extremity of the spherical body
    hierarchy.add(arm_left, "arm_left", "shoulder_left", {0,0,0});

        // Set the right part of the body arm: similar to the left part with a symmetry in x direction
    hierarchy.add(shoulder_right, "shoulder_right", "body", {0,0,0});
    hierarchy.add(arm_right, "arm_right", "shoulder_right");

        /** *************************************************************  **/
    /** Balles rebondissantes**/
    /** *************************************************************  **/

    float const r = 0.05f; // radius of the sphere
    sphere = mesh_drawable( mesh_primitive_sphere(r) );
    sphere.shading.color = {1.0f,0.5f,1.0f};
    disc = mesh_drawable( mesh_primitive_disc(2.0f) );
    disc.transform.translate = {0,0,-r};

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
            scene.spotlight_color[k] = {1.0f, 0.9f, 0.5f};
            scene.spotlight_position[k] = {street_lamp_position[k].x, street_lamp_position[k].y, street_lamp_position[k].z + 1.1f};

        }
    //scene.spotlight_color[0] = {1.0f, 0.0f+0.0*std::cos(3*t), 0.0f};
    //scene.spotlight_position[0] = {std::cos(t), std::sin(t), 2.5+0.2*std::cos(3*t)};

    //scene.spotlight_color[1] = {0.0f, 1.0f, 0.0f};
    //sscene.spotlight_position[1] = {std::cos(0.5*t+pi/2), std::sin(0.5*t+pi/2), 2.5+0.2*std::cos(2*t)};

    //scene.spotlight_position[2] = {0,0,1.05f};
    //scene.spotlight_color[2] = 2*(std::cos(t)+1.0f)/2.0*vec3(1,1,1);

    //scene.spotlight_position[3] = {3*std::cos(t), 3*std::sin(t), 2.5+0.2*std::cos(3*t)};
    //cene.spotlight_color[3] = vec3( (std::cos(t)+1)/2,0,1);

    //scene.spotlight_position[4] = {-3.0f,-3.0f,2.05f};
    //scene.spotlight_color[4] = {1.0f, 0.9f, 0.5f};

    // display the spotlights as small spheres
    for (size_t k = 0; k < scene.spotlight_position.size(); ++k)
    {
            sphere_spotlight.transform.translate = scene.spotlight_position[k];
            sphere_spotlight.shading.color = scene.spotlight_color[k];
            draw(sphere_spotlight, scene);
    }

        /** *************************************************************  **/
    /** Arbres, champignons et lampadaires  **/
    /** *************************************************************  **/

        for (vec3 pi : tree_position){
            int a = 0;

            for (vec3 p : tree_position){
                float d = norm(pi-p);
                if (d<0.2f) a++;
            }
            if(a==1){
            trunk.transform.translate = pi;
            foliage.transform.translate = pi;
            draw(trunk, scene);
            draw(foliage, scene);}
        }

        for (vec3 pi : street_lamp_position){
            street_lamp.transform.translate = pi;
            tore.transform.translate = {pi.x, pi.y, pi.z + 1.1f};
            draw(street_lamp, scene);
            draw(tore, scene);
        }

        for (vec3 pi : champi_position){
            champi.transform.translate = pi;
            draw(champi, scene);
        }

        billboard_flower.transform.rotate = rotation();
        draw(billboard_flower, scene);
        billboard_flower.transform.rotate = rotation(vec3{0,0,1}, 3.14f/2);
        draw(billboard_flower, scene);

        // Sanity check
        assert_vcl( key_times.size()==key_positions.size(), "key_time and key_positions should have the same size");

        if( t<timer.t_min+0.1f ) // clear trajectory when the timer restart
        trajectory.clear();


        // Compute the interpolated position
        vec3 const p = interpolation(t, key_positions, key_times);
        //Find the direction of trajectory
        float const ankl = direction(t, key_positions, key_times, dir);

        //dir = new_direction(t, key_positions, key_times);

        // Display the trajectory
        trajectory.visual.color = {1,0,0};
        trajectory.add(p, t);
        draw(trajectory, scene);

        /** *************************************************************  **/
    /** Compute the (animated) transformations applied to the elements **/
    /** *************************************************************  **/

        // Display the interpolated position
    hierarchy["body"].transform.translate = p;
    hierarchy["body"].transform.rotate = rotation({0,0,1}, ankl);

        // Rotation of the shoulder-left around the y axis
    hierarchy["shoulder_left"].transform.rotate = rotation({0,0.7,0}, 0.8*std::sin(-7*3.14f*(t-0.15f)) );
        // Rotation of the arm-left around the y axis (delayed with respect to the shoulder)
    //hierarchy["arm_left"].transform.rotate = rotation({0,1,0}, std::sin(2*3.14f*(t-0.2f)) );

    //Head nodding
    hierarchy["head"].transform.rotate = rotation({0,0,0}, 1*std::sin(-3*3.14f*(t-0.15f)) );

        // Rotation of the shoulder-right around the y axis
    hierarchy["shoulder_right"].transform.rotate = rotation({0,0.7,0}, 0.8*std::sin(7*3.14f*(t-0.15f)) );
    // Rotation of the arm-right around the y axis (delayed with respect to the shoulder)
    //hierarchy["arm_right"].transform.rotate = rotation({0,0,-1}, std::sin(2*3.14f*(t-0.6f)) );

        // update the global coordinates
        hierarchy.update_local_to_global_coordinates();

        // display the hierarchy
        if(user.gui.display_surface)
                draw(hierarchy, scene);
        if(user.gui.display_wireframe)
                draw_wireframe(hierarchy, scene);

        /** *************************************************************  **/
    /** Balles rebondissantes  **/
    /** *************************************************************  **/

        if (t<timer.t_min+0.1f) {
                vec3 const p0 = vec3(evaluate_terrain(0.5f, 0.5f).x, evaluate_terrain(0.5f, 0.5f).y,evaluate_terrain(0.5f, 0.5f).z+0.05f);

                 // Initial random velocity (x,y) components are uniformly distributed along a circle.
                const float theta = rand_interval(0,2*pi);
                const vec3 v0 = vec3( std::cos(theta), std::sin(theta), 5.0f);
                particles.push_back({p0,v0});
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
            v = vec3(0.9*v.x, 0.9*v.y, -0.8*v.z);
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
        }

        // Display particles
    for(particle_structure& particle : particles)
    {
        sphere.transform.translate = particle.p;
        draw(sphere, scene);
    }

    /** *************************************************************  **/



}


void display_interface()
{
    ImGui::SliderFloat("Time", &timer.t, timer.t_min, timer.t_max);
    ImGui::SliderFloat("Time scale", &timer.scale, 0.0f, 2.0f);
    ImGui::Checkbox("Frame", &user.gui.display_frame);
    ImGui::SliderFloat("Light falloff", &scene.spotlight_falloff, 0, 1.0f, "%0.4f", 2.0f);
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

        // Adapt the uniform values send to the shader
        int const N_spotlight = current_scene.spotlight_color.size();
        GLint const location_color    = glGetUniformLocation(shader, "spotlight_color");
        GLint const location_position = glGetUniformLocation(shader, "spotlight_position");
        glUniform3fv(location_color, N_spotlight, ptr(current_scene.spotlight_color[0]));
        glUniform3fv(location_position, N_spotlight, ptr(current_scene.spotlight_position[0]));

        /** Note: Here we use the raw OpenGL call to glUniform3fv allowing us to pass a vector of data (here an array of 5 positions and 5 colors) */

        //opengl_uniform(shader, "spotlight_falloff", current_scene.spotlight_falloff);
       // opengl_uniform(shader, "fog_falloff", current_scene.fog_falloff);
}



