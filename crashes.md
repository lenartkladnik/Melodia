# 1.
## download song - place that song into playlist - try to download another song

[New Thread 0x7bffe43d86c0 (LWP 642460)]
terminate called without an active exception
Info: Checking if yt-dlp already exists on the system

Thread 1 "Melodia" received signal SIGABRT, Aborted.
Downloading 4.48 K source file /usr/src/debug/glibc/glibc/nptl/pthread_kill.c
__pthread_kill_implementation (threadid=<optimized out>, signo=signo@entry=6, no_tid=no_tid@entry=0) at pthread_kill.c:44
44            return INTERNAL_SYSCALL_ERROR_P (ret) ? INTERNAL_SYSCALL_ERRNO (ret) : 0;
(gdb) bt
#0  __pthread_kill_implementation (threadid=<optimized out>, signo=signo@entry=6, no_tid=no_tid@entry=0) at pthread_kill.c:44
#1  0x00007ffff5c9a243 in __pthread_kill_internal (threadid=<optimized out>, signo=6) at pthread_kill.c:89
#2  0x00007ffff5c3e5d0 in __GI_raise (sig=sig@entry=6) at ../sysdeps/posix/raise.c:26
#3  0x00007ffff5c25685 in __GI_abort () at abort.c:77
#4  0x00007ffff689ac54 in __gnu_cxx::__verbose_terminate_handler () at ../../../../gcc/libstdc++-v3/libsupc++/vterminate.cc:95
#5  0x00007ffff68b581a in __cxxabiv1::__terminate (handler=<optimized out>) at ../../../../gcc/libstdc++-v3/libsupc++/eh_terminate.cc:48
#6  0x00007ffff689a5ed in std::terminate () at ../../../../gcc/libstdc++-v3/libsupc++/eh_terminate.cc:58
#7  0x000055555624529e in std::__terminate () at /usr/include/c++/16/x86_64-pc-linux-gnu/bits/c++config.h:358
#8  std::thread::~thread (this=0x7c1ff24ba7d0) at /usr/include/c++/16/bits/std_thread.h:184
#9  0x0000555556252f3b in std::default_delete<std::thread>::operator() (this=0x555557a8fd00 <download_song_thread>, __ptr=0x7c1ff24ba7d0) at /usr/include/c++/16/bits/unique_ptr.h:92
#10 0x00005555564ffe02 in std::__uniq_ptr_impl<std::thread, std::default_delete<std::thread> >::reset (this=0x555557a8fd00 <download_song_thread>, __p=0x7c1ff252e2b0) at /usr/include/c++/16/bits/unique_ptr.h:204
#11 0x00005555564c06ae in std::__uniq_ptr_impl<std::thread, std::default_delete<std::thread> >::operator= (this=0x555557a8fd00 <download_song_thread>, __u=...) at /usr/include/c++/16/bits/unique_ptr.h:184
#12 0x0000555556461c29 in std::__uniq_ptr_data<std::thread, std::default_delete<std::thread>, true, true>::operator= (this=0x555557a8fd00 <download_song_thread>) at /usr/include/c++/16/bits/unique_ptr.h:236
#13 0x0000555556461d15 in std::unique_ptr<std::thread, std::default_delete<std::thread> >::operator= (this=0x555557a8fd00 <download_song_thread>) at /usr/include/c++/16/bits/unique_ptr.h:418
#14 0x00005555563881be in download_from_search (component=0x555557a926a0 <init_playlist_selector(MenuData&)::search>) at src/download.cpp:341
#15 0x0000555556359a6f in std::__invoke_impl<bool, bool (*&)(InputComponent*), InputComponent*> (__f=@0x555557a92d30: 0x555556387f46 <download_from_search(InputComponent*)>) at /usr/include/c++/16/bits/invoke.h:63
#16 0x0000555556350ef2 in std::__invoke_r<void, bool (*&)(InputComponent*), InputComponent*> (__fn=@0x555557a92d30: 0x555556387f46 <download_from_search(InputComponent*)>) at /usr/include/c++/16/bits/invoke.h:113
#17 0x0000555556342de6 in std::_Function_handler<void(InputComponent*), bool (*)(InputComponent*)>::_M_invoke (__functor=..., __args#0=@0x7bfff0fc1de0: 0x555557a926a0 <init_playlist_selector(MenuData&)::search>)
    at /usr/include/c++/16/bits/std_function.h:295
#18 0x00005555562b3d9c in std::function<void(InputComponent*)>::operator() (this=0x555557a92d30 <init_playlist_selector(MenuData&)::search+1680>, __args#0=0x555557a926a0 <init_playlist_selector(MenuData&)::search>)
    at /usr/include/c++/16/bits/std_function.h:581
#19 0x000055555629894e in InputComponent::connect_signals()::{lambda()#6}::operator()() const (__closure=0x7cfff243c6e0) at src/include/components.hpp:279
#20 0x00005555562e1068 in std::__invoke_impl<void, InputComponent::connect_signals()::{lambda()#6}&>(std::__invoke_other, InputComponent::connect_signals()::{lambda()#6}&) (__f=...) at /usr/include/c++/16/bits/invoke.h:63
#21 0x00005555562d60b9 in std::__invoke_r<void, InputComponent::connect_signals()::{lambda()#6}&>(InputComponent::connect_signals()::{lambda()#6}&) (__fn=...) at /usr/include/c++/16/bits/invoke.h:113
#22 0x00005555562c6e93 in std::_Function_handler<void (), InputComponent::connect_signals()::{lambda()#6}>::_M_invoke(std::_Any_data const&) (__functor=...) at /usr/include/c++/16/bits/std_function.h:295
#23 0x00005555562238da in std::function<void()>::operator() (this=0x7cfff243c6e0) at /usr/include/c++/16/bits/std_function.h:581
#24 0x0000555556214ddd in Signal::emit (this=0x555557a93420 <confirm_signal>) at src/include/signals.hpp:33
#25 0x00005555561fd404 in main () at src/main.cpp:203
(gdb) exit
A debugging session is active.

        Inferior 1 [process 642181] will be killed.

Quit anyway? (y or n) y

# 2.
## try to download song that fails to get cover art

[download.cpp] Results page: 'https://www.last.fm/search/albums?q=We+Will+Rock+You+by+Queen'
[download.cpp] Cover art: 'https://lastfm-img.freetls.fastly.net/i/u/1000s/ffb04fbc8484423f812b21b1723ee286.jpg'
[download.cpp] Failed to get cover art data (url='https://lastfm-img.freetls.fastly.net/i/u/1000s/ffb04fbc8484423f812b21b1723ee286.jpg')
    Status code: 404
terminate called after throwing an instance of 'std::runtime_error'
  what():  Failed to decode image from .music_data/data/22.cover_art.png.tmp: can't fopen

