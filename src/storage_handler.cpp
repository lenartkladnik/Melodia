#include <iostream>
#include <filesystem>
#include <fstream>
#include <cctype>
#include <algorithm>
#include "include/utils.hpp"

extern const std::string base_path = "./";
extern const std::string base_path_misc = base_path + "misc/";
extern const std::string base_path_external = base_path + "external/";
extern const std::string base_path_external_prog = base_path_external + "prog/";
extern const std::string base_music_path = ".music_data/";
extern const std::string base_music_path_data = base_music_path + "data/";
extern const std::string base_music_path_playlists = base_music_path + "playlists/";
extern const std::string path_counter_prefix = " #";

bool ensure_storage() {
  try {
    if (!(must_exist(base_path) && must_exist(base_path_misc)))
      return false;

    mkdir(base_music_path);
    mkdir(base_music_path_data);
    mkdir(base_music_path_playlists);
    mkdir(base_path_external_prog);
    mkdir(base_path_external);
    mkdir(base_path_external_prog);
  } catch (const std::filesystem::filesystem_error err) {
    std::cout << "Error: Failed to check and create necessary directories '" << err.what() << "'.\n";
    return false;
  }

  return true;
}

void remove_playlist(std::string playlist) {
  auto playlist_path = base_music_path_playlists + playlist;
  if (std::filesystem::exists(playlist_path))
    std::filesystem::remove(playlist_path);
  if (std::filesystem::exists(playlist_path + ".png"))
    std::filesystem::remove(playlist_path + ".png");
}

std::string create_new_playlist(int song_id) {
  std::string playlist = "New playlist";
  auto playlist_path = get_next_available_path(base_music_path_playlists + playlist);
  auto playlist_stem = get_stem(playlist_path);

  std::ofstream playlist_file(playlist_path);
  playlist_file << song_id << "\n";
  playlist_file.close();

  std::filesystem::copy(base_music_path_data + std::to_string(song_id) + ".png", base_music_path_playlists + playlist_stem + ".png");

  return playlist_stem;
}

std::string rename_playlist(std::string old_playlist, std::string new_playlist) {
  if (!is_string_valid_name(new_playlist))
    throw "Playlist name contains invalid characters\n";

  auto new_playlist_path = get_next_available_path(base_music_path_playlists + new_playlist);
  std::filesystem::rename(base_music_path_playlists + old_playlist, new_playlist_path);

  return get_stem(new_playlist_path);
}

void add_to_playlist(std::string playlist, int song_id) {
  std::ofstream playlist_file(base_music_path_playlists + playlist, std::ios_base::app);
  playlist_file << song_id << "\n";
  playlist_file.close();
}

void remove_from_playlist(std::string playlist, int song_id) {
  std::ifstream playlist_file_i(base_music_path_playlists + playlist);
  std::string id;
  std::string new_ids = "";
  while (std::getline(playlist_file_i, id)) {
    if (id != std::to_string(song_id))
      new_ids += id + "\n";
  }
  playlist_file_i.close();

  std::ofstream playlist_file_o(base_music_path_playlists + playlist);
  playlist_file_o << new_ids;
  playlist_file_o.close();
}
