local function earlyInit( modApi )
	modApi.requirements = {}
end

local function init( modApi )
	local scriptPath = modApi:getScriptPath()

	include( scriptPath.."/states/state-main-menu" )
end

local function load( modApi, options, params )
	local scriptPath = modApi:getScriptPath()

	if params then
	end

	modApi:insertUIElements( include( scriptPath.."/screen_inserts" ) )
	modApi:addNewUIScreen( "modal-mod-manager", include( scriptPath.."/gui/modal-mod-manager" ) )
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
