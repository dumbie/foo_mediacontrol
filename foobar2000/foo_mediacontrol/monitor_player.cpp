#pragma once
#include "stdafx.h"
#include "monitor_player.h"
#include "media_controls.h"
#include "media_information.h"

void monitor_player::on_playback_new_track(metadb_handle_ptr metadb_handle)
{
	try
	{
		//Get media information from metadb
		auto media_info = media_information(metadb_handle);

		//Update system media transport controls
		auto media_control = media_controls::controls();
		media_control.media_clear();
		media_control.set_type(winrt::Windows::Media::MediaPlaybackType::Music);
		auto mediaTitle = media_info.get_title();
		if (mediaTitle.empty())
		{
			mediaTitle = media_info.get_file_name();
		}
		media_control.set_title(mediaTitle);
		media_control.set_artist(media_info.get_artist());
		media_control.set_album_artist(media_info.get_album_artist());
		media_control.set_album(media_info.get_album());
		media_control.set_genres(media_info.get_genres());
		media_control.set_track_number(media_info.get_track_number());
		media_control.set_track_length(media_info.get_track_length());
		media_control.set_position(0);
		media_control.set_album_art(media_info.get_album_art());
		media_control.media_update();

		console::info("Windows SMTC: Updated media controls.");
	}
	catch (...)
	{
		console::error("Windows SMTC: Failed to update media controls.");
	}
}

void monitor_player::on_playback_starting(play_control::t_track_command p_command, bool p_paused)
{
	media_controls::controls().status_play();
}

void monitor_player::on_playback_stop(play_control::t_stop_reason p_reason)
{
	media_controls::controls().status_stop();
}

void monitor_player::on_playback_pause(bool p_state)
{
	if (p_state)
	{
		media_controls::controls().status_pause();
	}
	else
	{
		media_controls::controls().status_play();
	}
}

void monitor_player::on_playback_time(double p_time)
{
	media_controls::controls().set_position(p_time);
}