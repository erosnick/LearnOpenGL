#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>

// ¶¥µã
struct Vertex
{
	Vertex(float inX, float inY, float inZ, float inU, float inV)
	{
		x = inX;
		y = inY;
		z = inZ;
		u = inU;
		v = inV;
	}

	float x;
	float y;
	float z;
	float u;
	float v;
};

struct VertexNormal
{
	VertexNormal(float inX, float inY, float inZ, float inNormalX, float inNormalY, float inNormalZ)
	{
		x = inX;
		y = inY;
		z = inZ;
		normalX = inNormalX;
		normalY = inNormalY;
		normalZ = inNormalZ;
	}

	float x;
	float y;
	float z;
	float normalX;
	float normalY;
	float normalZ;
};


class Model
{
public:

	Model();

	void initialize();

	void preDraw();

	void loadData(const std::vector<VertexNormal>& inVertices);
	void loadData(const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices = std::vector<unsigned int>());

	void setPosition(const glm::vec3& inPosition);

	void setScale(const glm::vec3& inScale);

	void resetScale();
	
	glm::mat4 modelMatrix() const;

	unsigned int verticesCount() const;

private:

	unsigned int VAO;
	unsigned int VBO;
	unsigned int IBO;

	std::vector<Vertex> vertices;
	std::vector<VertexNormal> normalVertices;
	std::vector<unsigned int> indices;

	glm::mat4 model;
	glm::vec3 position;
	glm::vec3 scale;
};