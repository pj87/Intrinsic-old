#include "IntrinsicCoreDynamicMesh.h"
//#include "IntrinsicCoreEntity.h"
//#include "IntrinsicCoreDod.h"

#include "stdafx.h"

namespace
{
_INTR_INLINE Entity::EntityRef
spawnDefaultEntity(const Name& p_Name,
                   const glm::quat& p_InitialOrientation = glm::quat())
{
  Entity::EntityRef entityRef = Entity::EntityManager::createEntity(p_Name);
  {

    Components::NodeRef nodeRef =
        Components::NodeManager::createNode(entityRef);
    Components::NodeManager::attachChild(World::_rootNode, nodeRef);

    Components::CameraRef activeCamera = World::_activeCamera;
    Components::NodeRef cameraNode =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(activeCamera));

    Math::Ray worldRay = {Components::NodeManager::_worldPosition(cameraNode),
                          Components::CameraManager::_forward(activeCamera)};

    // Spawn the object close to the ground
    physx::PxRaycastHit hit;
    if (PhysicsHelper::raycast(worldRay, hit, 50.0f,
                               physx::PxQueryFlag::eSTATIC))
    {
      Components::NodeManager::_position(nodeRef) =
          Components::NodeManager::_worldPosition(cameraNode) +
          Components::CameraManager::_forward(activeCamera) * hit.distance *
              0.9f;
    }
    else
    {
      Components::NodeManager::_position(nodeRef) =
          Components::NodeManager::_worldPosition(cameraNode) +
          Components::CameraManager::_forward(activeCamera) * 10.0f;
    }

    Components::NodeManager::_orientation(nodeRef) = p_InitialOrientation;
    GameStates::Editing::_currentlySelectedEntity = entityRef;
  }

  return entityRef;
}
}

namespace Intrinsic
{
namespace Core
{
namespace DynamicMesh
{
	void addCube()
	{
          //Intrinsic::Core::Entity::EntityRef entityRef = spawnDefaultEntity(_N(Cube));
          //Intrinsic::Core::Dod::Ref compRef =
          //    addComponentToEntity(entityRef, _N(Mesh));
          
          //Intrinsic::Core::Components::MeshManager::_descMeshName(compRef) = _N(cube);

          //Intrinsic::Core::Components::NodeManager::rebuildTreeAndUpdateTransforms();
          //Intrinsic::Core::Components::MeshManager::createResources(compRef);
		  
	}
}
}
}
