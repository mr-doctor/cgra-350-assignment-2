#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <cgra/bone.hpp>
#include <fstream>
#include <sstream>
#include <memory.h>
#include <iterator>
#include <array>
#include <algorithm>

#include "opengl.hpp"
#include "imgui.h"

#include "cgra/matrix.hpp"
#include "cgra/wavefront.hpp"

#include "ex1.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "skeleton.hpp"
#include "printer.h"

cgra::Program Application::m_program;
cgra::Mesh Application::m_bone_mesh;
cgra::Mesh Application::m_sphere_mesh;
cgra::Mesh Application::m_arrow_x_mesh;
cgra::Mesh Application::m_arrow_y_mesh;
cgra::Mesh Application::m_arrow_z_mesh;

glm::mat4 Application::m_model;
glm::mat4 Application::m_proj;
glm::mat4 Application::m_view;

void Application::init() {

	set_shaders(CGRA_SRCDIR "/res/shaders/simple.vs.glsl", CGRA_SRCDIR "/res/shaders/simple.fs.glsl");

	m_view = glm::mat4(1);
	m_view[3] = glm::vec4(0, 0, -10, 1);
	m_program.setViewMatrix(m_view);

	glm::vec3 rotation(1.0f, 1.0f, 0.0f);
	m_rotationMatrix = glm::rotate(glm::mat4(1.0f), 45.0f, glm::vec3(rotation[0], rotation[1], rotation[2]));
	selected.m_pos = glm::vec3(-20);

	m_skeleton = Skeleton(CGRA_SRCDIR "/res/models/priman.asf");
	m_bone_mesh = loadObj(CGRA_SRCDIR "/res/models/frustrum-small.obj", -2);
	m_sphere_mesh = loadObj(CGRA_SRCDIR "/res/models/sphere.obj", -1);
	m_arrow_x_mesh = loadObj(CGRA_SRCDIR "/res/models/arrow.obj", 0);
	m_arrow_y_mesh = loadObj(CGRA_SRCDIR "/res/models/arrow.obj", 1);
	m_arrow_z_mesh = loadObj(CGRA_SRCDIR "/res/models/arrow.obj", 2);

	printer::print(m_skeleton);
}

cgra::Mesh Application::loadObj(const char *filename, int id) {
	std::ifstream obj_file(filename);
	if (!obj_file.is_open()) {
		std::cerr << "File not open\n";

		// Return a blank mesh if no file is found
		cgra::Matrix<double> vertices(0, 3);
		cgra::Matrix<unsigned int> triangles(0, 3);
		cgra::Mesh empty_mesh;
		empty_mesh.setData(vertices, triangles, glm::vec3(0));
		return empty_mesh;
	}

	glm::vec3 colour(0.0, 1.0, 0.0);
	if (id == -2) {
		colour = {0.4, 0.4, 0.4};
	} else if (id == -1) {
		colour = {0.0, 1.0, 1.0};
	} else if (id == 0) {
		colour = {1.0, 0.0, 0.0};
	} else if (id == 1) {
		colour = {0.0, 1.0, 0.0};
	} else if (id == 2) {
		colour = {0.0, 0.0, 1.0};
	}

	unsigned int num_vertices = 0;
	unsigned int num_triangles = 0;

	// Count the number of vertices and faces
	for (std::string line; std::getline(obj_file, line);) {
		std::istringstream line_parser(line);
		char c = (char) line_parser.get();
		if (c == 'v') {
			num_vertices++;
		}
		if (c == 'f') {
			num_triangles++;
		}
	}
	// Reset the file to the top
	obj_file.clear();
	obj_file.seekg(0, std::ios::beg);

	cgra::Matrix<double> vertices(num_vertices, 3);
	cgra::Matrix<unsigned int> triangles(num_triangles, 3);
	cgra::Matrix<unsigned int> lines(0, 2);

	float vertices_raw[num_vertices][3];
	unsigned int triangles_raw[num_triangles][3];

	// Copy the vertices and faces out into 2d arrays
	int index_vertices = 0;
	int index_triangles = 0;
	for (std::string line; std::getline(obj_file, line);) {
		std::istringstream line_parser(line);
		char c = (char) line_parser.get();
		if (c == 'v') {
			line_parser >> vertices_raw[index_vertices][0];
			line_parser >> vertices_raw[index_vertices][1];
			line_parser >> vertices_raw[index_vertices][2];
			index_vertices++;
		}
		if (c == 'f') {
			line_parser >> triangles_raw[index_triangles][0];
			line_parser >> triangles_raw[index_triangles][1];
			line_parser >> triangles_raw[index_triangles][2];
			index_triangles++;
		}
	}

	// Write the vertices to the matrix
	for (size_t i = 0; i < num_vertices; i++) {
		for (int j = 0; j < 3; j++) {
			vertices(i, j) = vertices_raw[i][j];
		}
	}

	// Write the triangles to the matrix
	for (size_t i = 0; i < num_triangles; i++) {
		for (int j = 0; j < 3; j++) {
			triangles(i, j) = triangles_raw[i][j] - 1;
		}
	}

	obj_file.close();
	cgra::Mesh new_mesh;

	new_mesh.setData(vertices, triangles, colour);
	// Create and return a new mesh
	return new_mesh;
}

void Application::set_shaders(const char *vertex, const char *fragment) {
	m_program = cgra::Program::load_program(vertex, fragment);
}

glm::vec3 calculate_normalised_vector(glm::vec2 mouse_pos, glm::vec2 view_port_size) {
	// Convert to model coordinates
	glm::vec3 direction{mouse_pos.x / view_port_size.x * 2 - 1,
	                    -(mouse_pos.y / view_port_size.y * 2 - 1),
	                    0};

	float length = glm::length(direction);

	// Normalise
	if (length <= 1.0) {
		direction.z = (float) sqrt(1 - length);
	} else {
		direction = glm::normalize(direction);
	}
	return direction;
}

void Application::apply_arcball(glm::vec2 current_mouse_XY) {

	glm::vec3 current_arcball_direction = calculate_normalised_vector(current_mouse_XY, m_viewportSize);
	glm::vec3 previous_arcball_direction = calculate_normalised_vector(m_mousePosition, m_viewportSize);

	float angle = previous_arcball_direction.x * current_arcball_direction.x +
	              previous_arcball_direction.y * current_arcball_direction.y +
	              previous_arcball_direction.z * current_arcball_direction.z;

	glm::vec3 normal = glm::cross(previous_arcball_direction, current_arcball_direction);
	// If there is nothing to rotate around
	if (glm::length(normal) == 0) {
		return;
	}
	// Invert the matrix for camera coordinates
	glm::mat4 arcball_rotate = glm::inverse(glm::rotate(glm::mat4(), angle / -50, normal));
	// Apply matrix
	arcball_rotate *= m_rotationMatrix;
	m_rotationMatrix = arcball_rotate;
}

void Application::drawScene() {
	// Calculate the aspect ratio of the viewport;
	// width / height
	float aspectRatio = m_viewportSize.x / m_viewportSize.y;
	// Calculate the projection matrix with a field-of-view of 45 degrees
	m_proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

	// Set the projection matrix
	m_program.setProjectionMatrix(m_proj);
//	glm::mat4 model_transform = m_rotationMatrix * glm::mat4(1.0f);

	/************************************************************
	 *                                                          *
	 * Use `m_translation`, `m_scale`, and `m_rotationMatrix`   *
	 * to create the  `modelTransform` matrix.                  *
	 * The following glm functions will be useful:              *
	 *    `glm::translate`                                      *
	 *    `glm::scale`                                          *
	 ************************************************************/


	m_skeleton.renderSkeleton(m_translation, glm::vec3(m_scale), m_rotationMatrix);

	// Draw the mesh
//	draw(m_bone_mesh, glm::vec3(0, 0, 0), glm::vec3(2, 2, 5), glm::mat4(1));
//	m_mesh.draw();
}

void Application::draw(cgra::Mesh mesh,
					   glm::vec3 position,
					   glm::vec3 scale,
					   glm::mat4 rotate,
					   glm::vec3 global_translation,
					   glm::vec3 global_scale,
					   glm::mat4 global_rotation) {
	glm::mat4 model_transform = glm::mat4(m_model);

	model_transform = glm::translate(model_transform, glm::vec3(0, 0, 0));

	// Scale so that translation works properly
	model_transform = glm::scale(model_transform, glm::vec3(global_scale));

	model_transform *= global_rotation;

	model_transform *= rotate;

	model_transform = glm::translate(model_transform, position);

//	model_transform *= glm::scale(model_transform, glm::vec3(1.0f / global_scale));

	model_transform = glm::scale(model_transform, scale);

	m_program.setModelMatrix(model_transform);

	mesh.draw();

}


void Application::draw(cgra::Mesh mesh,
					   glm::vec3 scale,
					   glm::mat4 rotate,
					   glm::vec3 global_translation,
					   glm::vec3 global_scale,
					   glm::mat4 global_rotation) {

	glm::mat4 model_transform = glm::mat4(m_model);

	model_transform = glm::translate(model_transform, global_translation);

	// Scale so that translation works properly
	model_transform = glm::scale(model_transform, glm::vec3(global_scale));

	model_transform *= global_rotation;

	model_transform *= rotate;

//	model_transform *= glm::scale(model_transform, glm::vec3(1.0f / global_scale));

	model_transform = glm::scale(model_transform, scale);

	m_program.setModelMatrix(model_transform);

	mesh.draw();
}

bool fooed = false;

void Application::doGUI() {
	ImGui::SetNextWindowSize(ImVec2(450, 450), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Shapes");

	// Example for rotation, use glm to create a a rotation
	// matrix from this vector
	static glm::vec3 rotation(0.0f, 0.0f, 0.0f);
	if (ImGui::InputFloat3("Point Rotation", &rotation[0])) {
		// This block is executed if the input changes

	}

	if (ImGui::Button("Kick")) {
		if (!fooed) {
			fooed = true;
			m_skeleton.m_bones[m_skeleton.findBone("rfemur")].rotation = glm::vec3(45);
		} else {
			m_skeleton.m_bones[m_skeleton.findBone("rfemur")].rotation = glm::vec3(0);
			fooed = false;
		}
	}

	ImGui::End();
}


// Input Handlers

void Application::onMouseButton(int button, int action, int) {
	if (button >= 0 && button < 3) {
		// Set the 'down' state for the appropriate mouse button
		m_mouseButtonDown[button] = action == GLFW_PRESS;
	}
}

glm::vec3 Application::screen_to_world_coord(double mouse_x, double mouse_y) {
	mouse_y = m_viewportSize.y - mouse_y;
	glm::vec4 viewport(0, 0, m_viewportSize.x, m_viewportSize.y);
	// converts screen points to in-world coordinates using glm and the three matrices
	glm::vec3 worldPos = glm::unProject(glm::vec3(mouse_x, mouse_y, m_depth), m_view*m_model, m_proj, viewport);
	return worldPos;
}

void Application::onCursorPos(double xpos, double ypos) {

	// Make a vec2 with the current mouse position
	glm::vec2 currentMousePosition(xpos, ypos);

	// Get the difference from the previous mouse position
//    glm::vec2 mousePositionDelta = currentMousePosition - m_mousePosition;

	if (m_mouseButtonDown[GLFW_MOUSE_BUTTON_LEFT]) {
		apply_arcball(currentMousePosition);
	} else if (m_mouseButtonDown[GLFW_MOUSE_BUTTON_MIDDLE]) {

	} else if (m_mouseButtonDown[GLFW_MOUSE_BUTTON_RIGHT] && selected.m_pos.x != -20) {
		/*if (m_depth == -1) {
			// read the depth to the last pixel on the screen
			glReadPixels(int(xpos), int(m_viewportSize.y - ypos), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &m_depth);
		}*/
		selected = m_skeleton.get_bone(glm::vec3(screen_to_world_coord(xpos, ypos)));
	}
	// Update the mouse position to the current one
	m_mousePosition = currentMousePosition;
}

void Application::onKey(int key, int scancode, int action, int mods) {
	// `(void)foo` suppresses unused variable warnings
	(void) key;
	(void) scancode;
	(void) action;
	(void) mods;
}

void Application::onScroll(double xoffset, double yoffset) {
	// `(void)foo` suppresses unused variable warnings
	(void) xoffset;
	(void) yoffset;
	if (yoffset > 0) {
		m_scale += 0.1;
	} else if (yoffset < 0) {
		m_scale = (float) std::max(m_scale - 0.1, 0.05);
	}

}
