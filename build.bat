cl webcam.cpp escapi.cpp ^
   /nologo ^
   /I raylib/include ^
   /O2 ^
   /MD ^
   /link ^
   /SUBSYSTEM:WINDOWS ^
   /ENTRY:"mainCRTStartup" ^
   /LIBPATH:raylib/lib raylib.lib shell32.lib user32.lib gdi32.lib winmm.lib ^