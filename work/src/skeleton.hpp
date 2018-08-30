//---------------------------------------------------------------------------
//***
//*** This is an annotated version of a file from the COMP308 assignment on
//*** which we based the 2018 CGRA350 assignment 2.
//***
//***
//*** This file contains an example of the data structure that could be
//*** used for bones and skeleton.
//***
//***
//*** All annotations are on lines that start with the five characters //***
//***
//*** The COMP308 assignment was expected to be completed in legacy OpenGL;
//*** the CGRA350 assignment is expected to be completed in modern OpenGL.
//***
//*** Neil Dodgson, 26 August 2018
//***
//---------------------------------------------------------------------------
//
// Copyright (c) 2016 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#pragma once

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cgra/mesh.hpp>
#include "glm/glm.hpp"

//*** COMP308 had its own math library. CGRA350 students are
//*** expected to use GLM for their mathematics.
#include "opengl.hpp"



// Needed for Completion/Challenge
// We use bitmasking to work out the Degrees of Freedom
// To work out if a bone b has a y-axis dof, simply:
//     if (b.freedom & dof_ry) {...}
//
// To add and subtract degrees of freedom, respectively:
//     b.freedom |= dof_rx
//     b.freedom ^= dof_rx
using dof_set = unsigned int;

enum dof {
	dof_none = 0,
	dof_rx = 1,
	dof_ry = 2,
	dof_rz = 4,
	dof_root = 8 // Root has 6, 3 translation and 3 rotation
};


//*** The bone data structure includes various cgra::vec3 data types
//*** these will need to be replaced by the equivalent GLM data types.
//*** You will need to check every use of those variables (boneDir,
//*** basisRot, rotation, translation, rotation_max, rotation_min) in
//*** skeleton.cpp to ensure that they are being used correctly for the
//*** GLM equivalent and that the correct GLM functions are being used
//*** to operate on them.

// Type to represent a bone
struct bone {
	std::string name;
	int id;
	float length = 0;             // Length of the bone
	glm::vec3 boneDir;           // Direction of the bone
	glm::vec3 basisRot;          // Euler angle rotations for the bone basis
	dof_set freedom = dof_none;   // Degrees of freedom for the joint rotation
	std::vector<bone *> children; // Pointers to bone children

	// Completion and Challenge
	glm::vec3 rotation;          // Rotation of joint in the basis (degrees)

	// Challenge
	glm::vec3 translation;       // Translation (Only for the Root)
	glm::vec3 rotation_max;      // Maximum value for rotation for this joint (degrees)
	glm::vec3 rotation_min;      // Minimum value for rotation for this joint (degrees)
	glm::vec3 m_pos;
};


class Skeleton {

private:

	// Reading code
	void readASF(std::string);
	void readHeading(std::string, std::ifstream&);
	void readBone(std::ifstream&);
	void readHierarchy(std::ifstream&);

public:

	std::vector<bone> m_bones;
	Skeleton(std::string);
	void renderSkeleton(glm::vec3 global_translation,
	                    glm::vec3 global_scale,
	                    glm::mat4 global_rotation);
	void readAMC(std::string);

	// YOUR CODE GOES HERE
	// ...
	void renderBone(bone *bone, glm::vec3 position, glm::mat4 rotation, glm::vec3 global_translation, glm::vec3 global_scale, glm::mat4 global_rotation);

	bone get_bone(glm::vec3 point);

	void reset_point();

// Helper method
int findBone(std::string);
};
