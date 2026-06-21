# Omacut

A dead-simple video **length** trimmer. Open a video, drag the two yellow handles to pick a start and end, preview the clip, and export.

**Qt Quick (QML)** UI with the Material style — the same Qt stack Quickshell builds on — and **ffmpeg** for the cut. The C++ side compiles to a single executable; the QML is embedded in it via Qt resources.

## Install

Install via the Omarchy Package Repository via the `omacut` package. It's installed by default in new installations of Omarchy (from Quattro forward).

## Run

You can run omacut via the .desktop app through your launcher or via the terminal using:

```bash
./build/omacut
./build/omacut clip.mp4
```

## Requirements

- `xdg-desktop-portal` and a portal backend for the file picker
- `ffmpeg` and `ffprobe` on your PATH (used at runtime)

## Build

Uses Qt's own build tool, `qmake6` (no cmake needed):

```bash
./bin/build
```

This produces a single `omacut` binary in `build/`.

Requirements:

- A C++17 compiler and Qt6: `qt6-base`, `qt6-declarative` (Qt Quick + Controls),
  `qt6-multimedia`

## Test

```bash
./bin/test
```

## Using it

- Click **Open a video** or the preview to load or replace a video.
- The first frame is shown as soon as the video is ready.
- The status line says `Loading...` while the thumbnail strip fills in.
- Drag the **left/right yellow handles** to set the start and end.
- While dragging a trim handle, a small timestamp bubble shows that handle's time.
- Click inside the strip or drag the white playhead to **scrub**.
- **Play / Space** previews just the selected clip (auto-stops at the end).
- The export button opens a portal save dialog and suggests `<name>_trimmed.<ext>` next to the original.
- Exports are re-encoded with `libx264`/`aac` for frame-accurate cuts.

## Package

Build and install the local Arch package:

```bash
./bin/install
```

This runs `./bin/build`, then `makepkg -si` from `pkgbuild/`. Extra arguments are passed through to `makepkg`, for example `./bin/install --clean`. The package installs the binary, desktop entry, app icon, and MIT license. Local package outputs such as `pkgbuild/pkg/`, `pkgbuild/src/`, and `*.pkg.tar.*` are ignored.

## License

MIT. See `LICENSE`.

## Source layout

| File | Role |
|------|------|
| `src/Main.qml`        | The window: video preview, controls, file dialogs. |
| `src/TrimBar.qml`     | The filmstrip + draggable trim handles (custom QML). |
| `src/main.cpp`        | Entry point: sets the Material style, wires QML ↔ C++. |
| `src/backend.*`       | QObject exposed to QML: load, thumbnails, export. |
| `src/filepicker.h`    | Small file picker interface used by the backend and tests. |
| `src/portalfilepicker.*` | XDG desktop portal file picker implementation. |
| `src/thumbprovider.*` | Serves filmstrip thumbnails to QML `Image` elements. |
| `src/thumbworker.*`   | Background thread that builds the filmstrip thumbnails. |
| `src/ffmpeg.*`        | ffprobe/ffmpeg wrappers (probe, thumbnail, trim). |
| `src/resources.qrc`   | Embeds the QML into the binary. |
| `omacut.pro`      | qmake project file. |
| `tests/`          | Qt Test backend coverage. |
| `bin/build`       | Local build helper. |
| `bin/test`        | Local test helper. |
| `bin/install`     | Local Arch package build/install helper. |
| `pkgbuild/PKGBUILD`        | Arch package recipe. |
| `pkgbuild/omacut.svg`      | Application icon (installed to the hicolor theme). |
| `pkgbuild/omacut.desktop`  | Desktop entry: launcher menu + video file association. |
| `pkgbuild/omacut.install`  | pacman hook: refreshes desktop/icon caches on install. |
