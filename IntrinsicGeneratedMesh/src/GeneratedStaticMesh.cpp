#pragma once

#include "GeneratedStaticMesh.h"

int Intrinsic::Core::Resources::MeshGenerator::meshCount = 2;


std::vector <Intrinsic::Core::Resources::GeneratedStaticMesh>
                  Intrinsic::Core::Resources::MeshGenerator::meshes;

void AddPlane(
    std::vector<Intrinsic::Core::Resources::GeneratedStaticMesh>& p_Meshes)
{
  Intrinsic::Core::Resources::GeneratedStaticMesh mesh;

  mesh.name = "PJGeneratedMesh";
  mesh.material = "default";

  mesh.positions.push_back(glm::vec3(50.0, 0.0, -50.0));
  mesh.positions.push_back(glm::vec3(-50.0, 0.0, -50.0));
  mesh.positions.push_back(glm::vec3(-50.0, 0.0, 50.0));
  mesh.positions.push_back(glm::vec3(50.0, 0.0, 50.0));

  mesh.uv0.push_back(glm::vec2(1.0, 1.0));
  mesh.uv0.push_back(glm::vec2(0.0, 1.0));
  mesh.uv0.push_back(glm::vec2(0.0, 0.0));
  mesh.uv0.push_back(glm::vec2(1.0, 0.0));

  mesh.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh.normals.push_back(glm::vec3(0.0, 1.0, 0.0));

  mesh.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));

  mesh.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));

  mesh.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));

  mesh.indices.push_back(0);
  mesh.indices.push_back(1);
  mesh.indices.push_back(2);
  mesh.indices.push_back(0);
  mesh.indices.push_back(2);
  mesh.indices.push_back(3);

  p_Meshes.push_back(mesh);
}

void AddMesh1(
    std::vector<Intrinsic::Core::Resources::GeneratedStaticMesh>& p_Meshes)
{
  Intrinsic::Core::Resources::GeneratedStaticMesh mesh;

  mesh.name = "PJGeneratedMesh1";
  mesh.material = "default";

  mesh.positions.push_back(glm::vec3(50.0, 0.0, -50.0));
  mesh.positions.push_back(glm::vec3(-50.0, 0.0, -50.0));
  mesh.positions.push_back(glm::vec3(-50.0, 0.0, 50.0));
  mesh.positions.push_back(glm::vec3(50.0, -50.0, 50.0));

  mesh.uv0.push_back(glm::vec2(1.0, 1.0));
  mesh.uv0.push_back(glm::vec2(0.0, 1.0));
  mesh.uv0.push_back(glm::vec2(0.0, 0.0));
  mesh.uv0.push_back(glm::vec2(1.0, 0.0));

  mesh.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh.normals.push_back(glm::vec3(0.0, 1.0, 0.0));

  mesh.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));

  mesh.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));

  mesh.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));

  mesh.indices.push_back(0);
  mesh.indices.push_back(1);
  mesh.indices.push_back(2);
  mesh.indices.push_back(0);
  mesh.indices.push_back(2);
  mesh.indices.push_back(3);

  p_Meshes.push_back(mesh);
}

void Intrinsic::Core::Resources::MeshGenerator::GenerateMeshes()
{
  //Intrinsic::Core::Resources::MeshGenerator::meshes.resize(2);
  AddPlane(Intrinsic::Core::Resources::MeshGenerator::meshes);
  AddMesh1(Intrinsic::Core::Resources::MeshGenerator::meshes);
}
