#ifndef STORAGE_HANDLER_HPP
#define STORAGE_HANDLER_HPP

extern const std::string base_path;
extern const std::string base_path_misc;
extern const std::string base_path_misc_rasters;
extern const std::string base_path_external;
extern const std::string base_path_external_prog;
extern const std::string base_music_path;
extern const std::string base_music_path_data;
extern const std::string base_music_path_playlists;
extern const std::string path_counter_prefix;

bool ensure_storage();
void remove_playlist(std::string playlist);
void remove_song(std::string id);
size_t get_next_avaliable_song_id();
std::string create_new_playlist(int song_id);
std::string rename_playlist(std::string old_playlist, std::string new_playlist);
void add_to_playlist(std::string playlist, int song_id);
void remove_from_playlist(std::string playlist, int song_id);
std::string construct_song_path(int id);
std::u32string get_song_title(int id);
std::u32string get_song_artist(int id);
std::vector<std::string> get_all_playlists();
std::vector<int> get_playlist(const std::string& name);
std::vector<int> search_all_songs(const std::u32string& query);

#endif
