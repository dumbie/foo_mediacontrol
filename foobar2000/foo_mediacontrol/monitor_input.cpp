#pragma once
#include "stdafx.h"
#include "monitor_input.h"
#include "media_controls.h"

bool monitor_input::pretranslate_message(MSG* p_msg)
{
	if (media_controls::controls().get_enabled())
	{
		if (p_msg->wParam == HSHELL_APPCOMMAND)
		{
			switch (HIWORD(p_msg->lParam))
			{
			case APPCOMMAND_MEDIA_PLAY:
			case APPCOMMAND_MEDIA_PAUSE:
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
			case APPCOMMAND_MEDIA_STOP:
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
			case APPCOMMAND_MEDIA_NEXTTRACK:
				return true;
			}
		}
	}
	return false;
}