#include <vector>
#include <string>
#include <glm/glm.hpp>

#pragma once

namespace Intrinsic
{
namespace Generated
{
namespace Static
{
	static void GenerateMeshes();

	typedef struct
	{
        std::string name;
		std::vector<glm::vec3> positions; 
		std::vector<glm::vec2> uv0; 
		std::vector<glm::vec3> normals; 
		std::vector<glm::vec3> tangents; 
		std::vector<glm::vec3> binormals; 
		std::vector<glm::vec4> colors; 
		std::vector<int> indices;
		std::string material;
	} GeneratedStaticMesh;

	// Singleton taken from https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
    // Factory taken from: https://stackoverflow.com/questions/5120768/how-to-implement-the-factory-method-pattern-in-c-correctly

	class MeshGenerator
        {
        public:
          static MeshGenerator& GetInstance()
          {
            static MeshGenerator instance; // Guaranteed to be destroyed.
                               // Instantiated on first use.
            return instance;
          }

		  MeshGenerator(MeshGenerator const&) = delete;
          void operator=(MeshGenerator const&) = delete;

		  void InitGeometry();
          std::vector<std::unique_ptr<GeneratedStaticMesh>>& GetMeshes();
          GeneratedStaticMesh& CreateMesh();

        private:
          MeshGenerator() {} // Constructor? (the {} brackets) are needed here.
		  std::vector<std::unique_ptr<GeneratedStaticMesh>> meshes;
        };
}
}
};