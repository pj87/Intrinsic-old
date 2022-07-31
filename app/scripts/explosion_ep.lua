-- Explosion entire period

iTime_ep = 0.0
initialized_ep = 0
spawnCounter_ep = 0
modulus_ep = 1

function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, 0.016, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef);
  local size = nodeComponent.getSize(nodeRef)
  nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  local position = nodeComponent.getPosition(nodeRef)
  
  if initialized_ep == 0 then
	posX = position.x
	posZ = position.z
	initialized_ep = 1
  end
  
  --iTime_ep = iTime_ep + p_DeltaT * 3.0
 
 print("explosion_ep " .. p_DeltaT)
 
  if p_DeltaT > 3.9 then
    --iTime_ep = 0.0
	--position.x = posX + math.random(-500.0, 500.0)
	--position.z = posZ + math.random(-500.0, 500.0)
	spawnCounter_ep = spawnCounter_ep + 1
	--position.y = -5.0 * size.x
	print("explosion_ep reset")
  end
  
  if spawnCounter_ep % modulus_ep == 0 then
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
