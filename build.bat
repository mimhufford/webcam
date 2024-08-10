cl main.cpp escapi.cpp ^
   /nologo ^
   /I include ^
   /link ^
   /LIBPATH:lib SDL2.lib SDL2main.lib shell32.lib user32.lib ^
   /SUBSYSTEM:WINDOWS

main