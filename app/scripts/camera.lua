-- Explosion entire period

iTime = 0.0
initialized = 0
spawnCounter = 0
modulus = 2
angle = 0.0

function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, iTime * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef);
  local size = nodeComponent.getSize(nodeRef)
  nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  iTime = iTime + p_DeltaT
  
  position.y = position.y + iTime * 0.1
  nodeComponent.setPosition(nodeRef, position)
  
  nodeComponent.updateTransforms(nodeRef)
end

function onCreate(p_EntityRef, p_DeltaT)
	
	local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)
	local position = nodeComponent.getPosition(nodeRef)

	math.randomseed( os.time() )
end

function onDestroy(p_EntityRef, p_DeltaT)
end
