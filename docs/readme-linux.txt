
- Readme for Linux -

To run this game on Linux, you have two possibilities: You can either
compile your own binary or you can use a pre-compiled version if it's
available for download, too. The pre-compiled version sometimes is statically linked,
so it should run on all Linux/i86 systems. To use it simply run the
"smc" executable in the 1.* folder.

If it does not work or if you are using any other system than Linux/i86, you
have to get the source code and compile your own executable file.

The rest of this file describes how to compile this game for Linux.
The build files are just a quick-n-dirty hack and are not very well tested,
so if you run into problems, please contact me. 

1.) Requirements

You need:
- The Secret Maryo Chronicles source distribution.
- The GCC G++ compiler (version 3.4 or higher)
- Following libraries (with headers/development files):
	Boost Filesystem ( >= 1.31 )
	GNU Gettext
	SDL ( >= 1.2.10 )
	SDL_image ( >= 1.2.0 )
		libpng
	SDL_mixer ( >= 1.2.0 )
		libvorbis
	SDL_ttf ( >= 2.0 )
		FreeType 2
	CEGUI ( >= 0.5 )
		Either SILLY, DevIL or FreeImage
- GNU make
- autoconf (2.57 or higher) and automake (IMPORTANT: version 1.7 or higher)

2.) How to build the sources with automake/autoconf ?

Change to the directory with the "autogen.sh" and "configure.ac" files.
Then run the autogen.sh script by typing "./autogen.sh" (alternatively
you can also run "autoreconf -i" instead).
You'll get a "configure" script - run it by typing "./configure".
(You might want to run "./configure --help" first to have a look at the
available options).
If everything is fine, you'll get the Makefile. Now you can build SMC by
simply running "make".


3.) How to install the game from the source distribution ?

Make sure you have the graphics and levels files from the either the binary
Linux distribution of Secret Maryo Chronicles or from the Windows version of SMC
(i.e. you need the folder "data" folder with all its sub-folders like "font",
"levels", "music", "pixmaps", "sounds", "world" and more ) and copy these data
sub-folders to the "data" directory in the source tree. You should also
download the music files package if you want to have background musics.

Now type "make install" to install the game in the directory that has been
determined while running the "configure" script.


4.) How to start ? 

Afer installation, the "smc" executable has been placed in the "bin" folder
that has been determined while running "configure". Simply run this executable
to start SMC. Run it with the parameter "--help" to see the possible options.

5.) Please Contribute

If you get SMC compiled on a distribution or Operating System which SMC doesn't support
please send your modifications and a link to the prebuild installation packages.

