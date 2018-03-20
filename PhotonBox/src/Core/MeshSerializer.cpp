#include <fstream>
#include <iostream>

#include "../Resources/Mesh.h"
#include "../Resources/Vertex.h"
#include "MeshSerializer.h"

void MeshSerializer::write(const std::string & pathName, Mesh* mesh)
{

	int indicesSize = mesh->indices.size();
	int verticesSize = mesh->vertices.size();
	float boundingSphereRadius = mesh->boundingSphereRadius;
	Vector3f min = mesh->min;
	Vector3f max = mesh->max;

	std::ofstream myfile;
	myfile.open(pathName, std::ios::out | std::ios::binary);

	if (myfile.is_open())
	{
		myfile.seekp(0);
		// write vertices size
		myfile.write((char*)&verticesSize, sizeof(int));
		// write indices size
		myfile.write((char*)&indicesSize, sizeof(int));
		// write radius
		myfile.write((char*)&boundingSphereRadius, sizeof(float));
		// write min
		myfile.write((char*)&min, sizeof(Vector3f));
		// write max
		myfile.write((char*)&max, sizeof(Vector3f));
		// write indices
		myfile.write((char*)&mesh->vertices[0], sizeof(Vertex) * verticesSize);
		// write vertices
		myfile.write((char*)&mesh->indices[0], sizeof(unsigned int) * indicesSize);

		myfile.close();
	}
	else
	{
		std::cout << "Unable to write to file" << std::endl;
	}
}

Mesh * MeshSerializer::read(const std::string & pathName)
{
	Mesh* mesh = new Mesh();

	int indicesSize;
	int verticesSize;
	float boundingSphereRadius;
	Vector3f min;
	Vector3f max;


	std::ifstream myfile;
	myfile.open(pathName, std::ios::in | std::ios::binary);

	if (myfile.is_open())
	{
		myfile.seekg(0);

		// read vertices size
		myfile.read((char*)&verticesSize, sizeof(int));
		// read indices size
		myfile.read((char*)&indicesSize, sizeof(int));
		// read radius
		myfile.read((char*)&boundingSphereRadius, sizeof(float));
		// read min
		myfile.read((char*)&min, sizeof(Vector3f));
		// read max
		myfile.read((char*)&max, sizeof(Vector3f));

		mesh->vertices.resize(verticesSize);
		mesh->indices.resize(indicesSize);

		// read indices
		myfile.read((char*)&(mesh->vertices[0]), sizeof(Vertex) * verticesSize);
		// read vertices
		myfile.read((char*)&(mesh->indices[0]), sizeof(unsigned int) * indicesSize);

		myfile.close();

		mesh->boundingSphereRadius = boundingSphereRadius;
		mesh->min = min;
		mesh->max = max;

		return mesh;
	}
	else
	{
		std::cout << "Unable to open file" << std::endl;
		return nullptr;
	}
}
