#include <vector>
#include <functional>
#include <string>
#include <glm/glm.hpp>

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Resources
{
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

	class MeshGenerator
	{
        public: 
			static std::vector<GeneratedStaticMesh> meshes; 

			static std::vector<
				std::function<
					std::unique_ptr<
						Intrinsic::Core::Resources::GeneratedStaticMesh>()>>
							funcs;

			static void GenerateMeshes();
	};

	// Taken from https://stackoverflow.com/questions/1008019/c-singleton-design-pattern

	class MeshGenerator1
        {
        public:
          static MeshGenerator1& GetInstance()
          {
            static MeshGenerator1 instance; // Guaranteed to be destroyed.
                               // Instantiated on first use.
            return instance;
          }

		  MeshGenerator1(MeshGenerator1 const&) = delete;
          void operator=(MeshGenerator1 const&) = delete;

		  void InitGeometry();
          void AddGenerator(std::function<std::unique_ptr<GeneratedStaticMesh>()>& p_Generator);
          std::vector<GeneratedStaticMesh>& GetMeshes();

        private:
          MeshGenerator1() {} // Constructor? (the {} brackets) are needed here.

		  std::vector<std::unique_ptr<GeneratedStaticMesh>> meshes;
          std::vector<std::function<std::unique_ptr<GeneratedStaticMesh>()>> funcs;
        };
}
}
};