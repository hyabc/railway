# Project Railway

Project Railway is a gtk+ music player.

## Requirement

The following packages are needed to compile and run: `ffmpeg mpg123 gtk3-devel gstreamer1-devel gstreamer1-plugins-base-devel gstreamer1-plugins-bad-free-devel`.

On Debian: `ffmpeg mpg123 libgtk-3-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev`.

Also, you can install [Glade](https://glade.gnome.org/) to edit the UI file.

## Compile

Use `make` to compile. Also, you can use `make install` to install to local directory and `make uninstall` to remove.

## Configuration

The program reads a configuration file in `~/.railway`. The configuration file looks like
``` ini
[railway]
LIBRARY_PATH=(path to music library)
ALBUM_CACHE_PATH=(path to cache folder)
```

The program assumes music library is organized in this way:
```
└── Music library root
    └── Artist name
        └── Album name
            └── Music file
```
