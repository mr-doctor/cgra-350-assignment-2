#pragma once

#include <cgra/bone.hpp>
#include "cgra/mesh.hpp"
#include "cgra/shader.hpp"

#include "glm/glm.hpp"
#include "skeleton.hpp"

class Application {
public:
    // The window object managed by GLFW
    GLFWwindow *m_window;

    // The shader program used for drawing
    static cgra::Program m_program;
    // The mesh data
    cgra::Mesh m_mesh;

    // The current size of the viewport
    glm::vec2 m_viewportSize;
    // The current mouse position
    glm::vec2 m_mousePosition;

    // The translation of the mesh as a vec3
    glm::vec3 m_translation;
    // The scale of the mesh
    float m_scale;

    // A 4x4 matrix representing the rotation of the
    // mesh
    glm::mat4 m_rotationMatrix;

    // Whether or not the left, middle or right buttons are down.
    bool m_mouseButtonDown[3];

    Application(GLFWwindow *win)
        : m_window(win),
          m_viewportSize(1, 1), m_mousePosition(0, 0),
          m_translation(0), m_scale(1), m_rotationMatrix(1) {
        m_mouseButtonDown[0] = false;
        m_mouseButtonDown[1] = false;
        m_mouseButtonDown[2] = false;
    }

    void setWindowSize(int width, int height) {
        m_viewportSize.x = float(width);
        m_viewportSize.y = float(height);
    }

    void init();

    void drawScene();
    void doGUI();

    void onKey(int key, int scancode, int action, int mods);

    void onMouseButton(int button, int action, int mods);

    void onCursorPos(double xpos, double ypos);

    void onScroll(double xoffset, double yoffset);

	void apply_arcball(glm::vec2 current_mouse_XY);

	int m_divisions_lon = 10;
	int m_divisions_lat = 10;
	bool m_sphere_latlong = false;

	bool m_sphere_from_cube = false;
	int m_sphere_mode = 0;

	void set_shaders(const char *vertex, const char *fragment);

	Skeleton m_skeleton = Skeleton("");

	cgra::Mesh loadObj(const char *filename, int id);

	static cgra::Mesh m_bone_mesh;
	static cgra::Mesh m_sphere_mesh;
	static cgra::Mesh m_arrow_x_mesh;
	static cgra::Mesh m_arrow_y_mesh;
	static cgra::Mesh m_arrow_z_mesh;

	static void draw_bone(cgra::Mesh mesh, glm::vec3 position, glm::vec3 scale, glm::mat4 rotate,
	                      glm::vec3 global_translation,
	                      glm::vec3 global_scale, glm::mat4 global_rotation);

	static void draw(cgra::Mesh mesh, glm::vec3 position, glm::vec3 scale, glm::mat4 rotate,
	                 glm::vec3 global_translation,
	                 glm::vec3 global_scale, glm::mat4 global_rotation);


	glm::vec3 screen_to_world_coord(double mouse_x, double mouse_y);

	static glm::mat4 m_view;
	static glm::mat4 m_model;
	static glm::mat4 m_proj;

	float m_depth = -1.0f;
	bone selected = bone();
};
