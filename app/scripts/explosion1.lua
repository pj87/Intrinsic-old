iTime_1 = 0.0
initialized_1 = 0

function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, p_DeltaT * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef)
  local size = nodeComponent.getSize(nodeRef)
  nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  if initialized_1 == 0 then
	posX = position.x
	posZ = position.z
	initialized_1= 1
  end
  
 iTime_1 = iTime_1 + p_DeltaT
 
  if iTime_1 > 6.0 then
    iTime_1 = 2.0
	position.x = posX + math.random(-200.0, 200.0)
	position.y = -1.5 * size.x
	position.z = posZ + math.random(-200.0, 200.0)
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
