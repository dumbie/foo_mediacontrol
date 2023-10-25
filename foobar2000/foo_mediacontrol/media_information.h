#pragma once

class media_information
{
public:
	media_information(metadb_handle_ptr metadb_handle);

	inline std::wstring get_file_name()
	{
		return m_file_name;
	}

	inline std::wstring get_title()
	{
		return m_title;
	}

	inline std::wstring get_artist()
	{
		return m_artist;
	}

	inline std::wstring get_album_artist()
	{
		return m_album_artist;
	}

	inline std::wstring get_album()
	{
		return m_album;
	}

	inline std::vector<winrt::hstring>& get_genres()
	{
		return m_genres;
	}

	inline int get_track_number()
	{
		return m_track_number;
	}

	inline double get_track_length()
	{
		return m_track_length;
	}

	inline album_art_data::ptr& get_album_art()
	{
		return m_album_art;
	}

private:
	std::wstring m_file_name = L"";
	std::wstring m_title = L"";
	std::wstring m_artist = L"";
	std::wstring m_album_artist = L"";
	std::wstring m_album = L"";
	std::vector<winrt::hstring> m_genres = std::vector<winrt::hstring>();
	int m_track_number = 0;
	double m_track_length = 0;
	album_art_data::ptr m_album_art = nullptr;
};