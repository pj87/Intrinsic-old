posY_1 = 0.0
iTime_1 = 0.0
initialized_1 = 0

function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, p_DeltaT * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef);
  nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  if initialized_1 == 0 then
	posX = position.x
	posZ = position.z
	initialized_1 = 1
  end
  
  posY_1 = posY_1 + p_DeltaT * 1.0  
  iTime_1 = iTime_1 + p_DeltaT
 
  if iTime_1 > 4.0 then
    iTime_1 = 0
	posY_1 = -1.5
	offsetX_1 = math.random(-200.0, 200.0)
	offsetZ_1 = math.random(-200.0, 200.0)
  end
  
  nodeComponent.setPosition(nodeRef, Vec3.new(posX + offsetX_1, posY_1 * 100.0, posZ + offsetZ_1))
  
  -- local y = position.y + p_DeltaT
  -- print("DUPA")
  -- print(position.y)
  -- print(dupa)

  nodeComponent.updateTransforms(nodeRef)
end

function onCreate(p_EntityRef, p_DeltaT)

	local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)
	local position = nodeComponent.getPosition(nodeRef)

	math.randomseed( os.time() )
end

function onDestroy(p_EntityRef, p_DeltaT)
end
