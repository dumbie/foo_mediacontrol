#pragma once
#include "stdafx.h"
#include "monitor_input.h"

bool monitor_input::pretranslate_message(MSG* p_msg)
{
	//Fix check if foobar2000 media control setting is enabled
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
	return false;
}