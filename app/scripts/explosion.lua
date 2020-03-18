y = 0
t = 0

function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, p_DeltaT * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef);
  --nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  y = y + p_DeltaT * 1.0  
  t = t + p_DeltaT
 
  if t > 4.0 then
    t = 0
	y = -0.75
  end
  
  --nodeComponent.setPosition(nodeRef, Vec3.new(position.x, math.abs(math.sin(y)) * 100.0, position.z))
  
  nodeComponent.setPosition(nodeRef, Vec3.new(position.x, y * 100.0, position.z))
  
  print (t)
  
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
