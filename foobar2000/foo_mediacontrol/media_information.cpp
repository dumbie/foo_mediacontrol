#pragma once
#include "stdafx.h"
#include "media_information.h"

int meta_to_int(const char* meta_source)
{
	return atoi(meta_source);
}

std::wstring meta_to_wstring(const char* meta_source)
{
	t_size in_size = strlen(meta_source);
	t_size out_size = pfc::stringcvt::estimate_utf8_to_wide(meta_source, in_size);

	wchar_t* wchar_array = new wchar_t[out_size];
	pfc::stringcvt::convert_utf8_to_wide(wchar_array, out_size, meta_source, in_size);
	return wchar_array;
}

media_information::media_information(metadb_handle_ptr metadb_handle)
{
	//Get meta information
	metadb_info_container::ptr meta_container = metadb_handle->get_async_info_ref();
	if (meta_container == nullptr)
	{
		console::error("Windows SMTC: Could not read track information.");
		return;
	}

	//Get file information from container
	const file_info& file_info = meta_container->info();

	//Get file name without extension
	m_file_name = meta_to_wstring(pfc::io::path::getFileNameWithoutExtension(metadb_handle->get_path()).c_str());

	//Set track length
	m_track_length = file_info.get_length();

	//Set track information
	if (file_info.meta_exists("title"))
	{
		m_title = meta_to_wstring(file_info.meta_get("title", 0));
	}
	if (file_info.meta_exists("artist"))
	{
		m_artist = meta_to_wstring(file_info.meta_get("artist", 0));
	}
	if (file_info.meta_exists("album artist"))
	{
		m_album_artist = meta_to_wstring(file_info.meta_get("album artist", 0));
	}
	if (file_info.meta_exists("album"))
	{
		m_album = meta_to_wstring(file_info.meta_get("album", 0));
	}
	if (file_info.meta_exists("tracknumber"))
	{
		m_track_number = meta_to_int(file_info.meta_get("tracknumber", 0));
	}

	//Clear track genres
	m_genres.clear();

	//Set track genres
	t_size genre_count = file_info.meta_get_count_by_name("genre");
	for (t_size i = 0; i < genre_count; i++)
	{
		m_genres.push_back(winrt::hstring(meta_to_wstring(file_info.meta_get("genre", i))));
	}

	//Set track art
	m_album_art = nullptr;
	pfc::list_t<GUID> art_identifiers;
	art_identifiers.add_item(album_art_ids::cover_front);
	art_identifiers.add_item(album_art_ids::cover_back);
	art_identifiers.add_item(album_art_ids::disc);
	art_identifiers.add_item(album_art_ids::artist);
	art_identifiers.add_item(album_art_ids::icon);

	metadb_handle_list meta_items;
	meta_items.add_item(metadb_handle);

	abort_callback_dummy abort_callback;
	album_art_extractor_instance_v2::ptr extractor = static_api_ptr_t<album_art_manager_v3>()->open(meta_items, art_identifiers, abort_callback);
	for (t_size i = 0; i < art_identifiers.get_count(); i++)
	{
		if (extractor->query(art_identifiers[i], m_album_art, abort_callback))
		{
			break;
		}
	}
}