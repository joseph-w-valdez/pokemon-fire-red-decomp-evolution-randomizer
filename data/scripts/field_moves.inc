EventScript_CutTree::
	goto_if_questlog EventScript_ReleaseEnd
	lockall
	goto_if_unset FLAG_BADGE02_GET, EventScript_CantCutTree
	checkitem ITEM_EDGELORD, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_CantCutTree
	bufferitemname STR_VAR_2, ITEM_EDGELORD
	msgbox Text_CutTreeDown, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_DontCutTree
	msgbox Text_PlayerUsedKeyItem
	closemessage
	dofieldeffect FLDEFF_USE_CUT_ON_TREE
	waitstate
	goto EventScript_CutTreeDown
	end

EventScript_FldEffCut::
	lockall
	dofieldeffect FLDEFF_USE_CUT_ON_TREE
	waitstate
	goto EventScript_CutTreeDown
	end

EventScript_CutTreeDown::
	applymovement VAR_LAST_TALKED, Movement_CutTreeDown
	waitmovement 0
	removeobject VAR_LAST_TALKED
	releaseall
	end

Movement_CutTreeDown::
	cut_tree
	step_end

EventScript_CantCutTree::
	msgbox Text_TreeCanBeCutDown, MSGBOX_SIGN
	releaseall
	end

EventScript_DontCutTree::
	closemessage
	releaseall
	end

Text_CutTreeDown::
	.string "This tree looks like it can be\n"
	.string "cut down!\p"
	.string "Would you like to use the\n"
	.string "EDGELORD?$"

Text_PlayerUsedKeyItem::
	.string "{PLAYER} used the\n"
	.string "{STR_VAR_2}!$"

Text_TreeCanBeCutDown::
	.string "This tree looks like it can be\n"
	.string "cut down!$"

EventScript_RockSmash::
	goto_if_questlog EventScript_ReleaseEnd
	lockall
	goto_if_unset FLAG_BADGE06_GET, EventScript_CantSmashRock
	checkitem ITEM_GEOCIDE, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_CantSmashRock
	bufferitemname STR_VAR_2, ITEM_GEOCIDE
	msgbox Text_UseRockSmash, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_DontSmashRock
	msgbox Text_PlayerUsedKeyItem
	closemessage
	dofieldeffect FLDEFF_USE_ROCK_SMASH
	waitstate
	goto EventScript_UseRockSmash
	end

EventScript_FldEffRockSmash::
	lockall
	dofieldeffect FLDEFF_USE_ROCK_SMASH
	waitstate
	goto EventScript_UseRockSmash
	end

EventScript_UseRockSmash::
	applymovement VAR_LAST_TALKED, Movement_BreakRock
	waitmovement 0
	removeobject VAR_LAST_TALKED
	special RockSmashWildEncounter
	goto_if_eq VAR_RESULT, FALSE, EventScript_RockSmashNoEncounter
	waitstate
	releaseall
	end

EventScript_RockSmashNoEncounter::
	releaseall
	end

Movement_BreakRock::
	rock_smash_break
	step_end

EventScript_CantSmashRock::
	msgbox Text_MonMaySmashRock, MSGBOX_SIGN
	end

EventScript_DontSmashRock::
	closemessage
	releaseall
	end

Text_UseRockSmash::
	.string "This rock appears to be breakable.\n"
	.string "Would you like to use the GEOCIDE?$"

Text_MonMaySmashRock::
	.string "It's a rugged rock, but it looks\n"
	.string "like it can be smashed.$"

EventScript_StrengthBoulder::
	goto_if_questlog EventScript_ReleaseEnd
	lockall
	goto_if_unset FLAG_BADGE04_GET, EventScript_CantMoveBoulder
	goto_if_set FLAG_SYS_USE_STRENGTH, EventScript_AlreadyUsedStrength
	checkitem ITEM_ANCHOR_ARMS, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_CantMoveBoulder
	bufferitemname STR_VAR_1, ITEM_ANCHOR_ARMS
	msgbox Text_UseStrength, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_DontUseStrength
	closemessage
	dofieldeffect FLDEFF_USE_STRENGTH
	waitstate
	goto EventScript_UseStrength
	end

EventScript_FldEffStrength::
	lockall
	dofieldeffect FLDEFF_USE_STRENGTH
	waitstate
	goto EventScript_UseStrength
	end

EventScript_UseStrength::
	setflag FLAG_SYS_USE_STRENGTH
	bufferitemname STR_VAR_1, ITEM_ANCHOR_ARMS
	msgbox Text_MonUsedStrengthCanMoveBoulders, MSGBOX_SIGN
	end

EventScript_CantMoveBoulder::
	msgbox Text_MonMayPushBoulder, MSGBOX_SIGN
	end

EventScript_AlreadyUsedStrength::
	msgbox Text_StrengthMadeMovingBouldersPossible, MSGBOX_SIGN
	end

EventScript_DontUseStrength::
	closemessage
	releaseall
	end

Text_UseStrength::
	.string "It's a big boulder, but it looks\n"
	.string "like it can be pushed aside.\p"
	.string "Would you like to use the\n"
	.string "ANCHOR ARMS?$"

Text_MonUsedStrengthCanMoveBoulders::
	.string "{PLAYER} used the\n"
	.string "{STR_VAR_1}!\p"
	.string "It became possible to move\n"
	.string "boulders around!$"

Text_MonMayPushBoulder::
	.string "It's a big boulder, but it looks\n"
	.string "like it can be pushed aside.$"

Text_StrengthMadeMovingBouldersPossible::
	.string "The ANCHOR ARMS made it possible\n"
	.string "to move boulders around.$"

EventScript_Waterfall::
	goto_if_questlog EventScript_ReleaseEnd
	lockall
	goto_if_unset FLAG_BADGE07_GET, EventScript_WaterCrashingDown
	checkitem ITEM_FISH_LADDER, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_WaterCrashingDown
	bufferitemname STR_VAR_2, ITEM_FISH_LADDER
	msgbox Text_UseWaterfall, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_EndWaterfall
	msgbox Text_PlayerUsedKeyItem
	dofieldeffect FLDEFF_USE_WATERFALL
	goto EventScript_EndWaterfall

EventScript_CantUseWaterfall::
	lockall
EventScript_WaterCrashingDown:
	msgbox Text_WallOfWaterCrashingDown
EventScript_EndWaterfall:
	releaseall
	end

Text_WallOfWaterCrashingDown::
	.string "A wall of water is crashing down\n"
	.string "with a mighty roar.$"

Text_UseWaterfall::
	.string "It's a large waterfall.\p"
	.string "Would you like to use the\n"
	.string "FISH LADDER?$"

@ Unused leftover from R/S
EventScript_DeepWater::
	lockall
	checkitem ITEM_TITAN, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_MayGoUnderwater
	bufferitemname STR_VAR_2, ITEM_TITAN
	setfieldeffectargument 1, 1
	msgbox Text_SeaIsDeepUseDive, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_EndDive
	msgbox Text_PlayerUsedKeyItem
	dofieldeffect FLDEFF_USE_DIVE
	goto EventScript_EndDive

EventScript_CantDive::
	lockall
EventScript_MayGoUnderwater:
	msgbox Text_MonMayGoUnderwater
EventScript_EndDive:
	releaseall
	end

EventScript_TrySurface::
	lockall
	checkitem ITEM_TITAN, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_CantSurface
	bufferitemname STR_VAR_2, ITEM_TITAN
	setfieldeffectargument 1, 1
	msgbox Text_LightFilteringUseDive, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_EndSurface
	msgbox Text_PlayerUsedKeyItem
	dofieldeffect FLDEFF_USE_DIVE
	goto EventScript_EndSurface

EventScript_CantSurface::
	lockall
	msgbox Text_MonMaySurface
	goto EventScript_EndSurface

EventScript_ObstacleCantSurface::
	lockall
	msgbox Text_DiveCantBeUsedHere
EventScript_EndSurface:
	releaseall
	end

Text_MonMayGoUnderwater::
	.string "The sea is deep here.\n"
	.string "It looks like you could dive.$"

Text_SeaIsDeepUseDive::
	.string "The sea is deep here.\n"
	.string "Would you like to use the TITAN?$"

Text_MonMaySurface::
	.string "Light is filtering down from above.\n"
	.string "It looks like you could surface.$"

Text_LightFilteringUseDive::
	.string "Light is filtering down from above.\n"
	.string "Would you like to use the TITAN?$"

Text_DiveCantBeUsedHere::
	.string "There is an obstacle above.\n"
	.string "The TITAN can't be used here.$"
EventScript_FailSweetScent::
	msgbox Text_LooksLikeNothingHere, MSGBOX_SIGN
	end

Text_LooksLikeNothingHere::
	.string "Looks like there's nothing here…$"
