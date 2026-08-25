# Known bugs
- ~start song is always the same song_id~
- ~hand cursor persists after it is activated by a queue element~
- flickering when moving the queue items
- sometimes the queue items go into the wrong slot
- ~mouse cursor flickers when hovering over play icon and moving~
- ~go back to playlist selector button is unreliable, since it checks for clicks outside event loop~
- ~the number of items in playlist is wrong~
- ~in large playlist the dragging item is invisible~
- attempts loading songs that don't exist and are not in any playlist? (can't replicate)
- ~play playlist button is unreliable~
- ~sometimes the wrong song plays when selecting a playlist~
- ~fix random crash related to copying StaticPlayerData~
- when switching songs the song that was playing gets put into the same position as the newly selected song
- player UI breaks when resized
- create necessary directories if they don't exist
- gracefully handle exceptions
- in the player menu queue a song cannot be placed at the very end of the queue when dragging

# New features / changes

## important
- download song and/or song data while the user is still inputting the search phrase so it is ready as soon as the user confirms search

## general
- create an installer

## code

### coding style
- use m_ prefix for member variables
- use hover_event(s) to apply hover changes
- move constructors and deconstructors to the top of class definitions
- ~make utils.cpp~
- refactor functions into utils.cpp
- ~InputComponent should include its own input_string, prev_input_string, cursor_pos, ...~

### convention
- use emplace_back instead of push_back on std::vector
- use \n instead of std::endl when writing to stdout

### maybe
- use setCenter to position elements
- force resize aspect ratio

## input

### string
- scroll when the string is longer than what can be displayed

## songs

### removing
- add song removal and propagate song removal (check every playlist and remove the song if it is included)

### downloading
- mass downloading with multithreading
- ~progress bar for download (eg. small popup window)~
- better error handling (cleanup, exit on error, ...)

- command line argument for adding song by title (maybe)

- adding all songs from Spotify playlist
- adding all songs from Youtube playlist

- download yt-dlp binary instead of storing it
- refactor download.cpp into more files (eg.: postprocess.cpp)

## song container(s)

### hover
- ~change duration to ...~
- if title is truncated and title is hovered show the whole title with a darker background behind it (like alt in browsers)

### dragging
- when dragging don't visually place the cursor at the center of the dragged item, instead place it at the position where the user first clicked on the item

### title, artist
- ~max 26 chars (-3 + ...)~
- ~main player (under the main cover art) char limit~
  - ~artist: 53 chars (50 + ...)~
  - ~title: 45 chars (42 + ...)~

## queue
- ~scrolling in the queue~
- scrolling when dragging in the queue
- shuffle toggle button
- allow reordering of songs (in the playlist file as well) - only when shuffle is off

## playlist selector
- if there was a song playing when selected keep a small player at the bottom of the screen
- ~search at the top for adding new songs~ (~download from yt~, adding + removing songs from playlists)
- ~10px rounded rectangles with the playlist name, length (in h:m:s if possible else total items), playlist thumbnail, edit playlist (name, ...)~
- ~create new playlist, '+' button next to search bar on the right side~
- make create new playlist button function

## player
- make favorite button function
- make trash button function
- make edit button function
- figure out what to do with the remaining button (index 1)

## building
- add BUILDING.md
- dependency list for building all
  - scons
  - gcc
  - clang
  - mingw-w64
  - mingw-w64-icu

  - #### Arch Linux
  ```bash
  pacman -S scons
  pacman -S gcc
  pacman -S clang
  pacman -S mingw-w64 # Select all
  yay -S mingw-w64-icu
  ```
