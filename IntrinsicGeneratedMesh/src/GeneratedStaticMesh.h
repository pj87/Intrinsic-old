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
}
}
};