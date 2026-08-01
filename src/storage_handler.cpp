#include <iostream>
#include <filesystem>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <sys/stat.h>
#include "include/data.hpp"
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

std::string construct_song_path(int id) {
  return base_music_path_data + std::to_string(id);
}

std::string get_song_title(int id) {
  auto song_path = construct_song_path(id);

  std::ifstream title_file(song_path + ".title");
  std::string title_string = "";
  if (title_file.good()) {
    std::getline(title_file, title_string);
  } else {
    std::cerr << "Error: Failed to read title from '" << song_path << ".title" << "'.";
  }

  return title_string;
}

std::string get_song_artist(int id) {
  auto song_path = construct_song_path(id);

  std::ifstream artist_file(song_path + ".artist");
  std::string artist_string = "";
  if (artist_file.good()) {
    std::getline(artist_file, artist_string);
  } else {
    std::cerr << "Error: Failed to read artist name from '" << song_path << ".artist" << "'.";
  }

  return artist_string;
}

std::vector<std::string> get_all_playlists() {
  std::vector<std::string> playlists;

  struct stat s;
  for (const auto& entry : std::filesystem::directory_iterator(base_music_path_playlists)) {
    auto path = entry.path();
    auto str_path = path.string();

    // First check that the path doesn't contain a dot, because the playlist files don't
    // have an extension. Then check that the path is a file and not a directory.
    if (path.filename().string().find(".") == std::string::npos && stat(str_path.c_str(), &s) == 0 && !(s.st_mode & S_IFDIR)) {
      auto u8 = path.stem().u8string();
      playlists.push_back(std::string(reinterpret_cast<const char*>(u8.c_str())));
    }
  }

  return playlists;
}

std::vector<int> get_playlist(const std::string& name) {
  std::vector<int> ids;
  std::string entry;
  std::ifstream playlist_file(base_music_path_playlists + name);

  while (std::getline(playlist_file, entry)) {
    int id = std::stoi(entry);

    if (std::find(ids.begin(), ids.end(), id) == ids.end())
      ids.push_back(id);
  }

  std::shuffle(ids.begin(), ids.end(), rand_generator);

  return ids;
}

std::vector<int> search_all_songs(const std::string& query) {
  std::vector<int> results;

  struct stat s;
  for (const auto& entry : std::filesystem::directory_iterator(base_music_path_data)) {
    auto path = entry.path();
    auto str_path = path.string();

    // First check that the path contains the .title extensions, since it is one of the
    // extensions every song has. Then check that the path is a file and not a directory.
    if (path.filename().string().find(".title") != std::string::npos && stat(str_path.c_str(), &s) == 0 && !(s.st_mode & S_IFDIR)) {
      int id = -1;
      try {
        auto u8 = path.stem().u8string();
        id = std::stoi(std::string(reinterpret_cast<const char*>(u8.c_str())));
      } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
      } catch (const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
      }

      if (id >= 0) {
        if (matching(query, get_song_title(id), match_diff)) {
          results.push_back(id);
          continue; // Don't bother matching the artist string
        }

        if (matching(query, get_song_artist(id), match_diff)) {
          results.push_back(id);
        }
      }
    }
  }

  return results;
}
