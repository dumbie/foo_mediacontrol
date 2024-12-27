#pragma once

class monitor_player : public play_callback_impl_base
{
private:
	void on_playback_starting(play_control::t_track_command p_command, bool p_paused);
	void on_playback_new_track(metadb_handle_ptr metadb_handle);
	void on_playback_stop(play_control::t_stop_reason p_reason);
	void on_playback_pause(bool p_state);
	void on_playback_time(double p_time);
};