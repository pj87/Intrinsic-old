-- Temple movement script

iTime_t = 0.0
initialized_t = 0

function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, p_DeltaT * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef);
  local size = nodeComponent.getSize(nodeRef)
  --nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  if initialized_t == 0 then
	posX = position.x
	posZ = position.z
	initialized_t = 1
  end
  
  iTime_t = iTime_t + p_DeltaT
 
  --position.y = position.y + size.x * 0.01
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
