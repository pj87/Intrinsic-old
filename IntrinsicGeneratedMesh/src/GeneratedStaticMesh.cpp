#pragma once

#include "GeneratedStaticMesh.h"
#include <iostream>
#include <thread>

void AddPlane()
{
  Intrinsic::Core::Resources::GeneratedStaticMesh& mesh =
      Intrinsic::Core::Resources::MeshGenerator::GetInstance().CreateMesh();

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
}

void AddMesh1()
{
  Intrinsic::Core::Resources::GeneratedStaticMesh& mesh =
      Intrinsic::Core::Resources::MeshGenerator::GetInstance().CreateMesh();

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
}

void Intrinsic::Core::Resources::MeshGenerator::InitGeometry()
{
  GenerateMeshes();
}

std::vector<std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh>>&
Intrinsic::Core::Resources::MeshGenerator::GetMeshes()
{
  return meshes;
}

Intrinsic::Core::Resources::GeneratedStaticMesh&
Intrinsic::Core::Resources::MeshGenerator::CreateMesh() 
{
  std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh> mesh =
      std::make_unique<Intrinsic::Core::Resources::GeneratedStaticMesh>();

  meshes.push_back(std::move(mesh));

  return *meshes.back();
}

static void Intrinsic::Core::Resources::GenerateMeshes()
{ 
	AddPlane();
	AddMesh1();
}
