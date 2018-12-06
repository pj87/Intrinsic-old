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

_INTR_INLINE Dod::Ref addComponentToEntity(Entity::EntityRef p_EntityRef,
                                           const Name& p_ComponentName)
{
  Dod::Components::ComponentManagerEntry& entry =
      Application::_componentManagerMapping[p_ComponentName];

  _INTR_ASSERT(entry.createFunction);
  Dod::Ref compRef = entry.createFunction(p_EntityRef);

  if (entry.resetToDefaultFunction)
  {
    entry.resetToDefaultFunction(compRef);
  }

  return compRef;
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
	  Entity::EntityRef entityRef = spawnDefaultEntity(_N(Cube));
	  Dod::Ref compRef = addComponentToEntity(entityRef, _N(Mesh));

	  Components::MeshManager::_descMeshName(compRef) = _N(cube);
	  Components::NodeManager::rebuildTreeAndUpdateTransforms();
	  Components::MeshManager::createResources(compRef);
	}
}
}
}
