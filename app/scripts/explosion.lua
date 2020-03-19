posX = 0.0
posY = 0.0
posZ = 0.0
iTime = 0.0
offsetX = 0.0
offsetZ = 0.0
initialized = 0


function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, p_DeltaT * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef);
  nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  if initialized == 0 then
	posX = position.x
	posZ = position.z
	initialized = 1
  end
  
  posY = posY + p_DeltaT * 1.0  
  iTime = iTime + p_DeltaT
 
  if iTime > 4.0 then
    iTime = 0
	posY = -1.5
	offsetX = math.random(-200.0, 200.0)
	offsetZ = math.random(-200.0, 200.0)
  end
  
  nodeComponent.setPosition(nodeRef, Vec3.new(posX + offsetX, posY * 100.0, posZ + offsetZ))
  
  -- local y = position.y + p_DeltaT
  -- print("DUPA")
  -- print(position.y)
  -- print(dupa)

  nodeComponent.updateTransforms(nodeRef)
end

function onCreate(p_EntityRef, p_DeltaT)
end

function onDestroy(p_EntityRef, p_DeltaT)
end
