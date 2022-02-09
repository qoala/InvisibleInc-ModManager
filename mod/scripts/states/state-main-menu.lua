local stateMainMenu = include( "states/state-main-menu" )

local util = include("client_util")
local array = include( "modules/array" )
local mui = include( "mui/mui" )
local serverdefs = include( "modules/serverdefs" )
local modalDialog = include( "states/state-modal-dialog" )
local rig_util = include( "gameplay/rig_util" )
local cdefs = include("client_defs")
local scroll_text = include("hud/scroll_text")
local unitdefs = include( "sim/unitdefs" )
local metadefs = include( "sim/metadefs" )
local simdefs = include( "sim/simdefs" )
local movieScreen = include('client/fe/moviescreen')
local stateTeamPreview = include( "states/state-team-preview" )
local mui_button = include( "mui/widgets/mui_button" )


local oldOnLoad = stateMainMenu.onLoad

local function onEnterSpool( widget )
	widget:spoolText( widget:getText() )
end

local function onClickModManBtn(self)
end

function stateMainMenu:onLoad( ... )
	oldOnLoad( self, ... )

    self.screen.binder.modManBtn:setText("MOD MANAGER")
    self.screen.binder.modManBtn.onClick = util.makeDelegate( nil, onClickModManBtn, self )
    self.screen.binder.modManBtn.onEnter = onEnterSpool
end
