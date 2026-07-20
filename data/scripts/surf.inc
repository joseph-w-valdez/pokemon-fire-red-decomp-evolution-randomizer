EventScript_UseSurf::
	goto_if_questlog EventScript_UseSurfEnd
	goto_if_unset FLAG_BADGE05_GET, EventScript_UseSurfEnd
	checkitem ITEM_POOL_NOODLE, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_UseSurfEnd
	bufferitemname STR_VAR_2, ITEM_POOL_NOODLE
	lockall
	msgbox Text_WantToSurf, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_UseSurfRelease
	msgbox Text_UsedSurf
	dofieldeffect FLDEFF_USE_SURF
EventScript_UseSurfRelease:
	releaseall
EventScript_UseSurfEnd:
	end

EventScript_CurrentTooFast::
	lockall
	msgbox Text_CurrentTooFast
	releaseall
	end
