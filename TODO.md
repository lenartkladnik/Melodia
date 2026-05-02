# bugs
- ~start song is always the same song_id~
- ~hand cursor persists after it is activated by a queue element~
- flickering when moving the queue items
- sometimes the queue items go into the wrong slot
- ~mouse cursor flickers when hovering over play icon and moving~
- ~go back to playlist selector button is unreliable, since it checks for clicks outside event loop~
- ~the number of items in playlist is wrong~
- ~in large playlist the dragging item is invisible~
- attempts loading songs that don't exist and are not in any playlist?
- ~play playlist button is unreliable~
- ~sometimes the wrong song plays when selecting a playlist~
- ~fix random crash related to copying StaticPlayerData~

# hover over song in queue
- ~change duration to ...~
- if title is truncated and title is hovered show the whole title with a darker background behind it (like alt in browsers)

# current playing song
- ~darker background~

# title, artist
- ~max 26 chars (-3 + ...)~
- ~main player (under the main cover art) char limit~
  - ~artist: 53 chars (50 + ...)~
  - ~title: 45 chars (42 + ...)~

# queue
- scrolling in the queue
- scrolling when dragging in the queue
- shuffle toggle button
- allow reordering of songs (in the playlist file as well) - only when shuffle is off

# playlist selector
- if there was a song playing when selected keep a small player at the bottom of the screen
- ~search at the top for adding new songs~ (~download from yt~, adding + removing songs from playlists)
- ~10px rounded rectangles with the playlist name, length (in h:m:s if possible else total items), playlist thumbnail, edit playlist (name, ...)~
- ~create new playlist, '+' button next to search bar on the right side~
- make create new playlist button function

# player
- make favorite button function
- make trash button function
- make edit button function
- figure out what to do with the remaining button (index 1)

# downloading
- ~progress bar for download (eg. small popup window)~
- better error handling (cleanup, exit on error, ...)

- command line argument for adding song by title

- Python script for:
  - adding songs from Spotify playlist
  - adding songs from Youtube playlist

# general
- add song removal and propagate song removal (check every playlist and remove the song if it is included)
- use click_events to apply hover changes
- use setCenter to position elements
- force resize aspect ratio
- create necessary directories if they don't exist
- gracefully handle exceptions
- create some sort of installer

# building
- add BUILDING.md
