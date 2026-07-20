FuchsiaCity_SafariZone_Entrance_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FuchsiaCity_SafariZone_Entrance_OnTransition
	.byte 0

@ Safari Zone is a normal wild area now — clear any leftover session state.
FuchsiaCity_SafariZone_Entrance_OnTransition::
	special ExitSafariMode
	setvar VAR_MAP_SCENE_FUCHSIA_CITY_SAFARI_ZONE_ENTRANCE, 0
	end

FuchsiaCity_SafariZone_Entrance_EventScript_EntryTriggerMid::
	lockall
	setvar VAR_TEMP_2, 0
	goto FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneFree
	end

FuchsiaCity_SafariZone_Entrance_EventScript_EntryTriggerRight::
	lockall
	setvar VAR_TEMP_2, 1
	goto FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneFree
	end

FuchsiaCity_SafariZone_Entrance_EventScript_EntryTriggerLeft::
	lockall
	setvar VAR_TEMP_2, 2
	goto FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneFree
	end

@ Free entry: no fee, no dialogue, no Safari Mode session.
FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneFree::
	call_if_eq VAR_TEMP_2, 0, FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneMid
	call_if_eq VAR_TEMP_2, 1, FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneRight
	call_if_eq VAR_TEMP_2, 2, FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneLeft
	warp MAP_SAFARI_ZONE_CENTER, 255, 26, 30
	waitstate
	end

FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneRight::
	applymovement LOCALID_PLAYER, FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneRight
	waitmovement 0
	return

FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneMid::
	applymovement LOCALID_PLAYER, FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneMid
	waitmovement 0
	return

FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneLeft::
	applymovement LOCALID_PLAYER, FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneLeft
	waitmovement 0
	return

FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneMid::
	walk_up
	walk_up
	step_end

FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneRight::
	walk_up
	walk_left
	walk_up
	step_end

FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneLeft::
	walk_up
	walk_right
	walk_up
	step_end

FuchsiaCity_SafariZone_Entrance_EventScript_InfoAttendant::
	msgbox FuchsiaCity_SafariZone_Entrance_Text_SafariZoneOpen, MSGBOX_NPC
	end
