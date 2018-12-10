#include "MyView.hpp"
#include <sponza/sponza.hpp>
#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
//#include <cassert>

MyView::MyView()
{
}

MyView::~MyView() {
}

void MyView::createBuffer(GLuint &vbo, GLenum target, GLsizeiptr size, const void *data)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(target, vbo);
	glBufferData(target, size, data, GL_STATIC_DRAW);
	glBindBuffer(target, kNullId);
}

void MyView::bindBuffer(GLuint &vbo, GLenum target, GLuint index, GLint isize, GLsizeiptr size)
{
	glBindBuffer(target, vbo);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, isize, GL_FLOAT, GL_FALSE, size, TGL_BUFFER_OFFSET(0));
}

void MyView::setScene(const sponza::Context * scene)
{
    scene_ = scene;
}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(scene_ != nullptr);

	GLint compile_status = GL_FALSE;

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertex_shader_string
		= tygra::createStringFromFile("resource:///sponza_vs.glsl");
	const char * vertex_shader_code = vertex_shader_string.c_str();
	glShaderSource(vertex_shader, 1,
		(const GLchar **)&vertex_shader_code, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(vertex_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragment_shader_string
		= tygra::createStringFromFile("resource:///sponza_fs.glsl");
	const char * fragment_shader_code = fragment_shader_string.c_str();
	glShaderSource(fragment_shader, 1,
		(const GLchar **)&fragment_shader_code, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(fragment_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	// Create shader program & shader in variables
	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader);

	// glBindAttribLocation for all shader streamed IN variables
	glBindAttribLocation(shader_program_, kVertexPosition, "vertex_position");
	glDeleteShader(vertex_shader);

	glAttachShader(shader_program_, fragment_shader);
	glDeleteShader(fragment_shader);
	glLinkProgram(shader_program_);

	GLint link_status = GL_FALSE;
	glGetProgramiv(shader_program_, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(shader_program_, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	/*
		The framework provides a builder class that allows access to all the mesh data	
	*/

	sponza::GeometryBuilder builder;
	const auto& source_meshes = builder.getAllMeshes();

	// We can loop through each mesh in the scene
	for each (const sponza::Mesh& source in source_meshes)
	{
		// Each mesh has an id that you will need to remember for later use
		// obained by calling source.getId()
		MeshGL mesh;
		mesh.meshId = source.getId();

		// To access the actual mesh raw data we can get the array e.g.
		const auto& positions = source.getPositionArray();
		// you also need to get the normals, elements and texture coordinates in a similar way
		const auto& normals = source.getNormalArray();
		const auto& elements = source.getElementArray();
		const auto& textures = source.getTextureCoordinateArray();

		// Create VBOs for position, normals, elements and texture coordinates
		createBuffer(mesh.position_vbo, GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data());
		createBuffer(mesh.element_vbo, GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), elements.data());
		createBuffer(mesh.normal_vbo, GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data());
		createBuffer(mesh.texture_vbo, GL_ARRAY_BUFFER, textures.size() * sizeof(glm::vec2), textures.data());

		// Create a VAO to wrap all the VBOs
		glGenVertexArrays(1, &mesh.vao);
		glBindVertexArray(mesh.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.element_vbo);

		bindBuffer(mesh.position_vbo, GL_ARRAY_BUFFER, kVertexPosition, 3, sizeof(glm::vec3));
		bindBuffer(mesh.normal_vbo, GL_ARRAY_BUFFER, kVertexNormal, 3, sizeof(glm::vec3));
		bindBuffer(mesh.texture_vbo, GL_ARRAY_BUFFER, kVertexTex, 2, sizeof(glm::vec2));
						
		mesh.element_count = elements.size();
		glBindVertexArray(kNullId);

		// store in a mesh structure and add to a container for later use
		m_meshVector.push_back(mesh);
	}

}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
}

void MyView::windowViewRender(tygra::Window * window)
{
	assert(scene_ != nullptr);

	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear buffers from previous frame
	glClearColor(0.f, 0.f, 0.25f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_program_);
	 
	// Compute viewport
	GLint viewport_size[4];
	glGetIntegerv(GL_VIEWPORT, viewport_size);
	const float aspect_ratio = viewport_size[2] / (float)viewport_size[3];

	// Note: the code above is supplied for you and already works

	// Compute projection matrix
	float far_plane_dist = scene_->getCamera().getFarPlaneDistance();
	float near_plane_dist = scene_->getCamera().getNearPlaneDistance();
	float fov = scene_->getCamera().getVerticalFieldOfViewInDegrees();
	glm::mat4 projection_xform = glm::perspective(glm::radians(fov), aspect_ratio, near_plane_dist, far_plane_dist);

	// Compute view matrix
	// You can get the camera position, look at and world up from the scene e.g.
	const auto& camera_pos = (const glm::vec3&)scene_->getCamera().getPosition();
	const auto& camera_at_pos = (const glm::vec3&)scene_->getCamera().getPosition() + (const glm::vec3&)scene_->getCamera().getDirection();
	const auto& world_up = (const glm::vec3&)scene_->getUpDirection();

	// Compute camera view matrix and combine with projection matrix
	glm::mat4 view_xform = glm::lookAt(camera_pos, camera_at_pos, world_up);

	// create combined view * projection matrix and pass to shader as a uniform
	glm::mat4 comb_xform = projection_xform * view_xform;
	GLuint comb_xform_id = glGetUniformLocation(shader_program_, "comb_xform");
	glUniformMatrix4fv(comb_xform_id, 1, GL_FALSE, glm::value_ptr(comb_xform));

	// Get light data from scene via scene_->getAllLights()
	// then plug the values into the shader - you may want to leave this until you have a basic scene showing
	const auto& Lights = scene_->getAllLights();
	size_t numLights = Lights.size();
	GLuint num_lights_id = glGetUniformLocation(shader_program_, "numLights");
	glUniform1f(num_lights_id, numLights);

	for (int i = 0; i < numLights; i++)
	{
		std::string intensityString = "lights[" + std::to_string(i) + "].intensity";
		GLuint intensityId = glGetUniformLocation(shader_program_, intensityString.c_str());
		glUniform3fv(intensityId, 1, glm::value_ptr((const glm::vec3&)Lights[i].getIntensity()));

		std::string positionString = "lights[" + std::to_string(i) + "].position";
		GLuint positionId = glGetUniformLocation(shader_program_, positionString.c_str());
		glUniform3fv(positionId, 1, glm::value_ptr((const glm::vec3&)Lights[i].getPosition()));

		std::string rangeString = "lights[" + std::to_string(i) + "].range";
		GLuint rangeId = glGetUniformLocation(shader_program_, rangeString.c_str());
		glUniform1f(rangeId, Lights[i].getRange());
	}	

	glm::vec3 ambientIntensity = (glm::vec3&)scene_->getAmbientLightIntensity();
	GLuint ambientIntensityId = glGetUniformLocation(shader_program_, "ambient_intensity");
	glUniform3fv(ambientIntensityId, 1, glm::value_ptr(ambientIntensity));

	GLuint camPosId = glGetUniformLocation(shader_program_, "camPos");
	glUniform3fv(camPosId, 1, glm::value_ptr(camera_pos));

	const auto& all_materials = scene_->getAllMaterials();
	for (const auto& mat : all_materials)
	{
		if(std::find(m_texVector.begin(), m_texVector.end(), mat.getDiffuseTexture()) == m_texVector.end())
			m_texVector.push_back(mat.getDiffuseTexture());
		if (std::find(m_texVector.begin(), m_texVector.end(), mat.getSpecularTexture()) == m_texVector.end())
			m_texVector.push_back(mat.getSpecularTexture());
	}

	// Render each mesh
	// Loop through your mesh container e.g.
	for (const auto& mesh : m_meshVector)
	{
		// Each mesh can be repeated in the scene so we need to ask the scene for all instances of the mesh
		// and render each instance with its own model matrix
		// To get the instances we need to use the meshId we stored earlier e.g.
		const auto& instances = scene_->getInstancesByMeshId(mesh.meshId);
		// then loop through all instances
			// for each instance you can call getTransformationMatrix 
			// this then needs passing to the shader as a uniform
		for (auto i : instances)
		{
			glm::mat4 xform = (glm::mat4x3&)scene_->getInstanceById(i).getTransformationMatrix();
			GLuint xform_id = glGetUniformLocation(shader_program_, "xform");
			glUniformMatrix4fv(xform_id, 1, GL_FALSE, glm::value_ptr(xform));

			// Materials
			// Each instance of the mesh has its own material accessed like so:
			// Get material for this instance
			const auto& material_id = scene_->getInstanceById(i).getMaterialId();
			const auto& material = scene_->getMaterialById(material_id);
			// You can then get the material colours from material.XX - you need to pass these to the shader
			glm::vec3 ambient_colour = (glm::vec3&)material.getAmbientColour();
			GLuint ambient_colour_id = glGetUniformLocation(shader_program_, "ambient_colour");
			glUniform3fv(ambient_colour_id, 1, glm::value_ptr(ambient_colour));

			glm::vec3 diffuse_colour = (glm::vec3&)material.getDiffuseColour();
			GLuint diffuse_colour_id = glGetUniformLocation(shader_program_, "diffuse_colour");
			glUniform3fv(diffuse_colour_id, 1, glm::value_ptr(diffuse_colour));

			glm::vec3 spec_colour = (glm::vec3&)material.getSpecularColour();
			GLuint spec_colour_id = glGetUniformLocation(shader_program_, "spec_colour");
			glUniform3fv(spec_colour_id, 1, glm::value_ptr(spec_colour));

			float shininess = material.getShininess();
			GLuint shininess_id = glGetUniformLocation(shader_program_, "shininess");
			glUniform1f(shininess_id, shininess);

			// Finally you render the mesh e.g.
			glBindVertexArray(mesh.vao);
			glDrawElements(GL_TRIANGLES, mesh.element_count, GL_UNSIGNED_INT, 0);
		}
					
	}
}
