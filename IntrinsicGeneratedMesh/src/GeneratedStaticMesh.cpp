#pragma once

#include "GeneratedStaticMesh.h"

std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh> AddPlane()
{
  std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh> mesh =
      std::make_unique<Intrinsic::Core::Resources::GeneratedStaticMesh>();

  mesh->name = "PJGeneratedMesh";
  mesh->material = "default";

  mesh->positions.push_back(glm::vec3(50.0, 0.0, -50.0));
  mesh->positions.push_back(glm::vec3(-50.0, 0.0, -50.0));
  mesh->positions.push_back(glm::vec3(-50.0, 0.0, 50.0));
  mesh->positions.push_back(glm::vec3(50.0, 0.0, 50.0));

  mesh->uv0.push_back(glm::vec2(1.0, 1.0));
  mesh->uv0.push_back(glm::vec2(0.0, 1.0));
  mesh->uv0.push_back(glm::vec2(0.0, 0.0));
  mesh->uv0.push_back(glm::vec2(1.0, 0.0));

  mesh->normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh->normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh->normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh->normals.push_back(glm::vec3(0.0, 1.0, 0.0));

  mesh->tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh->tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh->tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh->tangents.push_back(glm::vec3(1.0, 0.0, 0.0));

  mesh->binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh->binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh->binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh->binormals.push_back(glm::vec3(0.0, 0.0, -1.0));

  mesh->colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh->colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh->colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh->colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));

  mesh->indices.push_back(0);
  mesh->indices.push_back(1);
  mesh->indices.push_back(2);
  mesh->indices.push_back(0);
  mesh->indices.push_back(2);
  mesh->indices.push_back(3);

  return mesh;
}

std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh> AddMesh1()
{
  std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh> mesh =
      std::make_unique<Intrinsic::Core::Resources::GeneratedStaticMesh>();

  mesh->name = "PJGeneratedMesh1";
  mesh->material = "default";

  mesh->positions.push_back(glm::vec3(50.0, 0.0, -50.0));
  mesh->positions.push_back(glm::vec3(-50.0, 0.0, -50.0));
  mesh->positions.push_back(glm::vec3(-50.0, 0.0, 50.0));
  mesh->positions.push_back(glm::vec3(50.0, -50.0, 50.0));

  mesh->uv0.push_back(glm::vec2(1.0, 1.0));
  mesh->uv0.push_back(glm::vec2(0.0, 1.0));
  mesh->uv0.push_back(glm::vec2(0.0, 0.0));
  mesh->uv0.push_back(glm::vec2(1.0, 0.0));

  mesh->normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh->normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh->normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  mesh->normals.push_back(glm::vec3(0.0, 1.0, 0.0));

  mesh->tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh->tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh->tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  mesh->tangents.push_back(glm::vec3(1.0, 0.0, 0.0));

  mesh->binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh->binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh->binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  mesh->binormals.push_back(glm::vec3(0.0, 0.0, -1.0));

  mesh->colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh->colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh->colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  mesh->colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));

  mesh->indices.push_back(0);
  mesh->indices.push_back(1);
  mesh->indices.push_back(2);
  mesh->indices.push_back(0);
  mesh->indices.push_back(2);
  mesh->indices.push_back(3);

  return mesh;
}

void 
Intrinsic::Core::Resources::MeshGenerator1::InitGeometry()
{
  for (auto fun : funcs)
  {
    std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh> mesh =
        fun();

    meshes.push_back(*mesh);
  }
}

void 
Intrinsic::Core::Resources::MeshGenerator1::AddGenerator(
    std::function<std::unique_ptr<GeneratedStaticMesh>()>& p_Generator)
{
  funcs.push_back(p_Generator);
}

std::vector<Intrinsic::Core::Resources::GeneratedStaticMesh>&
Intrinsic::Core::Resources::MeshGenerator1::GetMeshes()
{
  return meshes;
}
