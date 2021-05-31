#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;

out struct fragment_data
{
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;
	vec3 eye;
} fragment;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;


void main()
{
	vec3 p0 = position.xyz;
	float d = sqrt((p0.x+0.5)*(p0.x+0.5)+(p0.y)*(p0.y));
	float omega = 80*d - 80*time;

	// Procedural deformation: z = 0.05*cos( (x^2+y^2)^0.5 - 5*t );	
		
	vec3 p = vec3(p0.x,p0.y,p0.z);
		
	// Compute normals after deformation
	vec3 dpdx = vec3(1, 0, 0);
	vec3 dpdy = vec3(0, 1, 0);

	
	float lim = 80/80*(time-1.44);
	if (d>lim && d<lim+2*3.14/80) {
		p.z = 0.01*cos(omega);
		dpdx.z = -20*p0.x/d*0.05*sin(omega) ;
		dpdy.z = -20*p0.y/d*0.05*sin(omega) ;
	}

	vec3 n = normalize(cross(dpdx,dpdy));

	fragment.position = vec3(model * vec4(p,1.0));
	fragment.normal   = vec3(model * vec4(n  ,0.0));
	fragment.color = color;
	fragment.uv = uv;
	fragment.eye = vec3(inverse(view)*vec4(0,0,0,1.0));

	gl_Position = projection * view * model * vec4(p, 1.0);
}
