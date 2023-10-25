#pragma once
#include "stdafx.h"
#include "media_position.h"
#include "media_controls.h"

media_position::media_position()
{
	update_allowed = true;
	std::thread thread_position(&media_position::update_thread, this);
	thread_position.detach();
}

media_position::~media_position()
{
	update_allowed = false;
}

void media_position::update_thread()
{
	while (update_allowed)
	{
		fb2k::inMainThread([=]()
			{
				//Get playback control on main thread
				const static_api_ptr_t<playback_control> m_playback_control;

				//Get current media position
				double current_position = m_playback_control->playback_get_position();

				//Set current media position
				media_controls::controls().set_position(current_position);
			});

		//Wait for second to update
		Sleep(1000);
	}
}