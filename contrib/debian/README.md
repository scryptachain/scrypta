
Debian
====================
This directory contains files used to package lyrad/lyra-qt
for Debian-based Linux systems. If you compile lyrad/lyra-qt yourself, there are some useful files here.

## lyra: URI support ##


lyra-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install lyra-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your lyraqt binary to `/usr/bin`
and the `../../share/pixmaps/lyra128.png` to `/usr/share/pixmaps`

lyra-qt.protocol (KDE)

