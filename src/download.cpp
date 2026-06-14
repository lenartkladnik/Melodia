#include <fstream>
#include "../external/lib/httplib.h"
#include "../external/lib/nlohmann/json.hpp"
#include "include/data.hpp"

#ifndef _WIN32
  #define STB_IMAGE_IMPLEMENTATION
  #define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "../external/lib/stb/stb_image.h"
#include "../external/lib/stb/stb_image_resize2.h"
#include "../external/lib/stb/stb_image_write.h"

using json = nlohmann::json;

bool _yt_dlp_download_song_from_query(int new_id, const std::string& query) {
  auto new_base = base_music_path_data + std::to_string(new_id);
  std::string yt_dlp_args = " -I 1 \"https://music.youtube.com/search?q=" + query + "\" -xciw -f \"bestaudio/best\" --audio-format mp3 --audio-quality 0 --print-to-file \"%(artist)s\" " + new_base + ".artist --print-to-file \"%(track)s\" " + new_base + ".title -o \"" + new_base + "\".mp3";

  std::string progs_path = "";
  std::string dlp_path = "";

  #ifdef __MINGW32__
  progs_path = ".\\external\\programs";
  dlp_path = progs_path + "\\yt-dlp.exe";
  #else
  progs_path = "./external/programs";
  dlp_path = progs_path + "/yt-dlp";
  #endif

  std::cout << progs_path << "\n" << dlp_path << "\n" << yt_dlp_args << "\n";

  std::string command = dlp_path + " --ffmpeg-location \"" + progs_path + "\"" + yt_dlp_args;

  std::cout << "Calling system with '" << command << "'\n";

  return system(command.c_str()) == 0;
}

bool _resize_cover_art(const std::string& temp_file_path, const std::string& output, int target_w, int target_h) {
  progress_bar_doing_string = "Resizing cover art";
  progress_bar_amount += 1.f; // Done downloading / started resizing the cover art image

  int w, h, channels;
  unsigned char* data = stbi_load(temp_file_path.c_str(), &w, &h, &channels, 0);

  if (!data) {
    std::cerr << "Error: Failed to decode image from " << temp_file_path << std::endl;
    return false;
  }

  std::vector<unsigned char> resized(target_w * target_h * channels);

  stbir_pixel_layout layout;
  switch (channels) {
    case 1: layout = STBIR_1CHANNEL; break;
    case 2: layout = STBIR_2CHANNEL; break;
    case 3: layout = STBIR_RGB; break;
    case 4: layout = STBIR_RGBA; break;
    default:
      std::cerr << "Error: Unsupported channel count for cover art image." << std::endl;
      stbi_image_free(data);
      return false;
  }

  stbir_resize(
    data, w, h, 0,
    resized.data(), target_w, target_h, 0,
    layout,
    STBIR_TYPE_UINT8,
    STBIR_EDGE_CLAMP,
    STBIR_FILTER_DEFAULT
  );

  progress_bar_doing_string = "Writing resized cover art";
  progress_bar_amount += 1.f; // Done resizing the cover art image

  if (!stbi_write_png(output.c_str(), target_w, target_h, channels, resized.data(), target_w * channels)) {
    std::cerr << "Error: Failed to write cover art image." << std::endl;
    stbi_image_free(data);
    return false;
  }

  stbi_image_free(data);

  std::cout << "Info: Resized cover art image" << std::endl;
  progress_bar_amount += 1.f; // Done writing cover art image

  return true;
}

// urlencode - Source:
// https://gist.github.com/litefeel/1197e5c24eb9ec93d771

void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2) {
  hex1 = c / 16;
  hex2 = c % 16;
  hex1 += hex1 <= 9 ? '0' : 'a' - 10;
  hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

std::string urlencode(std::string s) {
  const char *str = s.c_str();
  std::vector<char> v(s.size());
  v.clear();
  for (size_t i = 0, l = s.size(); i < l; i++) {
    char c = str[i];
    if ((c >= '0' && c <= '9') ||
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
      c == '*' || c == '\'' || c == '(' || c == ')') {
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

// =====================================================

bool _download_cover_art(int new_id) {
  progress_bar_doing_string = "Downloading cover art";

  std::string main_base_url = "https://www.last.fm";
  std::string image_base_url = "https://lastfm.freetls.fastly.net";

  std::string new_base = base_music_path_data + std::to_string(new_id);
  std::string temp_file_path = ".cover_art.png.tmp";

  std::string artist_string = get_song_artist(new_id);
  std::string title_string = get_song_title(new_id);

  std::string query = "/search/albums?q=" + urlencode(title_string + " by " + artist_string);

  progress_bar_doing_string = "Connecting to '" + main_base_url + "'";

  std::cout << "[download.cpp] Results page: '" + main_base_url + query + "'" << std::endl;

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

        // Parse the url out of: '            src="https://lastfm.freetls.fastly.net/i/u/64s/<id>.jpg"'
        size_t start = line.find("src=");
        size_t url_start = start + 5;
        cover_art_path = line.substr(url_start, line.size() - url_start - 1);

        // Change the size to be maximally large
        std::string default_size = "64s";
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

  std::cout << "[download.cpp] Cover art: '" + image_base_url + cover_art_path + "'" << std::endl;

  httplib::Client img_dl_cli(image_base_url);
  auto img_dl_res = img_dl_cli.Get(cover_art_path);

  progress_bar_doing_string = "Writing cover art to temporary file";

  if (img_dl_res && img_dl_res->status == 200) {
    std::ofstream temp_file(temp_file_path, std::ios::binary);

    if (!temp_file) {
      std::cerr << "Error: Failed to write temporary cover art image." << std::endl;
      return false;
    }

    temp_file << img_dl_res->body;

    temp_file.close();
  }

  progress_bar_amount += 1.f; // Done writing cover art image to temp file

  if (!_resize_cover_art(temp_file_path, new_base + ".png", 1000, 1000)) return false;
  if (!_resize_cover_art(temp_file_path, new_base + ".small.png", 100, 100)) return false;

  progress_bar_doing_string = "Removing temporary file";

  std::remove(temp_file_path.c_str());

  return true;
}

bool download_song_from_query(const std::string& query) {
  if (pause_main_input_handling) return false; // Exit if a download is ongoing

  pause_main_input_handling = true;

  std::cout << "Info: Attempting to download song from query '" << query << "'." << std::endl;

  // Set progress bar
  progress_bar_string = "Downloading...";
  progress_bar_amount = 0.f;
  progress_bar_total = 12.f; // 3 in download_song_from_query
                             // |-> 3 in _download_cover_art
                             //     |-> 3 in _resize_cover_art (normal)
                             //     |-> 3 in _resize_cover_art (small)

  progress_bar_doing_string = "Getting the max id";

  int max_id = -1;

  for (const auto& entry : std::filesystem::directory_iterator(base_music_path_data)) {
    int id = std::stoi(entry.path().stem().string());

    if (id > max_id)
      max_id = id;
  }

  auto new_id = max_id + 1;

  progress_bar_amount += 1.f; // Done with getting the max_id

  progress_bar_doing_string = "Downloading song file and metadata";
  if (!_yt_dlp_download_song_from_query(new_id, query)) return false;
  std::cout << "Info: Downloaded song file and metadata for query '" << query << "'." << std::endl;
  progress_bar_amount += 1.f; // Done with downloading song file and metadata from query

  progress_bar_doing_string = "Downloading song cover";
  if (!_download_cover_art(new_id)) return false;
  std::cout << "Info: Downloaded song cover art for id '" << new_id << "'." << std::endl;
  progress_bar_amount += 1.f; // Done with downloading song cover from query

  progress_bar_doing_string = "Done!";
  progress_bar_string = "";

  return true;
}

bool _archive_org_download_song_from_query(int new_id, const std::string& title, const std::string& artist) { // Not viable - poor songs
  std::cout << "Calling with t:" << title << ", a:" << artist << "\n";
  auto base_url = "https://archive.org";
  httplib::Client archive_org(base_url);

  // Try the exact search with title and artist
  auto accurate_search_path = "/advancedsearch.php?q=title:%22" + urlencode(title) + "%22%20AND%20creator:%22" + urlencode(artist) + "%22&output=json&rows=100";
  auto accurate_res = archive_org.Get(accurate_search_path);

  if (accurate_res && accurate_res->status == 200) {
    json resp_data = json::parse(accurate_res->body)["response"];

    if (resp_data["numFound"] > 0) {
      auto match = resp_data["docs"][0];

      auto idf = match["identifier"].dump();

      auto files_path = "/download/" + idf.substr(1, idf.size() - 2);
      std::cout << files_path << "\n";
      auto files_res = archive_org.Get(files_path);

      if (files_res && files_res->status == 200) {
        std::cout << files_res->body << "\n";

        return true;
      }
    }
  }

  // Try the more general search (title + artist in one search string)
  auto simpler_search_path = "/advancedsearch.php?q=%22" + urlencode(title) + "%20" + urlencode(artist) + "%22&output=json&rows=100";
  auto simpler_res = archive_org.Get(simpler_search_path);

  if (simpler_res && simpler_res->status == 200) {
    std::cout << simpler_res->body << "\n";

    return true;
  }

  return false;
}
