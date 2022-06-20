# SyncThingy
SyncThingy = Synthing + simple tray incidator

![Screenshot1](screenshots/Screenshot_20220620_202001.png)

## Motivation

Why yet another Syncthing tray / statusbar / wrapper / ui ... application?

The answer is simple: there is currently no application available as Flatpak and on Flathub, which offers just the bare minimum: Syncthing and a tray icon.

Steam Deck users can thank me later.

## Installation

// Flatpak
// DEB

## Compile from source

**Build dependencies:**

- `build-essential`
- `qt5-default`

```bash
git clone https://github.com/zocker-160/SyncThingy.git
mkdir out && cd out
cmake ..
make
```

### NOTE for non-Flatpak users

You will need to have `syncthing` installed and available in $PATH, in order to make this work.

Check out the [Syncthing Github page](https://github.com/syncthing/syncthing) for more information.
