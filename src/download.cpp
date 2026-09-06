#include <fstream>
#include <iostream>
#include <filesystem>
#include <future>

#include "include/data.hpp"
#include "include/components.hpp"
#include "include/storage_handler.hpp"
#include "include/utils.hpp"

#ifndef _WIN32
  #define STB_IMAGE_IMPLEMENTATION
  #define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#define STB_IMAGE_RESIZE_IMPLEMENTATION

// Disable warnings produced by external libs
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "../external/lib/httplib.h"
#include "../external/lib/nlohmann/json.hpp"

#pragma GCC diagnostic pop // Enable all warnings

using json = nlohmann::json;

AutocompleteResult get_search_autocomplete_yt_music(const std::string& query) {
  if (query.size() < MIN_CHAR_COUNT_SEARCH) {
    return AutocompleteResult(query, {}, {});
  }

  std::string host = "https://music.youtube.com";
  std::string path = "/youtubei/v1/music/get_search_suggestions?prettyPrint=false";

  nlohmann::json body = {
    {"input", query},
    {"context", {{"client", {
      {"clientName", "WEB_REMIX"},
      {"clientVersion", "1.20240101.01.00"}
    }}}}
  };

  httplib::Client cli(host);
  cli.set_connection_timeout(MIN_AUTOCOMPLETE_RESPONSE_TIME);
  cli.set_read_timeout(MIN_AUTOCOMPLETE_RESPONSE_TIME);

  httplib::Headers headers = {
      {"Origin", "https://music.youtube.com"},
      {"X-Youtube-Client-Name", "67"},
      {"X-Youtube-Client-Version", "1.20240101.01.00"},
  };

  httplib::Result res;

  try {
    res = cli.Post(path, headers, body.dump(), "application/json");
  } catch (...) {
    std::cout << "[ERROR] Failed to perform a POST request for autocomplete.\n";
    return AutocompleteResult(query, {}, {}); // Failed to get results fast enough or something went wrong in the request, return no suggestions
  }

  if (!res || res->status != 200) {
    std::cout << "[ERROR] Failed to fetch valid response for autocomplete (" << (res ? ("status code is " + std::to_string(res->status)) : "no response was given") << ").\n";
    if (res)
      std::cout << "        Error body is:\n" << res->body << "\n";

    return AutocompleteResult(query, {}, {}); // Something went wrong, return no suggestion
  }

  nlohmann::json root = nlohmann::json::parse(res->body);

  auto join_runs = [](const nlohmann::json& node) {
    std::string text;
    for (const auto& run : node.value("runs", nlohmann::json::array()))
        text += run.value("text", "");
    return text;
  };

  std::vector<std::string> suggestions;
  for (const auto& section : root.value("contents", nlohmann::json::array())) {
    auto renderers = section.value("searchSuggestionsSectionRenderer", nlohmann::json::object()).value("contents", nlohmann::json::array());
    for (const auto& item : renderers) {
      for (const char* key : {"searchSuggestionRenderer", "historySuggestionRenderer"}) {
        if (item.contains(key))
          suggestions.push_back(title_string(join_runs( // title_string makes every first letter of a word uppercase
            item[key].value("suggestion", nlohmann::json::object())
          )));
      }
    }
  }

  return AutocompleteResult(query, {}, suggestions);
}

AutocompleteResult get_search_autocomplete_ytdlp(const std::string& query) {
  if (query.size() < MIN_CHAR_COUNT_SEARCH) {
    return AutocompleteResult(query, {}, {});
  }

  if (yt_dlp_path.empty()) {
    std::cout << "[ERROR] Can't get search autocompletion results with yt-dlp since it doesn't exist on the system.\n";
    return AutocompleteResult(query, {}, {});
  }

  int n = 1; // Number of songs
  auto yt_dlp_search_args = " -I \"1:" + std::to_string(n) + "\" \"https://music.youtube.com/search?q=" + query + "\"" + " --no-playlist --skip-download" + " --print \"%(track)s\" --print \"%(artist)s\"";

  std::string r = exec((yt_dlp_path + yt_dlp_search_args).c_str());

  std::vector<std::pair<std::string, std::string>> title_artist_pairs;
  std::vector<std::string> suggestions;

  std::pair<std::string, std::string> tmp_pair;

  bool is_ln_title = true; // First line is title
  for (const auto& ln : split_string(r, '\n')) {
    if (is_ln_title) {
      tmp_pair.first = ln;
    } else {
      tmp_pair.second = ln;

      title_artist_pairs.push_back(tmp_pair);
      suggestions.push_back(tmp_pair.first + " - " + tmp_pair.second);
    }

    is_ln_title = !is_ln_title; // Toggle
  }

  return AutocompleteResult(query, title_artist_pairs, suggestions);
}

bool _download_file(std::string url, const std::string& out_path, int max_redirects = 5) {
  for (int i = 0; i < max_redirects; i++) {
    auto path_start = url.find('/', url.find("://") + 3);
    std::string host = url.substr(0, path_start);
    std::string path = url.substr(path_start);

    httplib::Client cli(host);
    cli.set_follow_location(true);
    cli.set_connection_timeout(10);
    cli.set_read_timeout(30);

    auto res = cli.Get(path);

    if (!res) {
      // Request failed TODO: Add std::cerr
      return false;
    }

    if (res->status == 301 || res->status == 302 || res->status == 307 || res->status == 308) {
      auto new_location = res->get_header_value("location");
      url = new_location;
      continue;
    }

    if (res->status != 200) {
      // Failed to download
      return false;
    }

    std::ofstream out(out_path, std::ios::binary);
    if (!out) return false;
    out.write(res->body.data(), res->body.size());

    return true;
  }
  return false;
}

inline std::string get_yt_dlp_download_url() {
#ifdef _WIN32
    return "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe";
#elif __APPLE__
    return "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_macos";
#elif __linux__
    #ifdef __aarch64__
        return "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_linux_aarch64";
    #else
        return "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp";
    #endif
#endif
}

inline std::string get_yt_dlp_downloaded_path() {
#ifdef _WIN32
  return ".\\external\\programs\\yt-dlp.exe";
#else
  return "./external/programs/yt-dlp";
#endif
}

bool _yt_dlp_download_song_from_query(const std::string& dlp_path, const std::string& query, int new_id) {
  auto new_base = base_music_path_data + std::to_string(new_id);
  auto yt_dlp_args = " -I 1 \"https://music.youtube.com/search?q=" + query + "\" -xciw -f \"bestaudio/best\" --audio-format mp3 --audio-quality 0 --no-playlist --print-to-file \"%(artist)s\" " + new_base + ".artist --print-to-file \"%(track)s\" " + new_base + ".title -o \"" + new_base + "\".mp3";

  auto command = dlp_path + yt_dlp_args; // dlp_path + " --ffmpeg-location \"" + progs_path + "\"" + yt_dlp_args;

  std::cout << "Calling system with '" << command << "'\n";

  return system(command.c_str()) == 0;
}

bool _resize_cover_art(const std::string& temp_file_path, const std::string& output, int target_w, int target_h) {
  progress_bar_doing_string = "Resizing cover art";
  progress_bar_amount += 1.f; // Done downloading / started resizing the cover art image

  if (!resize_image(temp_file_path, output, {(unsigned)target_w, (unsigned)target_h})) {
    return false;
  }

  std::cout << "Info: Resized cover art image\n";
  progress_bar_amount += 1.f; // Done writing cover art image

  return true;
}

// urlencode and hexchar - Source:
// https://gist.github.com/litefeel/1197e5c24eb9ec93d771

void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2) {
  hex1 = c / 16;
  hex2 = c % 16;
  hex1 += hex1 <= 9 ? '0' : 'a' - 10;
  hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

std::string urlencode(const std::string& s) {
  const char *str = s.c_str();
  std::vector<char> v(s.size());
  v.clear();
  for (size_t i = 0, l = s.size(); i < l; i++) {
    auto c = str[i];
    if (
      (c >= '0' && c <= '9') ||
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      c == '-'  ||
      c == '_'  ||
      c == '.'  ||
      c == '!'  ||
      c == '~'  ||
      c == '*'  ||
      c == '\'' ||
      c == '('  ||
      c == ')'
    ) {
      v.push_back(c);
    }
    else if (c == ' ') {
      v.push_back('+');
    }
    else {
      v.push_back('%');
      unsigned char d1, d2;
      hexchar(c, d1, d2);
      v.push_back(d1);
      v.push_back(d2);
    }
  }

  return std::string(v.cbegin(), v.cend());
}

bool _download_cover_art(std::string title_string, std::string artist_string, int new_id) {
  progress_bar_doing_string = "Downloading cover art";

  std::string main_base_url = "https://www.last.fm";
  std::string image_base_url = "https://lastfm-img.freetls.fastly.net";

  std::string new_base = base_music_path_data + std::to_string(new_id);
  std::string temp_file_path = new_base + ".cover_art.png.tmp";

  auto query = "/search/albums?q=" + urlencode(title_string + " by " + artist_string);

  progress_bar_doing_string = "Connecting to '" + main_base_url + "'";

  std::cout << "[download.cpp] Results page: '" + main_base_url + query + "'\n";

  httplib::Client cli(main_base_url);
  auto res = cli.Get(query);

  progress_bar_amount += 1.f; // Done getting main website

  progress_bar_doing_string = "Parsing cover art path out of response";

  std::string cover_art_path;
  if (res && res->status == 200) {
    std::istringstream body_ss(res->body);
    for (std::string line; std::getline(body_ss, line);) {
      if (line.find(image_base_url) != std::string::npos) {
        // Store first occurrence and exit

        // Parse the url out of: 'src="https://lastfm.freetls.fastly.net/i/u/64s/<id>.jpg"'
        size_t start = line.find("src=");
        size_t url_start = start + 5;
        cover_art_path = line.substr(url_start, line.size() - url_start - 1);

        // Change the size to be maximally large
        size_t size_start = cover_art_path.find("i/u/") + 4;
        std::string default_size = cover_art_path.substr(size_start, cover_art_path.find('/', size_start) - size_start);
        cover_art_path.replace(cover_art_path.find(default_size), default_size.size(), "1000s");

        // Remove the image base url, leaving only the path
        cover_art_path.erase(0, image_base_url.size());

        break;
      }
    }
  }

  progress_bar_amount += 1.f; // Done getting the path to the cover art image

  // Download to temp file

  progress_bar_doing_string = "Connecting to '" + image_base_url + "'";

  std::cout << "[download.cpp] Cover art: '" + image_base_url + cover_art_path + "'\n";

  httplib::Client img_dl_cli(image_base_url);
  auto img_dl_res = img_dl_cli.Get(cover_art_path);

  progress_bar_doing_string = "Writing cover art to temporary file";

  if (img_dl_res && img_dl_res->status == 200) {
    std::ofstream temp_file(temp_file_path, std::ios::binary);

    if (!temp_file) {
      throw std::runtime_error("Failed to write temporary cover art image.");
      return false;
    }

    std::string img_data = img_dl_res->body;
    temp_file << img_data;

    std::cout << "[download.cpp] Wrote " << img_data.size() << "B to " << temp_file_path << "\n";

    temp_file.close();
  } else {
    std::cout << "[download.cpp] Failed to get cover art data (url='" << image_base_url << cover_art_path << "')\n";
    if (img_dl_res) {
      std::cout << "    Status code: " << img_dl_res->status << "\n";
    } else {
      std::cout << "    Response is a null ptr\n";
    }
  }

  progress_bar_amount += 1.f; // Done writing cover art image to temp file

  if (!_resize_cover_art(temp_file_path, new_base + ".png", 1000, 1000)) return false;
  if (!_resize_cover_art(temp_file_path, new_base + ".small.png", 100, 100)) return false;

  progress_bar_doing_string = "Downloading song"; // Most likely after _download_cover_art completes _yt_dlp_download_song_from_query is still running

  std::remove(temp_file_path.c_str());

  std::cout << "[download.cpp] Removed temporary file " << temp_file_path << "\n";

  return true;
}

bool _download_song_from_query(const std::u32string& query, size_t new_id) {
  if (pause_main_input_handling) return false; // Exit if a download is ongoing

  pause_main_input_handling = true;

  // Set progress bar
  progress_bar_string = "Downloading...";
  progress_bar_amount = 1.f;
  progress_bar_total = 10.f; // 3 in download_song_from_query
                             // |-> 3 in _download_cover_art
                             //     |-> 2 in _resize_cover_art (normal)
                             //     |-> 2 in _resize_cover_art (small)

  std::cout << "Info: Attempting to download song from query '" << u32_to_utf8(query) << "'.\n";

  progress_bar_doing_string = "Downloading song file and metadata";
  if (!_yt_dlp_download_song_from_query(yt_dlp_path, u32_to_utf8(query), new_id)) return false;
  std::cout << "Info: Downloaded song file and metadata for query '" << u32_to_utf8(query) << "'.\n";
  progress_bar_amount += 1.f; // Done with downloading song file and metadata from query

  progress_bar_doing_string = "Downloading song cover";
  if (!_download_cover_art(u32_to_utf8(get_song_title(new_id)), u32_to_utf8(get_song_artist(new_id)), new_id)) return false;
  std::cout << "Info: Downloaded song cover art for id '" << new_id << "'.\n";
  progress_bar_amount += 1.f; // Done with downloading song cover from query

  progress_bar_doing_string = "";
  progress_bar_string = "";

  return true;
}

std::string find_yt_dlp() {
  std::cout << "[INFO] Checking if yt-dlp exists on the system\n";

  std::string yt_dlp_path = "yt-dlp";
  if (!std::filesystem::exists(get_yt_dlp_downloaded_path()) &&
      !std::system("yt-dlp --version >"
    #ifdef _WIN32
      "nul 2>nul"
    #else
      "/dev/null 2>&1"
    #endif
    ) == 0) {
    std::cout << "[INFO] yt-dlp was not found on the system and will be downloaded.\n";

    progress_bar_doing_string = "Downloading yt-dlp";

    yt_dlp_path = get_yt_dlp_downloaded_path();
    if (!_download_file(get_yt_dlp_download_url(), yt_dlp_path)) {
      throw std::runtime_error("Failed to download the yt-dlp binary form '" + yt_dlp_path + "'. Consider installing yt-dlp yourself systemwide.");
    }
    #ifndef _WIN32
    // On POSIX like systems also chmod +x the file
    system(("chmod +x " + yt_dlp_path).c_str());
    #endif
  }

  return yt_dlp_path;
}

void _download_from_mf(std::string input_string, std::atomic<bool>& success, std::string& query, int& new_id) {
  new_id = get_next_avaliable_song_id();

  // Start the song audio download immediately (since yt-dlp performs the same search as the autocomplete suggestion)
  std::thread download_song_thread([&](){
    success = _yt_dlp_download_song_from_query(
      yt_dlp_path,
      input_string,
      new_id
    );
    progress_bar_amount += 1.f;
  });

  MultistateFuture<AutocompleteResult> mf;
  mf.launch([input_string](){return get_search_autocomplete_ytdlp(input_string);}, 1);

  // Wait for the get_search_autocomplete_yt_music to complete so the cover art download can start
  mf.wait();
  auto result = mf.get();
  if (!result || result.value().pairs.size() == 0) {
    return; // Don't download if there is no result
  }
  auto title_artist_pair = result.value().pairs[0];
  query = title_artist_pair.first + " " + title_artist_pair.second;

  progress_bar_amount += 1; // Got query and title_artist_pair

  std::thread download_details_thread([&](){
    success = _download_cover_art(title_artist_pair.first, title_artist_pair.second, new_id);
    progress_bar_amount += 1.f;
  });

  download_song_thread.join();
  download_details_thread.join();
}

void download_from_search(InputComponent* component) {
  if (pause_main_input_handling) return; // Exit if a download is ongoing

  pause_main_input_handling = true;

  if (yt_dlp_path.empty()) {
    std::cout << "[ERROR] Can't download since yt-dlp doesn't exist on the system.\n";
    return;
  }

  auto _reset_progress_bar = [](){
    progress_bar_amount = progress_bar_total;
    progress_bar_doing_string = "";
    progress_bar_string = "";

    pause_main_input_handling = false;
  };

  progress_bar_string = "Downloading...";
  progress_bar_amount = 1.f;
  progress_bar_total = 11.f;

  progress_bar_doing_string = "Getting song name and title";

  std::thread download_thread([=](){
    auto playlist_sel = std::get<MenuData::PlaylistSelector>(menu_data.data);
    std::atomic<bool> success = false;
    std::string query;
    int new_id;

    _download_from_mf(u32_to_utf8(playlist_sel.data->search->get_input_string()), success, query, new_id);

    // Reset progress bar
    _reset_progress_bar();

    if (!success) {
      std::cout << "[ERROR] Failed to download '" << query << "'.\n";
      remove_song(new_id);
    } else {
      component->force_input_refresh(); // Reset the search (so the new downloaded song is shown)
    }
  });
  download_thread.detach();
}

MultistateFuture<AutocompleteResult> get_search_autocomplete(const std::u32string& query) {
  MultistateFuture<AutocompleteResult> mf;
  mf.launch([query](){return get_search_autocomplete_yt_music(u32_to_utf8(query));}, 1);
  mf.launch([query](){return get_search_autocomplete_ytdlp(u32_to_utf8(query));}, 2);

  return mf;
}

