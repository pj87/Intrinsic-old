#pragma once

#include "GeneratedStaticMesh.h"

Intrinsic::Core::Resources::GeneratedStaticMesh Intrinsic::Core::Resources::PJGeneratedStaticMeshes::mesh;

void Intrinsic::Core::Resources::PJGeneratedStaticMeshes::GenerateMeshes() {
  mesh.name = "DUPA";
}

/*
void Intrinsic::Core::Resources::GenerateMeshes() {
  _generatedStaticMeshes.name = "PJGeneratedMesh";
  _generatedStaticMeshes.material = "default";

  _generatedStaticMeshes.positions.push_back(glm::vec3(50.0, 0.0, -50.0));
  _generatedStaticMeshes.positions.push_back(glm::vec3(-50.0, 0.0, -50.0));
  _generatedStaticMeshes.positions.push_back(glm::vec3(-50.0, 0.0, 50.0));
  _generatedStaticMeshes.positions.push_back(glm::vec3(50.0, 0.0, 50.0));

  _generatedStaticMeshes.uv0.push_back(glm::vec2(1.0, 1.0));
  _generatedStaticMeshes.uv0.push_back(glm::vec2(0.0, 1.0));
  _generatedStaticMeshes.uv0.push_back(glm::vec2(0.0, 0.0));
  _generatedStaticMeshes.uv0.push_back(glm::vec2(1.0, 0.0));

  _generatedStaticMeshes.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  _generatedStaticMeshes.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  _generatedStaticMeshes.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
  _generatedStaticMeshes.normals.push_back(glm::vec3(0.0, 1.0, 0.0));

  _generatedStaticMeshes.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  _generatedStaticMeshes.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  _generatedStaticMeshes.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));
  _generatedStaticMeshes.tangents.push_back(glm::vec3(1.0, 0.0, 0.0));

  _generatedStaticMeshes.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  _generatedStaticMeshes.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  _generatedStaticMeshes.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));
  _generatedStaticMeshes.binormals.push_back(glm::vec3(0.0, 0.0, -1.0));

  _generatedStaticMeshes.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  _generatedStaticMeshes.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  _generatedStaticMeshes.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  _generatedStaticMeshes.colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));

  _generatedStaticMeshes.indices.push_back(0);
  _generatedStaticMeshes.indices.push_back(1);
  _generatedStaticMeshes.indices.push_back(2);
  _generatedStaticMeshes.indices.push_back(0);
  _generatedStaticMeshes.indices.push_back(2);
  _generatedStaticMeshes.indices.push_back(3);
}
*/

//int main()
//{
//    return 0;
//}