import 'dart:convert';
import 'dart:math';
import 'package:audio_service/audio_service.dart';
import 'package:audio_session/audio_session.dart';
import 'package:flutter/foundation.dart' show kIsWeb, defaultTargetPlatform, TargetPlatform, debugPrint;
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:just_audio/just_audio.dart';
import 'package:just_audio_media_kit/just_audio_media_kit.dart';

const apiUrl = "http://lenart:5400";

var shuffleSeed = Random().nextInt(1024);
var doShuffle = true;
var doLoop = false;

List<String> determinedQueue = ["13", "42", "312"];

List<String> alreadyPlayed = [];
List<bool> alreadyPlayedFromQueue = [];

int playedFromQueueCount = 0;
int generatedQueueStart = 0;

int playlistPositionAfter(int currentIndex, int n) => generatedQueueStart + currentIndex - playedFromQueueCount + n;

Future<String> getSongToBeDisplayed(String playlist, int currentIndex, int offset) {
  assert(offset >= 1);
  final queued = determinedQueue.length;
  if (offset <= queued) return Future.value(determinedQueue[offset - 1]);

  return fetchNextSongId(playlist, playlistPositionAfter(currentIndex, offset - queued));
}

Future<String> getSongToBePlayed(String playlist, int targetIndex) {
  if (determinedQueue.isNotEmpty) return Future.value(determinedQueue.first);

  return fetchNextSongId(playlist, playlistPositionAfter(targetIndex - 1, 1));
}

final waitWidget = const Center(child: CircularProgressIndicator());
final nullWidget = const SizedBox.shrink();
Widget errorWidget(VoidCallback callback) {return Center(child: IconButton(onPressed: callback, tooltip: "Retry", iconSize: 32, icon: const Icon(Icons.refresh_rounded)));}

Widget songListItemSkeleton(BuildContext context) {
  return Material(type: MaterialType.transparency, child:
    Container(
      height: 120,
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(14),
        color: Theme.of(context).colorScheme.surfaceContainerLow
      ),
      child:
        Row(children: [
          SizedBox(width: 10),
          Container(
            width: 100,
            height: 100, 
            decoration: BoxDecoration(
              color: Color.fromARGB(10, 0, 0, 0),
              borderRadius: BorderRadius.circular(16),
            ),
          ),
          SizedBox(width: 10),
          Expanded(
            child: Column(crossAxisAlignment: CrossAxisAlignment.start, mainAxisAlignment: MainAxisAlignment.center, children: [
              FractionallySizedBox(
                widthFactor: 0.8,
                child: Container(
                  height: 20,
                  decoration: BoxDecoration(
                    color: Color.fromARGB(10, 0, 0, 0),
                    borderRadius: BorderRadius.circular(8)
                  ),
                ),
              ),
              SizedBox(height: 10),
              FractionallySizedBox(
                widthFactor: 0.6,
                child: Container(
                  height: 20,
                  decoration: BoxDecoration(
                    color: Color.fromARGB(10, 0, 0, 0),
                    borderRadius: BorderRadius.circular(8)
                  ),
                ),
              ),
            ])
          ),
          SizedBox(width: 8)
        ]),
    )
  );
}

double textWidth(BuildContext context, String text, TextStyle style) {
  final painter = TextPainter(
    text: TextSpan(text: text, style: style),
    maxLines: 1,
    textDirection: TextDirection.ltr,
    textScaler: MediaQuery.textScalerOf(context),
  )..layout();
  return painter.width;
}

late final MelodiaAudioHandler audioHandler;

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  if (!kIsWeb) JustAudioMediaKit.ensureInitialized();

  final useAudioService = !kIsWeb &&
      (defaultTargetPlatform == TargetPlatform.android ||
       defaultTargetPlatform == TargetPlatform.iOS ||
       defaultTargetPlatform == TargetPlatform.macOS);

  if (useAudioService) {
    audioHandler = await AudioService.init<MelodiaAudioHandler>(
      builder: () => MelodiaAudioHandler(),
      config: const AudioServiceConfig(
        androidNotificationChannelId: "com.lenartkladnik.melodia.playback",
        androidNotificationChannelName: "Melodia playback",
        androidNotificationOngoing: true,
      ),
    );
  } else {
    audioHandler = MelodiaAudioHandler();
  }

  runApp(const Melodia());
}

class Melodia extends StatefulWidget {
  const new({super.key});

  @override
  State<Melodia> createState() => _MelodiaState();
}

class _MelodiaState extends State<Melodia> {
  PlayerController playerController = PlayerController();
  PlayerWidget? playerWidget;

  @override void initState() {
    super.initState();
    playerWidget = PlayerWidget(controller: playerController);
    playerController.onClose = switchToPlaylistSelector;
  }

  void switchToPlaylistSelector() {
    setState(() {
      playerController.toMiniPlayer();
    });
  }

  void switchToPlayer(String playlist) {
    setState(() {
      playerController.loadPlaylist(playlist);
      playerController.toFullPlayer();
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: "Melodia",
      theme: ThemeData(fontFamily: "Roboto"),
      home: Scaffold(
        body: SafeArea(
          child: Stack(
            children: [
              PlaylistSelectorWidget(switchToPlayer: switchToPlayer),
              playerWidget ?? nullWidget,
            ]
          ),
        )
      )
    );
  }
}

class PlaylistEnd implements Exception {
  @override
  String toString() => "End of playlist";
}

Future<dynamic> _getJson(Uri uri, {bool notFoundIsNull = false}) async {
  final response = await http.get(uri).timeout(const Duration(seconds: 15));
  if (notFoundIsNull && response.statusCode == 404) return null;
  if (response.statusCode != 200) {
    throw http.ClientException("HTTP ${response.statusCode}", uri);
  }
  return jsonDecode(utf8.decode(response.bodyBytes));
}

Future<String> fetchNextSongId(String playlist, int index) async {
  if (index < 0) throw PlaylistEnd();

  final json = await _getJson(Uri.parse("$apiUrl/playlist/next").replace(
    queryParameters: {
      "playlist_name": playlist,
      "index": index.toString(),
      "shuffle": doShuffle.toString(),
      "seed": shuffleSeed.toString(),
      "loop": doLoop.toString()
    }
  ));

  final id = json is Map ? json["song_id"] : null;
  if (id == null) throw PlaylistEnd();
  return id.toString();
}

Future<List<String>> fetchAllPlaylists() async {
  final json = await _getJson(Uri.parse("$apiUrl/playlists"));
  return (json as List<dynamic>).cast<String>();
}

Future<int> fetchPlaylistSize(String playlist) async {
  final json = await _getJson(Uri.parse("$apiUrl/playlist/size").replace(
    queryParameters: {"playlist_name": playlist}
  ));
  return json["size"] as int;
}

class Playlist {
  final String image;

  new({
    required this.image,
  });
  
  factory Playlist.fromJson(Map<String, dynamic> json) {
    return Playlist(
      image: json['image'] as String
    );
  }
}

Future<Playlist> fetchPlaylist(String playlist) async {
  final json = await _getJson(Uri.parse("$apiUrl/playlist").replace(
    queryParameters: {"playlist_name": playlist}
  ));
  return Playlist.fromJson(json);
}

class PlaylistSelectorWidget extends StatefulWidget {
  final Function(String) switchToPlayer;

  const new({super.key, required this.switchToPlayer});

  @override
  State<PlaylistSelectorWidget> createState() => _PlaylistSelectorWidgetState();
}

class _PlaylistSelectorWidgetState extends State<PlaylistSelectorWidget> {
  Future<List<String>> allPlaylistsFuture = fetchAllPlaylists();
  final Map<String, Future<Playlist>> _playlistFutures = {};

  Future<Playlist> _futureFor(String name) =>
      _playlistFutures.putIfAbsent(name, () => fetchPlaylist(name));

  void _retryAllPlaylistsFuture() {
    setState(() {
      _playlistFutures.clear();
      allPlaylistsFuture = fetchAllPlaylists();
    });
  }

  void _retryPlaylist(String name) {
    setState(() {
      _playlistFutures[name] = fetchPlaylist(name);
    });
  }

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(
      future: allPlaylistsFuture,
      builder: (context, snapshot) {
        if (snapshot.connectionState == ConnectionState.waiting) {
          return waitWidget;
        }

        if (snapshot.hasError) {
          return errorWidget(_retryAllPlaylistsFuture);
        }
        
        List<String>? playlists = snapshot.data;
        if (playlists == null) return waitWidget; // Just to make the editor happy

        final eachSize = 300;

        List<Widget> playlistItems = [];
        for (var playlistName in playlists) {
          playlistItems.add(
            FutureBuilder<Playlist>(
              key: ValueKey(playlistName),
              future: _futureFor(playlistName),
              builder:(context, snapshot) {
                if (snapshot.connectionState == ConnectionState.waiting) {
                  return waitWidget;
                }

                if (snapshot.hasError) {
                  return errorWidget(() => _retryPlaylist(playlistName));
                }

                final playlistData = snapshot.data;
                if (playlistData == null) return waitWidget;

                return GestureDetector(
                  onTap:() {
                    widget.switchToPlayer(playlistName);
                  },
                  child: Container(
                    decoration: BoxDecoration(
                      color: Colors.transparent
                    ),
                    child: Padding(
                    padding: EdgeInsetsGeometry.all(10),
                    child: Row(children: [
                      Container(
                        width: eachSize * 0.3,
                        height: eachSize * 0.3,
                        decoration: BoxDecoration(
                          borderRadius: BorderRadius.circular(eachSize * 0.3 / 10),
                          image: DecorationImage(
                            image: NetworkImage("$apiUrl${playlistData.image}"),
                            fit: BoxFit.cover,
                          ),
                        ),
                      ),
                      SizedBox(width: eachSize * 0.02),
                      SizedBox(
                        child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                          Text(playlistName, style: TextStyle(fontSize: 18), overflow: TextOverflow.ellipsis)
                        ])
                      )
                    ])
                  )
                ));
              }
            )
          );
        }

        return Scaffold(
          body: Padding(
            padding: EdgeInsetsGeometry.fromLTRB(4, 10, 4, 10),
            child: Container(
              decoration: BoxDecoration(
                color: Color.fromARGB(5, 0, 0, 0),
              ),
              child: Padding(
                padding: EdgeInsetsGeometry.all(8),
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.start,
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Row(
                      children: [
                        Icon(Icons.library_music_rounded),
                        SizedBox(width: 8),
                        Text("Library", style: TextStyle(fontSize: 20))
                      ]
                    ),
                    SizedBox(height: 20),
                    Expanded(child: ListView(children: playlistItems))
                  ]
                )
              )
            )
          )
        );
      },
    );
  }
}

class PlayerController {
  _PlayerWidgetState? _state;
  VoidCallback? onClose;

  void loadPlaylist(String playlist) {_state?._setPlaylist(playlist);}
  void toMiniPlayer() => _state?._setMini(true);
  void toFullPlayer() => _state?._setMini(false);
}

class PlayerWidget extends StatefulWidget {
  final PlayerController? controller;

  const new({super.key, required this.controller});

  @override
  State<PlayerWidget> createState() => _PlayerWidgetState();
}

class _PlayerWidgetState extends State<PlayerWidget> {
  bool requested = false;
  bool miniPlayer = false;

  @override
  void initState() {
    super.initState();
    widget.controller?._state = this;
    audioHandler.loadError.addListener(_onLoadError);
  }

  void _onLoadError() {
    final message = audioHandler.loadError.value;
    if (message == null || !mounted) return;
    ScaffoldMessenger.maybeOf(context)?.showSnackBar(SnackBar(content: Text(message)));
  }

  void _setMini(bool mini) => setState(() {miniPlayer = mini;});

  Future<void> _setPlaylist(String newPlaylist) async {
    setState(() {requested = true;});
    try {
      await audioHandler.loadPlaylist(newPlaylist);
    } finally {
      if (mounted) setState(() {requested = false;});
    }
  }

  @override
  Widget build(BuildContext context) {
    final close = widget.controller?.onClose;

    return ValueListenableBuilder<NowPlaying?>(
      valueListenable: audioHandler.nowPlaying,
      builder: (context, now, _) {
        if (now == null) {
          return requested ? waitWidget : nullWidget;
        }
        if (close == null) {
          return waitWidget;
        }

        return Stack(children: [
          Player(
            song: now.song,
            onFR: audioHandler.skipToPrevious,
            onFF: audioHandler.skipToNext,
            close: close,
            toFull: () => widget.controller?.toFullPlayer(),
            isMini: miniPlayer,
          ),
          if (!miniPlayer) PlayerSideMenu(nowPlaying: now.song.id, playlistName: now.playlist, playlistIndex: now.index),
        ]);
      },
    );
  }

  @override
  void dispose() {
    audioHandler.loadError.removeListener(_onLoadError);
    widget.controller?._state = null;
    super.dispose();
  }
}

class Song {
  final String artist;
  final String audio;
  final String base;
  final String image;
  final String thumbnail;
  final String title;
  final String id;

  new({
    required this.artist,
    required this.audio,
    required this.base,
    required this.image,
    required this.thumbnail,
    required this.title,
    required this.id,
  });
  
  factory Song.fromJsonWithId(Map<String, dynamic> json, String id) {
    return Song(
      artist: json['artist'] as String,
      audio: json['audio'] as String,
      base: json['base'] as String,
      image: json['image'] as String,
      thumbnail: json['thumbnail'] as String,
      title: json['title'] as String,
      id: id,
    );
  }
}

final Map<String, Future<Song?>> _songCache = {};

Future<Song?> fetchSong(String id) {
  final cached = _songCache[id];
  if (cached != null) return cached;

  final future = _fetchSong(id);
  _songCache[id] = future;
  future.catchError((Object _) {
    _songCache.remove(id);
    return null;
  });
  return future;
}

Future<Song?> _fetchSong(String id) async {
  final json = await _getJson(
    Uri.parse("$apiUrl/song").replace(queryParameters: {"song_id": id}),
    notFoundIsNull: true,
  );

  if (json == null || json is! Map || json.isEmpty) return null; // song doesn't exist
  return Song.fromJsonWithId(Map<String, dynamic>.from(json), id);
}

class NowPlaying {
  final String playlist;
  final int index;
  final Song song;

  const NowPlaying(this.playlist, this.index, this.song);
}

class MelodiaAudioHandler extends BaseAudioHandler with SeekHandler {
  static const _maxAttempts = 3;
  static const _maxSkips = 5;
  static const _maxHistory = 100;

  final AudioPlayer player = AudioPlayer();
  final ValueNotifier<NowPlaying?> nowPlaying = ValueNotifier<NowPlaying?>(null);

  final ValueNotifier<String?> loadError = ValueNotifier<String?>(null);

  String? _playlist;
  int _index = 0;
  int _loadToken = 0;
  bool _handlingCompletion = false;

  Future<Song?>? _next;
  String? _nextPlaylist;
  int? _nextIndex;

  MelodiaAudioHandler() {
    _init().catchError((e){});
  }

  Future<void> _init() async {
    player.playbackEventStream.listen(
      (_) => _broadcast()
    );
    player.playingStream.listen((_) => _broadcast());

    player.durationStream.listen((duration) {
      final item = mediaItem.value;
      if (item != null && duration != null) {
        mediaItem.add(item.copyWith(duration: duration));
      }
    });

    player.processingStateStream.listen((state) async {
      if (state != ProcessingState.completed || _handlingCompletion) return;
      _handlingCompletion = true;
      try {
        await skipToNext();
      } finally {
        _handlingCompletion = false;
      }
    });

    try {
      final session = await AudioSession.instance;
      await session.configure(const AudioSessionConfiguration.music());
    } catch (e) {
      debugPrint("Audio session setup failed: $e");
    }
  }

  void setShuffle(bool value) {
    doShuffle = value;
    _invalidatePrefetch();
  }

  void setLoop(bool value) {
    doLoop = value;
    _invalidatePrefetch();
  }

  void _invalidatePrefetch() {
    _next = null;
    _nextPlaylist = null;
    _nextIndex = null;
  }

  Future<void> loadPlaylist(String name) async {
    if (name == _playlist && nowPlaying.value != null) return;

    _playlist = name;
    _index = 0;
    _invalidatePrefetch();

    alreadyPlayed = [];
    alreadyPlayedFromQueue = [];
    playedFromQueueCount = 0;
    generatedQueueStart = 0;

    await _playIndex(0);
  }

  void _onSongStarted(Song song, {required bool replay}) {
    if (replay) {
      if (alreadyPlayed.length > 1) {
        final leftId = alreadyPlayed.removeLast();
        final leftFromQueue = alreadyPlayedFromQueue.removeLast();
        if (leftFromQueue) {
          determinedQueue.insert(0, leftId);
          playedFromQueueCount--;
        }
      }
      return;
    }

    final fromQueue = determinedQueue.isNotEmpty && determinedQueue.first == song.id;
    if (fromQueue) {
      determinedQueue.removeAt(0);
      playedFromQueueCount++;
    }

    alreadyPlayed.add(song.id);
    alreadyPlayedFromQueue.add(fromQueue);
    if (alreadyPlayed.length > _maxHistory) {
      alreadyPlayed.removeAt(0);
      alreadyPlayedFromQueue.removeAt(0);
    }
  }

  Future<Song?> _resolve(String playlist, int index) async {
    while (true) {
      final id = await getSongToBePlayed(playlist, index).timeout(const Duration(seconds: 15));
      final song = await fetchSong(id).timeout(const Duration(seconds: 15));

      if (song == null && determinedQueue.isNotEmpty && determinedQueue.first == id) {
        determinedQueue.removeAt(0);
        continue;
      }
      return song;
    }
  }

  Future<Song?> _songFor(String playlist, int index) async {
    final cached = _next;
    if (cached != null && _nextPlaylist == playlist && _nextIndex == index) {
      _next = null;
      try {
        return await cached;
      } catch (_) {}
    }
    return _resolve(playlist, index);
  }

  void _prefetch(String playlist, int index) {
    _nextPlaylist = playlist;
    _nextIndex = index;
    final future = _resolve(playlist, index);
    future.ignore();
    _next = future;
  }

  Future<void> _playIndex(int index, {String? replayId}) async {
    final playlist = _playlist;
    if (playlist == null || index < 0) return;

    final token = ++_loadToken;
    loadError.value = null;

    Song? song;
    var attempts = 0;
    var skipped = 0;
    while (song == null) {
      try {
        song = replayId != null ? await fetchSong(replayId) : await _songFor(playlist, index);
        if (token != _loadToken) return;

        if (song == null) {
          if (replayId != null) return;
          if (++skipped > _maxSkips) {
            loadError.value = "Couldn't find a playable song";
            await player.stop();
            return;
          }
          index++;
        }
      } on PlaylistEnd {
        if (token == _loadToken) await player.stop();
        return;
      } catch (e) {
        if (token != _loadToken) return;
        if (++attempts >= _maxAttempts) {
          loadError.value = "Couldn't load the next song";
          return;
        }
        await Future.delayed(Duration(milliseconds: 500 * attempts));
        if (token != _loadToken) return;
      }
    }

    _index = index;
    _publish(playlist, index, song, replay: replayId != null);

    for (var attempt = 1; ; attempt++) {
      try {
        await player.setUrl("$apiUrl${song.audio}");
        if (token != _loadToken) return;

        player.play();
        _prefetch(playlist, index + 1);
        return;
      } catch (e) {
        if (token != _loadToken) return;
        if (attempt >= _maxAttempts) {
          loadError.value = "Couldn't play \"${song.title}\"";
          return;
        }
        await Future.delayed(Duration(milliseconds: 500 * attempt));
        if (token != _loadToken) return;
      }
    }
  }

  void _publish(String playlist, int index, Song song, {required bool replay}) {
    _onSongStarted(song, replay: replay);
    nowPlaying.value = NowPlaying(playlist, index, song);
    mediaItem.add(MediaItem(
      id: song.audio,
      title: song.title,
      artist: song.artist,
      artUri: Uri.parse("$apiUrl${song.image}"),
    ));
  }

  @override
  Future<void> play() => player.play();

  @override
  Future<void> pause() => player.pause();

  @override
  Future<void> seek(Duration position) => player.seek(position);

  @override
  Future<void> skipToNext() => _playIndex(_index + 1);

  @override
  Future<void> skipToPrevious() async {
    if (player.position > const Duration(seconds: 3) || alreadyPlayed.length < 2) {
      return player.seek(Duration.zero);
    }
    return _playIndex(_index - 1, replayId: alreadyPlayed[alreadyPlayed.length - 2]);
  }

  @override
  Future<void> stop() async {
    await player.stop();
    return super.stop();
  }

  void _broadcast() {
    playbackState.add(PlaybackState(
      controls: [
        MediaControl.skipToPrevious,
        if (player.playing) MediaControl.pause else MediaControl.play,
        MediaControl.skipToNext,
      ],
      systemActions: const {
        MediaAction.seek,
        MediaAction.skipToNext,
        MediaAction.skipToPrevious,
      },
      androidCompactActionIndices: const [0, 1, 2],
      processingState: const {
        ProcessingState.idle: AudioProcessingState.idle,
        ProcessingState.loading: AudioProcessingState.loading,
        ProcessingState.buffering: AudioProcessingState.buffering,
        ProcessingState.ready: AudioProcessingState.ready,
        ProcessingState.completed: AudioProcessingState.completed,
      }[player.processingState]!,
      playing: player.playing,
      updatePosition: player.position,
      bufferedPosition: player.bufferedPosition,
      speed: player.speed,
    ));
  }
}

class MediaController extends StatefulWidget {
  final AudioPlayer player;
  final VoidCallback onFR;
  final VoidCallback onFF;
  final bool isSeeking;
  final bool isMini;
  
  const new({super.key, required this.player, required this.onFR, required this.onFF, required this.isSeeking, required this.isMini});

  @override
  State<MediaController> createState() => _MediaControllerState();
}

class _MediaControllerState extends State<MediaController> {
  double? size;

  @override
  void initState() {
    super.initState();
    size = 28;
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        IconButton(
          icon: Icon(Icons.fast_rewind_rounded, size: size),
          onPressed: () {
            widget.onFR();
          },
        ),
        StreamBuilder(
          stream: widget.player.playingStream,
          initialData: widget.player.playing,
          builder: (context, snapshot) {
            final playing = snapshot.data ?? false;
            return IconButton(
              icon: Icon((playing || widget.isSeeking) ? Icons.stop_rounded : Icons.play_arrow_rounded, size: size),
              onPressed: () {
                if (playing) {
                  widget.player.pause();
                } else {
                  widget.player.play();
                }
              },
            );
          },
        ),
        IconButton(
          icon: Icon(Icons.fast_forward_rounded, size: size),
          onPressed: () {
            widget.onFF();
          },
        ),
      ],
    );
  }
}

class WaveformProgress extends CustomPainter {
  WaveformProgress(this.progress, {this.color = Colors.black});
  final amps = <double>[
    0.0, 0.0, 0.13, 0.4, 0.2, 0.5, 0.7, 0.2, 0.63, 0.37, 0.4, 0.77, 0.27, 0.2,
    0.7, 0.17, 0.73, 0.37, 0.43, 0.63, 0.73, 0.6, 0.63, 0.57, 0.2, 0.2, 0.9,
    0.83, 0.23, 0.47, 0.2, 0.4, 0.77, 0.6, 0.5, 0.97, 0.33, 0.27, 0.4, 0.67,
    0.27, 0.63, 0.9, 0.33, 0.97, 0.27, 0.53, 0.2, 0.33, 0.2, 0.57, 0.63, 0.7,
    0.93, 0.5, 0.33, 0.5, 0.53, 0.27, 0.73, 0.27, 0.87, 0.13, 0.73, 0.87, 0.37,
    0.27, 0.4, 0.53, 0.33, 0.63, 0.13, 0.37, 0.43, 0.9, 0.37, 0.87, 0.43, 0.67,
    0.2, 0.67, 0.17, 0.4, 0.37, 0.73, 0.17, 0.37, 0.73, 0.67, 0.47, 0.57, 0.63,
    0.63, 0.83, 0.73, 0.2, 0.33, 0.13, 0.13, 0.0,
  ]; // Randomly generated list of amplitudes
  final double progress;
  final Color color;

  @override
  void paint(Canvas canvas, Size size) {
    final pitch = size.width / amps.length;
    final w = pitch * 0.41;
    final x = progress * size.width;
    final minHeight = 3.0;
    final rightH = 0.8;
    final fadeW = 40;
    final fadeOffset = 5;

    canvas.save();
    canvas.clipRect(Rect.fromLTRB(x, 0, size.width, size.height));
    canvas.drawRRect(
      RRect.fromRectAndRadius(
        Rect.fromLTRB(0, (size.height / 2) - rightH, size.width, (size.height / 2) + rightH),
        Radius.circular(w),
      ),
      Paint()..shader = LinearGradient(
        colors: [Colors.transparent, Colors.grey, Colors.grey, Colors.transparent],
        stops: [0, 0.1, 0.9, 1],
      ).createShader(Rect.fromLTRB(x, 0, size.width, size.height)),
    );
    canvas.restore();

    canvas.save();
    canvas.clipRect(Rect.fromLTRB(0, 0, x + fadeOffset, size.height));
    for (var i = 0; i < amps.length; i++) {
      final barX = i * pitch + w / 2;
      final alpha = (((x + fadeOffset) - barX) / fadeW).clamp(0.0, 1.0);

      final rect = Rect.fromCenter(
        center: Offset(i * pitch + w / 2, size.height / 2),
        width: w,
        height: (amps[i] * size.height).clamp(minHeight, size.height),
      );
      final rrect = RRect.fromRectAndRadius(
        rect,
        Radius.circular(w / 2),
      );

      canvas.drawRRect(rrect, Paint()..color = color.withAlpha((alpha * 255).round()));
    }
    canvas.restore();
  }

  @override
  bool shouldRepaint(WaveformProgress old) => old.progress != progress || old.color != color;
}

class SongListItem extends StatelessWidget {
  final String songId;

  const new({super.key, required this.songId});

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(future: fetchSong(songId), builder: (context, snapshot) {
      if (snapshot.connectionState == ConnectionState.waiting) {
        return songListItemSkeleton(context);
      }

      if (snapshot.hasError) {
        return songListItemSkeleton(context);
      }

      final song = snapshot.data;
      if (song == null) { // Song doesn't exist so skip it
        return nullWidget;
      }

      return Material(type: MaterialType.transparency, child:
        Container(
          height: 120,
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(14),
            color: Theme.of(context).colorScheme.surfaceContainerLow
          ),
          child:
            Row(children: [
              SizedBox(width: 10),
              Container(
                width: 100,
                height: 100, 
                decoration: BoxDecoration(
                  borderRadius: BorderRadius.circular(16),
                  boxShadow: const [
                    BoxShadow(color: Colors.black26, blurRadius: 12, offset: Offset(0, 6)),
                  ],
                  image: DecorationImage(
                    image: NetworkImage("$apiUrl${song.thumbnail}"),
                    fit: BoxFit.cover,
                  ),
                ),
              ),
              SizedBox(width: 10),
              Expanded(
                child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                  Text(song.title, style: TextStyle(fontSize: 16), overflow: TextOverflow.ellipsis),
                  Text(song.artist, style: TextStyle(fontSize: 16, color: Colors.grey), overflow: TextOverflow.ellipsis)
                ])
              ),
              SizedBox(width: 8)
            ]),
        )
      ); 
    });
  }
}

class PlayerSideMenu extends StatefulWidget {
  final String nowPlaying;
  final String playlistName;
  final int playlistIndex;

  const new({super.key, required this.nowPlaying, required this.playlistName, required this.playlistIndex});

  @override
  State<PlayerSideMenu> createState() => _PlayerSideMenuState();
}

class _PlayerSideMenuState extends State<PlayerSideMenu> {
  bool isExpanded = false;

  late Future<int> _sizeFuture;
  final Map<int, Future<String>> _itemFutures = {};

  @override
  void initState() {
    super.initState();
    _sizeFuture = fetchPlaylistSize(widget.playlistName);
  }

  @override
  void didUpdateWidget(PlayerSideMenu old) {
    super.didUpdateWidget(old);

    if (old.playlistName != widget.playlistName) {
      _sizeFuture = fetchPlaylistSize(widget.playlistName);
    }
    if (old.playlistName != widget.playlistName ||
        old.playlistIndex != widget.playlistIndex ||
        old.nowPlaying != widget.nowPlaying) {
      _itemFutures.clear();
    }
  }

  void _retrySizeFuture() {
    setState(() {
      _sizeFuture = fetchPlaylistSize(widget.playlistName);
    });
  }

  Future<String> _itemFuture(int index) => _itemFutures.putIfAbsent(
    index,
    () => getSongToBeDisplayed(widget.playlistName, widget.playlistIndex, index),
  );

  Widget _section(String title, Widget child, {bool topGap = true}) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        if (topGap) SizedBox(height: 10),
        Text(title, style: TextStyle(fontSize: 16)),
        SizedBox(height: 5),
        child,
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    final expandedMenu = Align(
      alignment: AlignmentGeometry.topLeft,
      child: Container(
        padding: EdgeInsets.fromLTRB(8, 0, 0, 0),
        constraints: const BoxConstraints(maxWidth: 450),
        width: double.infinity,
        height: double.infinity,
        decoration: BoxDecoration(
          borderRadius: BorderRadius.circular(12),
          boxShadow: const [
            BoxShadow(color: Colors.black26, blurRadius: 12, offset: Offset(0, 6)),
          ],
          color: Theme.of(context).colorScheme.surfaceContainer,
        ),
        child: Align(
          alignment: AlignmentGeometry.topLeft,
          child: Padding(
            padding: EdgeInsets.all(8),
            child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
              IconButton(
                icon: Icon(Icons.arrow_back_rounded),
                onPressed:() {
                  setState(() {
                    isExpanded = false;
                  });
                },
              ),
              SizedBox(height: 10),
              FutureBuilder<int>(
                future: _sizeFuture,
                builder: (context, snapshot) {
                  if (snapshot.connectionState == ConnectionState.waiting) {
                    return waitWidget;
                  }

                  if (snapshot.hasError) {
                    return errorWidget(_retrySizeFuture);
                  }

                  final playlistSize = snapshot.data;
                  if (playlistSize == null) return waitWidget;

                  final queued = determinedQueue.length;
                  final consumed = playlistPositionAfter(widget.playlistIndex, 0) + 1;
                  final remaining = doLoop ? playlistSize : max(0, playlistSize - consumed);

                  return Expanded(
                    child: ListView.builder(
                      itemCount: 1 + queued + remaining,
                      itemBuilder:(context, index) {
                        if (index == 0) {
                          return _section("Now playing", SongListItem(songId: widget.nowPlaying), topGap: false);
                        }

                        return FutureBuilder<String>(
                          future: _itemFuture(index),
                          builder:(context, snapshot) {
                            final songId = snapshot.data;
                            if (songId == null) {
                              return songListItemSkeleton(context);
                            }

                            if (queued > 0 && index == 1) {
                              return _section("Next from queue", SongListItem(songId: songId));
                            }
                            if (index == queued + 1) {
                              return _section("Next from: ${widget.playlistName}", SongListItem(songId: songId));
                            }
                            return SongListItem(songId: songId);
                          },
                        );
                      }
                    )
                  );
                },
              )
            ])
          )
        )
      ),
    );

    final contractedMenu = Align(
      alignment: AlignmentGeometry.topLeft,
      child: Padding(
        padding: EdgeInsets.all(8),
        child:
          IconButton(
            icon: Icon(Icons.queue_music_rounded),
            onPressed:() {
              setState(() {
                isExpanded = true;
              });
            },
          ),
      )
    );

    if (isExpanded) {
      return expandedMenu;
    }
    return contractedMenu;
  }
}

class Player extends StatefulWidget {
  final Song song;
  final VoidCallback onFR;
  final VoidCallback onFF;
  final VoidCallback close;
  final VoidCallback toFull;
  final bool isMini;

  const new({super.key, required this.song, required this.onFR, required this.onFF, required this.close, required this.toFull, required this.isMini});

  @override
  State<Player> createState() => _PlayerState();
}

class _PlayerState extends State<Player> {
  AudioPlayer get player => audioHandler.player;
  bool _isSeeking = false;
  bool _dragging = false;
  bool _wasPlaying = false;
  double _seekingPos = 0.0;

  @override
  void didUpdateWidget(Player old) {
    super.didUpdateWidget(old);
    if (old.song.id != widget.song.id) {
      _isSeeking = false;
      _dragging = false;
    }
  }

  void _visualSeek(double dx, double size) {
    if (!_isSeeking) {
      _wasPlaying = player.playing;
      player.pause();
    }
    setState(() {
      _isSeeking = true;
      _seekingPos = (dx / size).clamp(0.0, 1.0);
    });
  }

  Future<void> _commitSeek() async {
    if (!_isSeeking) return;
    await player.seek((player.duration ?? Duration.zero) * _seekingPos);
    if (!mounted) return;
    setState(() {
      _isSeeking = false;
      _dragging = false;
    });
    if (_wasPlaying) player.play();
  }

  void _cancelSeek() {
    Future.microtask(() {
      if (!mounted || _dragging || !_isSeeking) return;
      setState(() {
        _isSeeking = false;
      });
      if (_wasPlaying) player.play();
    });
  }

  @override
  Widget build(BuildContext context) {
    double imageSize;
    double waveformSize;
    
    final size = MediaQuery.sizeOf(context);
    final smallerSize = min(size.width, size.height);
    if (widget.isMini) {
      imageSize = 80;
      waveformSize = smallerSize * 0.5;
    } else {
      imageSize = smallerSize * 0.5;
      waveformSize = smallerSize * 0.5;
    }

    double minImageSize = imageSize / 2;
  
    final content = Center(
      child: Builder(builder: (context) {
        final song = widget.song;

        Container image(size) {
          return Container(
            width: size,
            height: size,
            decoration: BoxDecoration(
              borderRadius: BorderRadius.circular(size / 10),
              boxShadow: const [
                BoxShadow(color: Colors.black26, blurRadius: 12, offset: Offset(0, 6)),
              ],
              image: DecorationImage(
                image: NetworkImage("$apiUrl${song.image}"),
                fit: BoxFit.cover,
              ),
            ),
          );
        }

        StreamBuilder progressAndControls(justifyLeft) {
          return StreamBuilder<Duration?>(
            stream: player.durationStream,
            builder: (context, durSnap) {
              final duration = durSnap.data ?? Duration.zero;
              return StreamBuilder<Duration>(
                stream: player.positionStream,
                builder: (context, posSnap) {
                  Duration position;
                  if (!_isSeeking) {
                    position = posSnap.data ?? Duration.zero;
                  } else {
                    position = duration * _seekingPos;
                  }
                  final progress = duration.inMilliseconds == 0
                      ? 0.0
                      : (position.inMilliseconds / duration.inMilliseconds).clamp(0.0, 1.0);

                  const timeWidth = 60.0;

                  final remaining = duration > position ? duration - position : Duration.zero;
                  final minutes = remaining.inMinutes;
                  final seconds = (remaining.inSeconds % 60).toString().padLeft(2, '0');

                  return Column(mainAxisSize: MainAxisSize.min, children: [
                    Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        if (justifyLeft) const SizedBox(width: timeWidth), // mirrors the text width so the waveform stays centered
                        GestureDetector(
                          behavior: HitTestBehavior.opaque,
                          onTapDown: (details) {
                            _visualSeek(details.localPosition.dx, waveformSize);
                          },
                          onTapUp: (_) => _commitSeek(),
                          onTapCancel: _cancelSeek,
                          onHorizontalDragStart: (details) {
                            _dragging = true;
                            _visualSeek(details.localPosition.dx, waveformSize);
                          },
                          onHorizontalDragUpdate: (details) {
                            _visualSeek(details.localPosition.dx, waveformSize);
                          },
                          onHorizontalDragEnd: (_) => _commitSeek(),
                          onHorizontalDragCancel: () {
                            _dragging = false;
                            _cancelSeek();
                          },
                          child: CustomPaint(
                            size: Size(waveformSize, 30),
                            painter: WaveformProgress(progress, color: Theme.of(context).colorScheme.onSurface),
                          )
                        ),
                        SizedBox(
                          width: timeWidth,
                          child: Padding(
                            padding: const EdgeInsets.only(left: 8),
                            child: Text("$minutes:$seconds"),
                          ),
                        ),
                      ],
                    ),
                    MediaController(player: player, onFR: widget.onFR, onFF: widget.onFF, isSeeking: _isSeeking, isMini: widget.isMini),
                  ]);
                },
              );
            },
          );
        }

        if (widget.isMini) {
          return LayoutBuilder(builder:(context, constraints) {
            final double textSize = 14;
            const paddings = 10.0 + 8.0 + 8.0 + 50.0;
            const textGap = 65;

            final style = DefaultTextStyle.of(context).style.merge(TextStyle(fontSize: textSize));
            final textCalculated = max(
              textWidth(context, song.title, style),
              textWidth(context, song.artist, style),
            );
            final textAvailable = ((constraints.maxWidth - waveformSize) / 2) - imageSize - paddings;

            bool isNarrow = false;
            bool isVeryNarrow = false;
            
            if (textAvailable < textCalculated) {
              isNarrow = true;

              final narrowImageSize = min(imageSize, (constraints.maxWidth - waveformSize - paddings - textCalculated - textGap));
              imageSize = narrowImageSize;
            }

            if (imageSize < minImageSize) {
              isVeryNarrow = true;
              imageSize = minImageSize;
            }

            final titleArtistText = Expanded(
              child: Column(
                mainAxisAlignment: isVeryNarrow ? MainAxisAlignment.center : MainAxisAlignment.start,
                mainAxisSize: MainAxisSize.min,
                crossAxisAlignment: isVeryNarrow ? CrossAxisAlignment.center : CrossAxisAlignment.start,
                children: [
                  Text(
                    isVeryNarrow ? "  ${song.title}" : song.title, // Add a bit of space on the left to prevent clipping out of screen on phones
                    overflow: TextOverflow.ellipsis,
                    style: TextStyle(fontSize: textSize),
                  ),
                  Text(
                    isVeryNarrow ? "  ${song.artist}" : song.artist, // Add a bit of space on the left to prevent clipping out of screen on phones
                    overflow: TextOverflow.ellipsis,
                    style: TextStyle(fontSize: textSize, color: Colors.grey),
                  ),
                ],
              ),
            );

            return Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                Expanded(
                  child: GestureDetector(
                    onTap: widget.toFull,
                    child: Align(
                      alignment: Alignment.centerLeft,
                      child: isVeryNarrow ?
                      Column(
                        children: [
                          SizedBox(height: 8, width: 10),
                          image(imageSize),
                          titleArtistText
                        ],
                      )
                      :
                      Row(
                        children: [
                          SizedBox(width: 10),
                          image(imageSize),
                          SizedBox(width: 8),
                          titleArtistText
                        ],
                      )
                    ),
                  ),
                ),
                Align(
                  alignment: AlignmentGeometry.bottomCenter,
                  child: progressAndControls(!isNarrow)
                ),
                if(!isNarrow) Expanded(child: SizedBox.shrink())
              ],
            );
          });
        }

        return Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            image(imageSize),
            SizedBox(height: 10),
            progressAndControls(true),
            SizedBox(height: 10),
            Text(
              song.title,
              overflow: TextOverflow.ellipsis,
              style: TextStyle(fontSize: 18),
            ),
            Text(
              song.artist,
              overflow: TextOverflow.ellipsis,
              style: TextStyle(fontSize: 18, color: Colors.grey),
            ),
            SizedBox(height: 10),
          ]
        );
      })
    );

    if (widget.isMini) {
      return Align(
        alignment: AlignmentGeometry.bottomCenter,
        child: Material(
          child: SizedBox(
            height: 100,
            child: content,
          ),
        )
      );
    }

    return Scaffold(body: Stack(
      children: [
        Center( // Create a fake bar that represents the general area of the gesture detector
          child: Align(
            alignment: AlignmentGeometry.topCenter,
            child: Padding(
              padding: EdgeInsetsGeometry.fromLTRB(0, 10, 0, 0),
              child: Container(
                width: 100,
                height: 3,
                decoration: BoxDecoration(
                  borderRadius: BorderRadius.all(Radius.circular(10)),
                  color: Colors.grey,
                ),
              )
            )
          )
        ),
        GestureDetector( // Create a bigger gesture detector than the fake bar
          child: Align(
            alignment: AlignmentGeometry.topCenter,
            child: GestureDetector(
              onTap: widget.close,
              child: Container(
                width: 140,
                height: 23,
                decoration: BoxDecoration(color: Colors.transparent),
              ),
            )
          )
        ),
        content,
      ]
    ));
  }
}
