#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <map>

class MyView : public tygra::WindowViewDelegate
{
public:
    
    MyView();
    
    ~MyView();
    
    void setScene(const sponza::Context * scene);

private:

    void windowViewWillStart(tygra::Window * window) override;
    
    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;
    
    void windowViewRender(tygra::Window * window) override;

private:

    const sponza::Context * scene_;

	// Me from here down
	GLuint shader_program_{ 0 };

	const static GLuint kNullId = 0;

	// define values for your Vertex attributes
	enum VertexAttribIndexes
	{
		kVertexPosition = 0,
		kVertexNormal = 1,
		kVertexTex = 2
	};

	// a mesh structure to hold VBO ids etc.
	struct MeshGL
	{
		int meshId{ 0 };
		GLuint position_vbo{ 0 };
		GLuint element_vbo{ 0 };
		GLuint vao{ 0 };
		int element_count{ 0 };
		GLuint normal_vbo{ 0 };
		GLuint texture_vbo{ 0 };
	};

	// a container of these mesh e.g.
	std::vector<MeshGL> m_meshVector;

	struct MaterialData
	{
		GLuint id{ 0 };
		std::string fileName;
		bool hasDiffuse{ false };
		bool hadSpecular{ false };
	};

	std::vector<std::string> m_texVector;
	std::map<sponza::MaterialId, MaterialData> m_texMap;

	void createBuffer(GLuint &vbo, GLenum target, GLsizeiptr size, const void *data);
	void bindBuffer(GLuint &vbo, GLenum target, GLuint index, GLint isize, GLsizeiptr size);

};
