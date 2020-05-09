-- Explosion half period

iTime_hp_1 = 0.0
initialized_hp_1 = 0
spawnCounter_hp_1 = 1
modulus_hp_1 = 2

function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, p_DeltaT * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef)
  local size = nodeComponent.getSize(nodeRef)
  nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  if initialized_hp_1 == 0 then
	posX = position.x
	posZ = position.z
	initialized_hp_1 = 1
  end
  
  iTime_hp_1 = iTime_hp_1 + p_DeltaT
 
  if iTime_hp_1 > 6.0 then
    iTime_hp_1 = 2.0
	position.x = posX + math.random(-500.0, 500.0)
	position.z = posZ + math.random(-500.0, 500.0)
	spawnCounter_hp_1 = spawnCounter_hp_1 + 1
	position.y = -5.0 * size.x
  end
  
  if spawnCounter_hp_1 % modulus_hp_1 == 0 then
    position.y = position.y + size.x * 0.1
    nodeComponent.setPosition(nodeRef, position)
  end

  nodeComponent.updateTransforms(nodeRef)
end

function onCreate(p_EntityRef, p_DeltaT)

	local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)
	local position = nodeComponent.getPosition(nodeRef)

	math.randomseed( os.time() )
end

function onDestroy(p_EntityRef, p_DeltaT)
end
