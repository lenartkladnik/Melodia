import json

def _csv_escape(s: str) -> str:
    return '"' + s.replace('"', '""') + '"'

class Response:
    def __init__(self, title: str, artist: str) -> None:
        self.title = title
        self.artist = artist

    def __str__(self) -> str:
        return _csv_escape(self.title) + "," + _csv_escape(self.artist)

def from_spotify(id: str):
    from spotapi import PublicPlaylist

    playlist = PublicPlaylist(id)

    responses = []

    resp = playlist.get_playlist_info()
    for item in resp["data"]["playlistV2"]["content"]["items"]:
        item_data = item["itemV3"]["data"]["identityTrait"]

        artist = item_data["contentHierarchyParent"]["identityTrait"]["name"]
        title = item_data["name"]

        responses.append(str(Response(title, artist)))

    print(responses)

def from_ytmusic(id: str):
    ...

from_spotify("2bYs7IlWQqyuLHmcd4bt9g")
