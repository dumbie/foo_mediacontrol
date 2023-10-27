#pragma once
#include "stdafx.h"
#include "media_controls.h"

void OnButtonPressed(winrt::Windows::Media::SystemMediaTransportControls sender, winrt::Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs args)
{
	fb2k::inMainThread([=]()
		{
			//Get playback control on main thread
			const static_api_ptr_t<playback_control> m_playback_control;

			//Check pressed button action
			switch (args.Button())
			{
			case winrt::Windows::Media::SystemMediaTransportControlsButton::Play:
				m_playback_control->play_or_unpause();
				break;
			case winrt::Windows::Media::SystemMediaTransportControlsButton::Pause:
				m_playback_control->pause(true);
				break;
			case winrt::Windows::Media::SystemMediaTransportControlsButton::Stop:
				m_playback_control->stop();
				break;
			case winrt::Windows::Media::SystemMediaTransportControlsButton::Next:
				m_playback_control->next();
				break;
			case winrt::Windows::Media::SystemMediaTransportControlsButton::Previous:
				m_playback_control->previous();
				break;
			}
		});
}

void OnPlaybackPositionChangeRequested(winrt::Windows::Media::SystemMediaTransportControls sender, winrt::Windows::Media::PlaybackPositionChangeRequestedEventArgs args)
{
	fb2k::inMainThread([=]()
		{
			//Get playback control on main thread
			const static_api_ptr_t<playback_control> m_playback_control;

			//Seek player to requested position
			double seek_converted = args.RequestedPlaybackPosition().count() / 10000000;
			m_playback_control->playback_seek(seek_converted);
		});
}

media_controls::media_controls()
{
	m_controls = winrt::Windows::Media::Playback::BackgroundMediaPlayer::Current().SystemMediaTransportControls();
	m_timeline = winrt::Windows::Media::SystemMediaTransportControlsTimelineProperties();

	m_updater = m_controls.DisplayUpdater();
	m_updater.Type(winrt::Windows::Media::MediaPlaybackType::Music);
	m_properties = m_updater.MusicProperties();

	m_controls.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Closed);
	m_controls.IsEnabled(false);
	m_controls.IsPlayEnabled(true);
	m_controls.IsPauseEnabled(true);
	m_controls.IsStopEnabled(true);
	m_controls.IsPreviousEnabled(true);
	m_controls.IsNextEnabled(true);

	m_controls.ButtonPressed(winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Media::SystemMediaTransportControls, winrt::Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs>(OnButtonPressed));
	m_controls.PlaybackPositionChangeRequested(winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Media::SystemMediaTransportControls, winrt::Windows::Media::PlaybackPositionChangeRequestedEventArgs>(OnPlaybackPositionChangeRequested));

	console::info("Windows SMTC: Initialized controls.");
}

media_controls::~media_controls()
{
	media_clear();
}

void media_controls::status_play()
{
	m_controls.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Playing);
	m_controls.IsEnabled(true);
}

void media_controls::status_pause()
{
	m_controls.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Paused);
	m_controls.IsEnabled(true);
}

void media_controls::status_stop()
{
	m_controls.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Stopped);
	m_controls.IsEnabled(false);
}

void media_controls::media_clear()
{
	m_updater.ClearAll();
}

void media_controls::media_update()
{
	m_updater.Update();
}

bool media_controls::get_enabled()
{
	return m_controls.IsEnabled();
}

void media_controls::set_type(winrt::Windows::Media::MediaPlaybackType type)
{
	m_updater.Type(type);
}

void media_controls::set_status(winrt::Windows::Media::MediaPlaybackStatus status)
{
	m_controls.PlaybackStatus(status);
}

void media_controls::set_title(const std::wstring title)
{
	m_properties.Title(title);
}

void media_controls::set_artist(const std::wstring artist)
{
	m_properties.Artist(artist);
}

void media_controls::set_album_artist(const std::wstring album_artist)
{
	m_properties.AlbumArtist(album_artist);
}

void media_controls::set_album(const std::wstring album)
{
	m_properties.AlbumTitle(album);
}

void media_controls::set_genres(std::vector<winrt::hstring>& genres)
{
	m_properties.Genres().ReplaceAll(genres);
}

void media_controls::set_track_number(const int number)
{
	m_properties.TrackNumber(number);
}

void media_controls::set_track_length(const double length)
{
	auto length_converted = winrt::Windows::Foundation::TimeSpan{ (long long)round(length) * 10000000 };

	m_timeline.StartTime(winrt::Windows::Foundation::TimeSpan{ 0 });
	m_timeline.EndTime(length_converted);

	m_timeline.MinSeekTime(winrt::Windows::Foundation::TimeSpan{ 0 });
	m_timeline.MaxSeekTime(length_converted);

	m_controls.UpdateTimelineProperties(m_timeline);
}

void media_controls::set_position(const double position)
{
	auto position_converted = winrt::Windows::Foundation::TimeSpan{ (long long)round(position) * 10000000 };

	m_timeline.Position(position_converted);

	m_controls.UpdateTimelineProperties(m_timeline);
}

void media_controls::set_album_art(album_art_data::ptr data)
{
	if (data != nullptr)
	{
		if (IStream* istream = SHCreateMemStream((const BYTE*)data->get_ptr(), data->get_size()))
		{
			winrt::Windows::Storage::Streams::IRandomAccessStream randomstream;
			auto uuid = __uuidof(ABI::Windows::Storage::Streams::IRandomAccessStream);
			if (CreateRandomAccessStreamOverStream(istream, BSOS_DEFAULT, uuid, (void**)&randomstream) == S_OK)
			{
				m_updater.Thumbnail(winrt::Windows::Storage::Streams::RandomAccessStreamReference::CreateFromStream(randomstream));
			}
		}
	}
}