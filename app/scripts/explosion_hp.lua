-- Explosion half period

iTime_hp = 0.0
initialized_hp = 0
spawnCounter_hp = 0
modulus_hp = 1

function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, 0.016, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef)
  local size = nodeComponent.getSize(nodeRef)
  nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  if initialized_hp == 0 then
	posX = position.x
	posZ = position.z
	initialized_hp = 1
  end
  
  --iTime_hp = iTime_hp + p_DeltaT * 3.0
 
  --print("explosion_hp " .. p_DeltaT)
 
  if p_DeltaT > 5.9 then
    --iTime_hp = 2.0
	--position.x = posX + math.random(-500.0, 500.0)
	--position.z = posZ + math.random(-500.0, 500.0)
	spawnCounter_hp = spawnCounter_hp + 1
	--position.y = -5.0 * size.x
	--print("explosion_hp reset")
  end
  
  if spawnCounter_hp % modulus_hp == 0 then
    --position.y = position.y + size.x * 0.5
    --nodeComponent.setPosition(nodeRef, position)
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
