#pragma once

#include "GeneratedStaticMesh.h"
#include <iostream>
#include <thread>

void AddPlane()
{
  Intrinsic::Generated::Static::GeneratedStaticMesh& mesh =
      Intrinsic::Generated::Static::MeshGenerator::GetInstance().CreateMesh();

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
  Intrinsic::Generated::Static::GeneratedStaticMesh& mesh =
      Intrinsic::Generated::Static::MeshGenerator::GetInstance().CreateMesh();

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

void AddSphere()
{
  // chyba trzeba zmienic triangle strip na triangles!!!!! 
  // TODO: Change the triagnle strip to triangles (now it is other way round VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST -> VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP) 

  Intrinsic::Generated::Static::GeneratedStaticMesh& mesh =
      Intrinsic::Generated::Static::MeshGenerator::GetInstance().CreateMesh();

  mesh.name = "PJGeneratedSphere";
  mesh.material = "default";

  const unsigned int X_SEGMENTS = 64;
  const unsigned int Y_SEGMENTS = 64;
  const float PI = 3.14159265359;
  for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
  {
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
      float xSegment = (float)x / (float)X_SEGMENTS;
      float ySegment = (float)y / (float)Y_SEGMENTS;
      float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
      float yPos = std::cos(ySegment * PI);
      float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

      mesh.positions.push_back(glm::vec3(xPos, yPos, zPos));
      mesh.uv0.push_back(glm::vec2(xSegment, ySegment));
      mesh.normals.push_back(glm::vec3(xPos, yPos, zPos));
      mesh.colors.push_back(glm::vec4(xPos, yPos, zPos, 1.0));
      mesh.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
      mesh.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
    }
  }

  bool oddRow = false;
  for (int y = 0; y < Y_SEGMENTS; ++y)
  {
    if (!oddRow) // even rows: y == 0, y == 2; and so on
    {
      for (int x = 0; x <= X_SEGMENTS; ++x)
      {
        mesh.indices.push_back(y * (X_SEGMENTS + 1) + x);
        mesh.indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
      }
    }
    else
    {
      for (int x = X_SEGMENTS; x >= 0; --x)
      {
        mesh.indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
        mesh.indices.push_back(y * (X_SEGMENTS + 1) + x);
      }
    }
    oddRow = !oddRow;
  }
}

void Intrinsic::Generated::Static::MeshGenerator::InitGeometry()
{
  GenerateMeshes();
}

std::vector<std::unique_ptr<Intrinsic::Generated::Static::GeneratedStaticMesh>>&
Intrinsic::Generated::Static::MeshGenerator::GetMeshes()
{
  return meshes;
}

Intrinsic::Generated::Static::GeneratedStaticMesh&
Intrinsic::Generated::Static::MeshGenerator::CreateMesh() 
{
  std::unique_ptr<Intrinsic::Generated::Static::GeneratedStaticMesh> mesh =
      std::make_unique<Intrinsic::Generated::Static::GeneratedStaticMesh>();

  meshes.push_back(std::move(mesh));

  return *meshes.back();
}

static void Intrinsic::Generated::Static::GenerateMeshes()
{ 
	AddPlane();
	AddMesh1();
    AddSphere();
}
