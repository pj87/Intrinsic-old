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

	class S
        {
        public:
          static S& getInstance()
          {
            static S instance; // Guaranteed to be destroyed.
                               // Instantiated on first use.
            return instance;
          }

        private:
          S() {} // Constructor? (the {} brackets) are needed here.

          // C++ 11
          // =======
          // We can use the better technique of deleting the methods
          // we don't want.
        public:
          S(S const&) = delete;
          void operator=(S const&) = delete;

          // Note: Scott Meyers mentions in his Effective Modern
          //       C++ book, that deleted functions should generally
          //       be public as it results in better error messages
          //       due to the compilers behavior to check accessibility
          //       before deleted status
        };
}
}
};