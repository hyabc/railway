# Project Railway

Project Railway is a gtk+ music player.

## Requirement

The following packages are needed to compile and run: `ffmpeg gtk3-devel gstreamer1-devel gstreamer1-plugins-base-devel gstreamer1-plugins-bad-free-devel`.

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
