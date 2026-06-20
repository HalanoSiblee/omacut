# omacut

A dead-simple video **length** trimmer — like the iPhone Photos clip editor.
Open a video, drag the two yellow handles to pick a start and end, preview the
clip, and export. **Qt Quick (QML)** UI with the Material style — the same Qt
stack Quickshell builds on — and **ffmpeg** for the cut. The C++ side compiles
to a single executable; the QML is embedded in it via Qt resources.

## Requirements

- A C++17 compiler and Qt6: `qt6-base`, `qt6-declarative` (Qt Quick + Controls),
  `qt6-multimedia`
- `ffmpeg` and `ffprobe` on your PATH (used at runtime)

On Arch / Omarchy:

```bash
sudo pacman -S --needed qt6-base qt6-declarative qt6-multimedia ffmpeg
```

## Build

Uses Qt's own build tool, `qmake6` (no cmake needed):

```bash
mkdir -p build && cd build
qmake6 ../omacut.pro
make -j$(nproc)
```

This produces a single `omacut` binary in `build/`.

## Run

```bash
./omacut              # then click Open…
./omacut clip.mp4     # or open a file directly
```

## Using it

- **Open…** loads a video, shows its first frame, and builds a thumbnail
  filmstrip.
- Drag the **left/right yellow handles** to set the start and end.
- Click inside the strip or drag the white playhead to **scrub**.
- **Play / Space** previews just the selected clip (auto-stops at the end).
- **Export…** writes the trimmed video next to the original.

### Lossless vs. precise — chosen automatically

A stream copy is instant and lossless but can only *start* on a keyframe. So
omacut probes the video's keyframes and picks the cut method based on where your
start handle sits, shown live next to the Export button:

- **Lossless · instant** — your start is on a keyframe (or you didn't trim the
  start). ffmpeg copies the streams (`-c copy`); no quality loss, near-instant.
- **Precise · re-encodes** — your start falls between keyframes, so omacut
  re-encodes (libx264 / aac) to cut at the exact frame you chose.

## Source layout

| File | Role |
|------|------|
| `Main.qml`        | The window: video preview, controls, file dialogs. |
| `TrimBar.qml`     | The filmstrip + draggable trim handles (custom QML). |
| `main.cpp`        | Entry point: sets the Material style, wires QML ↔ C++. |
| `backend.*`       | QObject exposed to QML: load, thumbnails, keyframes, export. |
| `thumbprovider.*` | Serves filmstrip thumbnails to QML `Image` elements. |
| `thumbworker.*`   | Background thread that builds the filmstrip thumbnails. |
| `keyframeworker.*`| Background thread that probes keyframe timestamps. |
| `ffmpeg.*`        | ffprobe/ffmpeg wrappers (probe, thumbnail, trim). |
| `resources.qrc`   | Embeds the QML into the binary. |
| `omacut.pro`      | qmake project file. |
