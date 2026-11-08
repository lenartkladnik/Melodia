import 'dart:convert';
import 'dart:io' show Platform;
import 'dart:math';
import 'package:audio_service/audio_service.dart';
import 'package:audio_session/audio_session.dart';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:just_audio/just_audio.dart';
import 'package:just_audio_media_kit/just_audio_media_kit.dart';

final apiUrl = "http://lenart:5400";
var shuffleSeed = Random().nextInt(1024);
var doShuffle = true;
var doLoop = false;

final waitWidget = const Center(child: CircularProgressIndicator());
final nullWidget = const SizedBox.shrink();
Widget errorWidget(VoidCallback callback) {return Center(child: IconButton(onPressed: callback, tooltip: "Retry", iconSize: 32, icon: const Icon(Icons.refresh_rounded)));}

double textWidth(BuildContext context, String text, TextStyle style) {
  final painter = TextPainter(
    text: TextSpan(text: text, style: style),
    maxLines: 1,
    textDirection: TextDirection.ltr,
    textScaler: MediaQuery.textScalerOf(context),
  )..layout();
  return painter.width;
}

late final AudioHandler audioHandler;

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  JustAudioMediaKit.ensureInitialized();

  if (Platform.isLinux || Platform.isWindows) {
    audioHandler = AudioHandler();
  } else {
    audioHandler = await AudioService.init(
      builder: () => AudioHandler(),
      config: const AudioServiceConfig(
        androidNotificationChannelId: "com.lenartkladnik.melodia.playback",
        androidNotificationChannelName: "Melodia playback",
        androidNotificationOngoing: true,
      ),
    );
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

Future<String> fetchNextSongId(String playlist, int index) async {
  final uri = Uri.parse("$apiUrl/playlist/next").replace(
    queryParameters: {
      "playlist_name": playlist,
      "index": index.toString(),
      "shuffle": doShuffle.toString(),
      "seed": shuffleSeed.toString(),
      "loop": doLoop.toString()
    }
  );

  final response = await http.get(uri);
  return jsonDecode(response.body)["song_id"].toString();
}

Future<List<String>> fetchAllPlaylists() async {
  final uri = Uri.parse("$apiUrl/playlists");

  final response = await http.get(uri);
  return (jsonDecode(response.body) as List<dynamic>).cast<String>();
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
  final uri = Uri.parse("$apiUrl/playlist").replace(
    queryParameters: {"playlist_name": playlist}
  );

  final response = await http.get(uri);
  return Playlist.fromJson(jsonDecode(response.body));
}

class PlaylistSelectorWidget extends StatefulWidget {
  final Function(String) switchToPlayer;

  const new({super.key, required this.switchToPlayer});

  @override
  State<PlaylistSelectorWidget> createState() => _PlaylistSelectorWidgetState();
}

class _PlaylistSelectorWidgetState extends State<PlaylistSelectorWidget> {
  Future<List<String>> allPlaylistsFuture = fetchAllPlaylists(); 

  void _retryAllPlaylistsFuture() {
    setState(() {
      allPlaylistsFuture = fetchAllPlaylists();
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

        List<FutureBuilder> playlistItems = [];
        for (var playlistName in playlists) {
          Future<Playlist> playlistFuture = fetchPlaylist(playlistName);
          void retryPlaylistFuture() {
            setState(() {
              playlistFuture = fetchPlaylist(playlistName);
            });
          }
          playlistItems.add(
            FutureBuilder(
              future: playlistFuture,
              builder:(context, snapshot) {
                if (snapshot.connectionState == ConnectionState.waiting) {
                  return waitWidget;
                }

                if (snapshot.hasError) {
                  errorWidget(retryPlaylistFuture);
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
                    Column(children: playlistItems)
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
  }

  void _setMini(bool mini) => setState(() {miniPlayer = mini;});

  void _setPlaylist(String newPlaylist) {
    setState(() {requested = true;});
    audioHandler.loadPlaylist(newPlaylist);
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
          if (!miniPlayer) PlayerSideMenu(playlistName: now.playlist, playlistIndex: now.index),
        ]);
      },
    );
  }

  @override
  void dispose() {
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

  new({
    required this.artist,
    required this.audio,
    required this.base,
    required this.image,
    required this.thumbnail,
    required this.title,
  });
  
  factory Song.fromJson(Map<String, dynamic> json) {
    return Song(
      artist: json['artist'] as String,
      audio: json['audio'] as String,
      base: json['base'] as String,
      image: json['image'] as String,
      thumbnail: json['thumbnail'] as String,
      title: json['title'] as String,
    );
  }
}

Future<Song?> fetchSong(String id) async {
  final uri = Uri.parse("$apiUrl/song").replace(
    queryParameters: {"song_id": id},
  );
  final response = await http.get(uri);

  if (jsonDecode(response.body).isEmpty) {
    return null;
  }

  return Song.fromJson(jsonDecode(response.body));
}

class NowPlaying {
  final String playlist;
  final int index;
  final Song song;

  const NowPlaying(this.playlist, this.index, this.song);
}

class AudioHandler extends BaseAudioHandler with SeekHandler {
  final AudioPlayer player = AudioPlayer();
  final ValueNotifier<NowPlaying?> nowPlaying = ValueNotifier<NowPlaying?>(null);

  String? _playlist;
  int _index = 0;
  int _loadToken = 0;

  Future<Song?>? _next;
  String? _nextPlaylist;
  int? _nextIndex;

  AudioHandler() {
    _init();
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

    player.processingStateStream.listen((state) {
      if (state == ProcessingState.completed) {
        skipToNext();
      }
    });

    try {
      final session = await AudioSession.instance;
      await session.configure(const AudioSessionConfiguration.music());
    } finally {}
  }

  Future<void> loadPlaylist(String name) async {
    if (name == _playlist && nowPlaying.value != null) return;

    _playlist = name;
    _index = 0;
    _next = null;
    await _playIndex(0);
  }

  Future<Song?> _resolve(String playlist, int index) async {
    final id = await fetchNextSongId(playlist, index).timeout(const Duration(seconds: 15));
    return fetchSong(id).timeout(const Duration(seconds: 15));
  }

  Future<Song?> _songFor(String playlist, int index) async {
    final cached = _next;
    if (cached != null && _nextPlaylist == playlist && _nextIndex == index) {
      try {
        return await cached;
      } finally {}
    }
    return _resolve(playlist, index);
  }

  void _prefetch(String playlist, int index) {
    _nextPlaylist = playlist;
    _nextIndex = index;
    _next = _resolve(playlist, index)..ignore();
  }

  Future<void> _playIndex(int index) async {
    final playlist = _playlist;
    if (playlist == null || index < 0) return;

    final token = ++_loadToken;

    while (true) {
      try {
        Song? song;
        while (song == null) {
          song = await _songFor(playlist, index);
          if (token != _loadToken) return;
          if (song == null) index++;
        }

        _index = index;
        _publish(playlist, index, song);

        await player.setUrl("$apiUrl${song.audio}");
        if (token != _loadToken) return;

        player.play();
        _prefetch(playlist, index + 1);
        return;
      } catch (e) {
        if (token != _loadToken) return;
      }
    }
  }

  void _publish(String playlist, int index, Song song) {
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
  Future<void> skipToPrevious() => _playIndex(_index - 1);

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
          icon: Icon(Icons.fast_rewind_rounded, size: size, color: Colors.black),
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
              icon: Icon((playing || widget.isSeeking) ? Icons.stop_rounded : Icons.play_arrow_rounded, size: size, color: Colors.black),
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
          icon: Icon(Icons.fast_forward_rounded, size: size, color: Colors.black),
          onPressed: () {
            widget.onFF();
          },
        ),
      ],
    );
  }
}

class WaveformProgress extends CustomPainter {
  WaveformProgress(this.progress);
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

      canvas.drawRRect(rrect, Paint()..color = Color.fromARGB((alpha * 255).round(), 0, 0, 0));
    }
    canvas.restore();
  }

  @override
  bool shouldRepaint(WaveformProgress old) => old.progress != progress;
}

class SongListItem extends StatelessWidget {
  final String songId;

  const new({super.key, required this.songId});

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(future: fetchSong(songId), builder: (context, snapshot) {
      if (snapshot.connectionState == ConnectionState.waiting) {
        return waitWidget;
      }

      if (snapshot.hasError) {
        return waitWidget;
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
              SizedBox(
                width: 300,
                child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                  Text(song.title, style: TextStyle(fontSize: 16), overflow: TextOverflow.ellipsis),
                  Text(song.artist, style: TextStyle(fontSize: 16, color: Colors.grey), overflow: TextOverflow.ellipsis)
                ])
              ),
              SizedBox(width: 2)
            ]),
        )
      ); 
    });
  }
}

class PlayerSideMenu extends StatefulWidget {
  final String playlistName;
  final int playlistIndex;

  const new({super.key, required this.playlistName, required this.playlistIndex});

  @override
  State<PlayerSideMenu> createState() => _PlayerSideMenuState();
}

class _PlayerSideMenuState extends State<PlayerSideMenu> {
  bool isExpanded = false;
  late Future<Playlist> playlistDataFuture;

  @override
  void initState() {
    // TODO: get all song ids and corresponding data once
    playlistDataFuture = fetchPlaylist(widget.playlistName);
    super.initState();
  }

  void _retryPlaylistDataFuture() {
    setState(() {
      playlistDataFuture = fetchPlaylist(widget.playlistName);
    });
  }

  @override
  Widget build(BuildContext context) {
    final expandedMenu = Container(
      padding: EdgeInsets.fromLTRB(8, 0, 0, 0),
      width: 450,
      height: MediaQuery.sizeOf(context).height,
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
            FutureBuilder(
              future: playlistDataFuture,
              builder:(context, snapshot) {
                List<Widget> menuItems = [];
  
                if (snapshot.connectionState == ConnectionState.waiting) {
                  return waitWidget;
                }

                if (snapshot.hasError) {
                  return errorWidget(_retryPlaylistDataFuture);
                }

                final playlistItems = snapshot.data;
                if (playlistItems == null) return waitWidget;

                for (int i = 0; i < 10; i++) {
                  menuItems.add(
                    FutureBuilder(future: fetchNextSongId(widget.playlistName, i + widget.playlistIndex), builder:(context, snapshot) {
                      if (snapshot.connectionState == ConnectionState.waiting) {
                        return waitWidget;
                      }

                      if (snapshot.hasError) {
                        return waitWidget;
                      }

                      final songId = snapshot.data;
                      if (songId == null) {
                        return waitWidget;
                      }
                      
                      return SongListItem(songId: songId);
                    })
                  );
                  menuItems.add(SizedBox(height: 10));
                }
                
                return Column(children: menuItems);
              }
            )
          ])
        )
      )
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
  double _seekingPos = 0.0;

  void _visualSeek(double dx, double size) {
    player.pause();
    setState(() {
      _isSeeking = true;
      _seekingPos = (dx / size).clamp(0.0, 1.0);
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

                  final remaining = duration - position;
                  final minutes = remaining.inMinutes;
                  final seconds = (remaining.inSeconds % 60).toString().padLeft(2, '0');

                  return Column(mainAxisSize: MainAxisSize.min, children: [
                    Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        if (justifyLeft) const SizedBox(width: timeWidth), // mirrors the text width so the waveform stays centered
                        GestureDetector(
                          behavior: HitTestBehavior.opaque,
                          onTapDown:(details) {
                            _visualSeek(details.localPosition.dx, waveformSize);
                          },
                          onHorizontalDragStart: (details) {
                            _visualSeek(details.localPosition.dx, waveformSize);
                          },
                          onHorizontalDragUpdate: (details) {
                            _visualSeek(details.localPosition.dx, waveformSize);
                          },
                          onHorizontalDragEnd: (details) {
                            final seekPos = (details.localPosition.dx / waveformSize).clamp(0.0, 1.0);
                            player.seek((player.duration ?? Duration.zero) * seekPos);
                            setState(() {
                              _isSeeking = false;
                            });
                            player.play();
                          },
                          child: CustomPaint(
                            size: Size(waveformSize, 30),
                            painter: WaveformProgress(progress),
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

            final style = DefaultTextStyle.of(context).style;
            style.merge(TextStyle(fontSize: 14));
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
