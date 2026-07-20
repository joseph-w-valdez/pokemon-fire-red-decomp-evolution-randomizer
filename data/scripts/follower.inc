EventScript_FollowerTalk::
	lock
	faceplayer
	bufferpartymonnick STR_VAR_1, 0
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, GetPartyMonSpecies
	playmoncry VAR_RESULT, CRY_MODE_NORMAL
	msgbox FollowerTalk_Text_HappyToFollow, MSGBOX_DEFAULT
	waitmoncry
	release
	end

FollowerTalk_Text_HappyToFollow::
	.string "{STR_VAR_1} is happy to follow you!$"
