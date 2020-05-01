-- Explosion entire period

iTime_ep = 0.0
initialized_ep = 0

function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, p_DeltaT * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef);
  local size = nodeComponent.getSize(nodeRef)
  nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  if initialized_ep == 0 then
	posX = position.x
	posZ = position.z
	initialized_ep = 1
  end
  
  iTime_ep = iTime_ep + p_DeltaT
 
  if iTime_ep > 4.0 then
    iTime_ep = 0.0
	position.x = posX + math.random(-500.0, 500.0)
	position.y = -3.0 * size.x
	position.z = posZ + math.random(-500.0, 500.0)
  end
  
  position.y = position.y + size.x * 0.1
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
