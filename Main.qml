import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs
import QtMultimedia

ApplicationWindow {
    id: win
    width: 960
    height: 680
    minimumWidth: 640
    minimumHeight: 460
    visible: true
    title: backend.source.toString() === "" ? "omacut" : "omacut — " + fileName(backend.source)

    Material.theme: Material.Dark
    Material.accent: "#FFD60A"
    color: "#0e0e10"

    function pad2(n) { return (n < 10 ? "0" : "") + n; }
    function fmt(sec) {
        if (sec < 0 || isNaN(sec)) sec = 0;
        var m = Math.floor(sec / 60);
        var s = sec - m * 60;
        return pad2(m) + ":" + (s < 10 ? "0" : "") + s.toFixed(2);
    }
    function fileName(url) {
        var s = url.toString();
        return s === "" ? "" : decodeURIComponent(s.substring(s.lastIndexOf('/') + 1));
    }
    function togglePlay() {
        if (player.playbackState === MediaPlayer.PlayingState) {
            player.pause();
            return;
        }
        var pos = player.position / 1000;
        if (pos < trimBar.startSec || pos >= trimBar.endSec)
            player.position = Math.round(trimBar.startSec * 1000);
        player.play();
    }

    MediaPlayer {
        id: player
        source: backend.source
        videoOutput: videoOut
        audioOutput: AudioOutput {}

        // Render the opening frame on load instead of showing black: a quick
        // play→pause makes the decoder present the first frame at position 0.
        property bool primed: false
        onMediaStatusChanged: {
            if (mediaStatus === MediaPlayer.LoadedMedia && !primed) {
                primed = true;
                play();
                pause();
            }
        }
        onPositionChanged: {
            // Stop at the trim end, like a clip preview.
            if (playbackState === MediaPlayer.PlayingState && position / 1000 >= trimBar.endSec) {
                pause();
                position = Math.round(trimBar.endSec * 1000);
            }
            if (!trimBar.interacting)
                trimBar.playheadSec = position / 1000;
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        // --- video preview ---
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: "black"
            clip: true

            VideoOutput {
                id: videoOut
                anchors.fill: parent
            }
            Label {
                anchors.centerIn: parent
                visible: backend.source.toString() === ""
                text: "Open a video to begin"
                color: "#5a5a5e"
                font.pixelSize: 16
            }
        }

        // --- trim bar ---
        TrimBar {
            id: trimBar
            Layout.fillWidth: true
            durationSec: backend.duration
            thumbCount: backend.thumbCount
            thumbRevision: backend.thumbRevision
            onScrub: (seconds) => player.position = Math.round(seconds * 1000)
        }

        // --- controls ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Open…"
                onClicked: openDialog.open()
            }
            Button {
                text: player.playbackState === MediaPlayer.PlayingState ? "Pause" : "Play"
                enabled: backend.duration > 0
                onClicked: togglePlay()
            }

            Item { Layout.fillWidth: true }

            Label {
                text: backend.duration > 0
                      ? fmt(trimBar.startSec) + "  →  " + fmt(trimBar.endSec)
                        + "    (" + fmt(trimBar.endSec - trimBar.startSec) + ")"
                      : "—"
                color: "#d6d6da"
                font.pixelSize: 14
                font.family: "monospace"
            }

            Item { Layout.fillWidth: true }

            // Auto-detected cut method. Re-evaluates when the start handle moves
            // (trimBar.startSec) and when keyframe analysis lands
            // (backend.analysisRevision).
            RowLayout {
                spacing: 6
                visible: backend.duration > 0
                property bool reenc: (backend.analysisRevision, backend.willReencode(trimBar.startSec))

                Rectangle {
                    width: 8; height: 8; radius: 4
                    Layout.alignment: Qt.AlignVCenter
                    color: parent.reenc ? "#FFD60A" : "#5bd66f"
                }
                Label {
                    text: parent.reenc ? "Precise cut · re-encodes" : "Lossless cut · instant"
                    color: "#b8b8bc"
                    font.pixelSize: 13
                    ToolTip.visible: hovered
                    ToolTip.text: parent.reenc
                        ? "Your start isn't on a keyframe, so the clip is re-encoded for a frame-accurate cut (slower)."
                        : "The cut lands on a keyframe, so it's copied losslessly and instantly."
                    HoverHandler { id: hoverHandler }
                    property bool hovered: hoverHandler.hovered
                }
            }
            Button {
                text: "Export…"
                highlighted: true
                enabled: backend.duration > 0 && !backend.busy
                onClicked: {
                    saveDialog.currentFolder = backend.sourceFolder();
                    saveDialog.selectedFile = backend.suggestedExportUrl();
                    saveDialog.open();
                }
            }
        }

        // --- status line ---
        Label {
            Layout.fillWidth: true
            visible: backend.status !== ""
            text: backend.status
            color: "#8a8a8e"
            font.pixelSize: 12
            elide: Text.ElideMiddle
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: backend.busy
        visible: backend.busy
    }

    Connections {
        target: backend
        function onInfoChanged() {
            player.primed = false;
            trimBar.startSec = 0;
            trimBar.endSec = backend.duration;
            trimBar.playheadSec = 0;
        }
        function onExportDone(path) {
            messageDialog.title = "Done";
            messageDialog.text = "Trimmed video saved to:\n" + path;
            messageDialog.open();
        }
        function onExportFailed(message) {
            messageDialog.title = "Export failed";
            messageDialog.text = message;
            messageDialog.open();
        }
        function onLoadError(message) {
            messageDialog.title = "Cannot open video";
            messageDialog.text = message;
            messageDialog.open();
        }
    }

    FileDialog {
        id: openDialog
        title: "Open video"
        nameFilters: ["Video files (*.mp4 *.mov *.mkv *.avi *.webm *.m4v)", "All files (*)"]
        onAccepted: backend.load(selectedFile)
    }

    FileDialog {
        id: saveDialog
        title: "Export trimmed video"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Video files (*.mp4 *.mov *.mkv *.avi *.webm *.m4v)", "All files (*)"]
        onAccepted: backend.exportClip(selectedFile, trimBar.startSec, trimBar.endSec)
    }

    MessageDialog {
        id: messageDialog
        buttons: MessageDialog.Ok
    }
}
