#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

#include "include/utils.hpp"

#include "../external/lib/webview/webview.h"
#include "../external/lib/nlohmann/json.hpp"

#ifdef __linux__
#include <gtk/gtk.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#if defined(__APPLE__)
#import <AppKit/NSWindow.h>
#endif

std::vector<std::string> scrape_playlist(const std::string& query) {
  #ifdef __linux__
  setenv("WEBKIT_DISABLE_DMABUF_RENDERER", "1", 1); // This is needed in Wayland (if this isn't set the app crashes)
  setenv("GDK_BACKEND", "x11", 1); // This allows the window icon to be hidden in Wayland
  #endif

  std::string url = query;
  if (url.find("https://") != 0) {
    url = "https://" + url;
  }

  std::vector<std::string> collected;

  webview::webview w(true, nullptr);
  w.set_title("Melodia - Playlist scraper");
  w.set_size(1, 1, WEBVIEW_HINT_NONE);

  // Hide icon in taskbar
  #ifdef __linux__
  auto window_res = w.window();
  if (window_res.has_value()) {
    void* raw_window = window_res.value();
    if (raw_window) {
      GtkWindow* gtk_win = static_cast<GtkWindow*>(raw_window);
      gtk_window_set_skip_taskbar_hint(gtk_win, TRUE);
    }
  }
  #endif
  #if defined(_WIN32)
  auto window_res = w.window();
  if (window_res.has_value()) {
    HWND hwnd = static_cast<HWND>(window_res.value());
    if (hwnd) {
      LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
      SetWindowLongPtr(hwnd, GWL_EXSTYLE, (exStyle | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW);
    }
  }
  #endif
  #if defined(__APPLE__)
  auto window_res = w.window();
  if (window_res.has_value()) {
    void* native_win = window_res.value();
    if (native_win) {
      id window = static_cast<id>(native_win);
      [window setCollectionBehavior:NSWindowCollectionBehaviorTransient | NSWindowCollectionBehaviorIgnoresCycle];
    }
  }
  #endif

  // This will be called by the js when it finishes scraping
  w.bind("returnSongs", [&](const std::string&, const std::string& req, void* arg) {
    auto json_data = nlohmann::json::parse(req)[0];

    for (const auto& item : json_data) {
      collected.push_back('"' + escape_csv(item["title"]) + "\",\"" + escape_csv(item["artist"]) + '"');
    }

    webview::webview* w_ptr = static_cast<webview::webview*>(arg);
    w_ptr->terminate();
    return "";
  }, &w);

  if (url.find("open.spotify.com") != std::string::npos) {
    w.init(R"(
      function sleep(ms) {
        return new Promise((resolve) => setTimeout(resolve, ms));
      }

      async function fromSpotify() {
        const songContainerCssPath = '[data-testid="tracklist-row"] div[role="gridcell"][aria-colindex="2"]';
        const titleCssPath = '[data-testid="internal-track-link"] div';
        const artistCssPath = 'a[href*="/artist/"]';

        const results = [];
        const seenSongs = new Set();

        async function waitForContainer(timeout = 10000) {
          const start = Date.now();
          while (Date.now() - start < timeout) {
            const el = document.querySelector(songContainerCssPath);
            if (el) return el;
            await sleep(100);
          }
          return null;
        }

        const testSongContainer = await waitForContainer(10000);
        if (!testSongContainer) {
          returnSongs([]);
          return;
        }

        let lastBottomSongKey = null;
        let stagnationCount = 0;

        while (true) {
          const allSongs = Array.from(document.querySelectorAll(songContainerCssPath));

          for (const songContainer of allSongs) {
            if (!document.body.contains(songContainer)) continue;

            const titleEl = songContainer.querySelector(titleCssPath);
            const artistEl = songContainer.querySelector(artistCssPath);

            if (!titleEl || !artistEl) continue;

            const title = titleEl.textContent.trim();
            const artist = artistEl.textContent.trim();
            const key = title + "," + artist;

            if (!seenSongs.has(key)) {
              seenSongs.add(key);
              results.push({"title": title, "artist": artist});
            }
          }

          if (results.length > 0) {
            const currentBottomKey = results[results.length - 1].title + "," + results[results.length - 1].artist;
            if (currentBottomKey === lastBottomSongKey) {
              stagnationCount++;
              sleep(1000);
              if (stagnationCount > 3) break;
            } else {
              stagnationCount = 0;
              lastBottomSongKey = currentBottomKey;
            }
          }

          if (allSongs.length > 0) {
            allSongs[allSongs.length - 1].scrollIntoView(true);
          }

          await sleep(500);
        }

        returnSongs(results);
      }

      window.addEventListener('DOMContentLoaded', () => {
        setTimeout(fromSpotify, 2000);
      });
    )");
  } else if (url.find("music.youtube.com") != std::string::npos) {
    w.init(R"(
      async function fromYtmusic() {
        const rejectButtonCssPath = '[action="https://consent.youtube.com/save"]';
        const contentCssPath = 'body ytmusic-app ytmusic-app-layout#layout.style-scope.ytmusic-app div#content.style-scope.ytmusic-app ytmusic-browse-response#browse-page.style-scope.ytmusic-app div.background-gradient.style-scope.ytmusic-browse-response div#content-wrapper.style-scope.ytmusic-browse-response div#contents.style-scope.ytmusic-browse-response ytmusic-two-column-browse-results-renderer.style-scope.ytmusic-browse-response div#secondary.style-scope.ytmusic-two-column-browse-results-renderer ytmusic-section-list-renderer.description.scroller.scroller-on-hover.style-scope.ytmusic-two-column-browse-results-renderer div#contents.style-scope.ytmusic-section-list-renderer ytmusic-playlist-shelf-renderer.style-scope.ytmusic-section-list-renderer.fullbleed div.style-scope.ytmusic-playlist-shelf-renderer div#contents.style-scope.ytmusic-playlist-shelf-renderer';

        const results = [];
        const seenSongs = new Set();

        async function waitForSelector(selector, timeout = 10000) {
          const start = Date.now();
          while (Date.now() - start < timeout) {
            const el = document.querySelector(selector);
            if (el) return el;
            await sleep(100);
          }
          returnSongs([]);
          return;
        }

        // Dismiss cookie prompt
        try {
          const rejectButton = await waitForSelector(rejectButtonCssPath, 5000);
          rejectButton.closest('form')?.requestSubmit
            ? rejectButton.closest('form').requestSubmit()
            : rejectButton.click();
        } catch (e) {
          // No cookie prompt - continue
        }

        const content = await waitForSelector(contentCssPath, 10000);

        const items = content.querySelectorAll('ytmusic-responsive-list-item-renderer');
        for (const div of items) {
          const songContainer = div.querySelector(
            'div[class="flex-columns style-scope ytmusic-responsive-list-item-renderer"]'
          );
          if (!songContainer) continue;

          const formattedStrings = songContainer.querySelectorAll('yt-formatted-string');
          const title = formattedStrings[0]?.textContent.trim();
          const artist = formattedStrings[1]?.textContent.trim();
          const key = title + "," + artist;

          if (!seenSongs.has(key)) {
            seenSongs.add(key);
            results.push({"title": title, "artist": artist});
          }
        }

        returnSongs(results);
      }

      window.addEventListener('DOMContentLoaded', () => {
        setTimeout(fromYtmusic, 2000);
      });
  )");
  } else {
    throw std::runtime_error("Invalid URL.\n");
  }

  w.navigate(url);
  w.run();

  return collected;
}
