#version 330

// add uniforms to take in the matrix
uniform mat4 comb_xform;
uniform mat4 xform;

// variables for each of the streamed attributes
in vec3 vertex_position;
in vec3 vertex_normal;
in vec3 vertex_tex;

// specify out variables to be varied to the FS
out vec3 vpos;
out vec3 vnormal;

void main(void)
{
	 vpos = (xform * vec4(vertex_position, 1.0)).xyz;
	 vnormal = (xform * vec4(vertex_normal, 0.0)).xyz;
	 gl_Position = comb_xform * xform * vec4(vertex_position, 1.0);
}
