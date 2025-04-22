# SyncThingy
SyncThingy = Syncthing + simple tray indicator

![Screenshot1](screenshots/Screenshot_v3.png)

[<img width="240px" src="https://flathub.org/assets/badges/flathub-badge-en.png" />](https://flathub.org/apps/details/com.github.zocker_160.SyncThingy)

## Motivation

Why yet another Syncthing tray / statusbar / wrapper / ui ... application?

The answer is simple: I just wanted a Flatpak on Flathub which offers just the bare minimum: Syncthing and a tray icon.

## Configuration

SyncThingy config is located at `~/.var/app/com.github.zocker_160.SyncThingy/config/SyncThingy`:

- `autostart` can be `true` or `false`
- `icon` can be set to `default`, `white` or `black`
- `url` defines the link opened when selecting `Open WebUI`
- `notifications` can be `true` or `false`

#### Note

Syncthing config is located at `~/.var/app/com.github.zocker_160.SyncThingy/config/syncthing`.

NOTE: this location seems to differ on some installations, so if the folder above does not work for you,
try `~/.var/app/com.github.zocker_160.SyncThingy/.local/state/syncthing` instead.

You can reuse your already existing syncthing `config.xml` file, by moving / copying it there.

## Install background service

**Note**: this feature is mainly targeted at Steam Deck users, that want to have it running in the background when in Gaming Mode.
It does not really make much sense to use this on a normal Linux Desktop.

- (Steam Deck) switch to Desktop Mode
- right-click the SyncThiny icon in the system tray
- select `Settings`
- press `install as system service` button
- press `copy to clipboard`
- open terminal (`konsole` on Steam Deck)
- paste the command and hit enter (press `paste` in the top right on Steam Deck)
- (Steam Deck) open virtual keyboard with `STEAM` + `X` and press `R2` or `Enter` button
- reboot

## Compile from source

**Build dependencies:**

- `build-essential`
- `qt5-default` || `qtbase5-dev` `qtbase5-dev-tools`
- `libportal-qt5-dev`

```bash
git clone --recurse-submodules https://github.com/zocker-160/SyncThingy.git
cd SyncThingy
mkdir build && cd build
cmake ..
cmake --build .
sudo make install (optional)
```

### Note for non-Flatpak users

You will need to have `syncthing` installed and available in `$PATH`, in order to make this work.

**Running outside of Flatpak is not supported, you are on your own.**

### Links

- [Syncthing Github page](https://github.com/syncthing/syncthing)
- [SyncThingy on Flathub](https://flathub.org/apps/details/com.github.zocker_160.SyncThingy)
