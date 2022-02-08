local function earlyInit( modApi )
	modApi.requirements = {}
end

local function init( modApi )
end

local function load( modApi, options, params )
	if params then
	end
end


local function initStrings( modApi )
	local scriptPath = modApi:getScriptPath()

	local strings = include( scriptPath .. "/strings" )
	modApi:addStrings( modApi:getDataPath(), "IIMODMAN", strings )
end

return {
	earlyInit = earlyInit,
	init = init,
	load = load,
	initStrings = initStrings,
}
