-- Removes all references to a module.
-- Do not call unrequire on a shared library based module unless you are 100% confidant that nothing uses the module anymore.
-- @param m Name of the module you want removed.
-- @return Returns true if all references were removed, false otherwise.
-- @return If returns false, then this is an error message describing why the references weren't removed.
local function unrequire(m)
	package.loaded[m] = nil
	_G[m] = nil
	 
	-- Search for the shared library handle in the registry and erase it
	local registry = debug.getregistry()
	local nMatches, mKey, mt = 0, nil, registry["_LOADLIB"]
	 
	for key, ud in pairs(registry) do
		if type(key) == "string" and string.find(key, "LOADLIB: .*" .. m) and type(ud) == "userdata" and getmetatable(ud) == mt then
			nMatches = nMatches + 1
			if nMatches > 1 then
				return false, "More than one possible key for module '" .. m .. "'. Can't decide which one to erase."
			end

			mKey = key
		end
	end
	 
	if mKey then
		registry[mKey] = nil
	end
	 
	return true
end