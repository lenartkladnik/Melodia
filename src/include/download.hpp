#ifndef DOWNLOAD_HPP
#define DOWNLOAD_HPP

#include "data.hpp"
#include "components.hpp"
#include "utils.hpp"

std::string find_yt_dlp();
bool download_from_search(InputComponent*);
MultistateFuture<AutocompleteResult> get_search_autocomplete(const std::u32string& query);

#endif
