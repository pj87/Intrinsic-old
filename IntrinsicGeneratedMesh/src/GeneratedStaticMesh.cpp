#pragma once

#include "GeneratedStaticMesh.h"

std::vector<Intrinsic::Core::Resources::GeneratedStaticMesh>
    Intrinsic::Core::Resources::MeshGenerator::meshes;

std::vector<std::function<
    std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh>()>>
    Intrinsic::Core::Resources::MeshGenerator::funcs;

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

void Intrinsic::Core::Resources::MeshGenerator::GenerateMeshes()
{
  // Intrinsic::Core::Resources::MeshGenerator::meshes.resize(2);
  // AddPlane(Intrinsic::Core::Resources::MeshGenerator::meshes);
  // AddMesh1(Intrinsic::Core::Resources::MeshGenerator::meshes);

  funcs.push_back(AddMesh1);
  funcs.push_back(AddPlane);

  for (auto fun : funcs)
  {
	//std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh>
	//meshes.push_back(fun());
    std::unique_ptr<Intrinsic::Core::Resources::GeneratedStaticMesh> mesh = fun();
    //std::move(mesh);
    //meshes.push_back(std::move(mesh));
    meshes.push_back(*mesh);
  }


  // std::unique_ptr<std::unique_ptr<GeneratedStaticMesh()>> func = AddMesh1();

  //(GeneratedStaticMesh*)() mesh = AddMesh1();
  // funcs.push_back(AddMesh1);
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
