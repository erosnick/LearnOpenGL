#include "Model.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

Model::Model()
	: model(glm::mat4(1.0f)),
	  position(glm::vec3(0.0f)),
	  scale(glm::vec3(1.0f))
{
	
}

void Model::initialize()
{
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &IBO);

	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
}

void Model::preDraw()
{
	glBindVertexArray(VAO);
}

void Model::loadData(const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices)
{
	vertices = inVertices;
	indices = inIndices;

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Position attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
	glEnableVertexAttribArray(0);

	// TexCoord attribute.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
}

void Model::setPosition(const glm::vec3& inPosition)
{
	position = inPosition;

	model = glm::translate(model, position);
}

void Model::setScale(const glm::vec3& inScale)
{
	scale = inScale;

	model = glm::scale(model, scale);
}

void Model::resetScale()
{
	scale = glm::vec3(1.0f);

	model = glm::scale(model, scale);
}

glm::mat4 Model::modelMatrix() const
{
	return model;
}

unsigned int Model::verticesCount() const
{
	return indices.size();
}
