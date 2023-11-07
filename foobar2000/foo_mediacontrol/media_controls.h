#pragma once

class media_controls
{
public:
	media_controls();
	~media_controls();
	void status_play();
	void status_pause();
	void status_stop();
	void media_clear();
	void media_update();
	bool get_enabled();
	void set_type(winrt::Windows::Media::MediaPlaybackType type);
	void set_status(winrt::Windows::Media::MediaPlaybackStatus status);
	void set_title(const std::wstring title);
	void set_artist(const std::wstring artist);
	void set_album_artist(const std::wstring album_artist);
	void set_album(const std::wstring album);
	void set_genres(const std::vector<winrt::hstring> genres);
	void set_track_number(const int number);
	void set_track_length(const double length);
	void set_position(const double position);
	void set_album_art(const album_art_data::ptr art_data);
	inline static media_controls& controls()
	{
		static media_controls controls;
		return controls;
	}

private:
	winrt::Windows::Media::SystemMediaTransportControls m_controls = nullptr;
	winrt::Windows::Media::SystemMediaTransportControlsDisplayUpdater m_updater = nullptr;
	winrt::Windows::Media::SystemMediaTransportControlsTimelineProperties m_timeline = nullptr;
	winrt::Windows::Media::MusicDisplayProperties m_properties = nullptr;
};