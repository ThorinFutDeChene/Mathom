#!/bin/sh
set -eu

VERSION="${MATHOM_VERSION:-0.1.9}"
REVISION="${MATHOM_DEB_REVISION:-0dev2pages1}"
ARCH="amd64"

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
PACKAGING="$ROOT/packaging"
APPDIR="$PACKAGING/Mathom.AppDir"
DEBROOT="$PACKAGING/mathom-debroot"
OUTPUT="$PACKAGING/mathom_${VERSION}-${REVISION}_${ARCH}.deb"
MANIFEST="$ROOT/.flatpak-manifest.json"

LINUXDEPLOY="$PACKAGING/tools/linuxdeploy-x86_64.AppImage"

cd "$ROOT"

echo "=== Mathom ${VERSION}-${REVISION} : construction ==="

if [ ! -x "$LINUXDEPLOY" ]; then
    echo "Erreur : linuxdeploy absent ou non exécutable :"
    echo "$LINUXDEPLOY"
    exit 1
fi

if [ ! -x "$PACKAGING/tools/linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
    echo "Erreur : plugin Qt de linuxdeploy absent."
    exit 1
fi

echo
echo "=== 1/7 Compilation dans le SDK KDE ==="

flatpak-builder \
    --user \
    --force-clean \
    flatpak-build \
    "$MANIFEST"

echo
echo "=== 2/7 Création du Mathom.AppDir ==="

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr"

cp -a "$ROOT/flatpak-build/files/." "$APPDIR/usr/"
rm -rf "$APPDIR/usr/lib/debug"

ln -s usr/bin/mathom "$APPDIR/AppRun"

if [ ! -x "$APPDIR/usr/bin/mathom" ]; then
    echo "Erreur : le binaire mathom n'a pas été produit."
    exit 1
fi

ICON="$APPDIR/usr/share/icons/hicolor/scalable/apps/fr.thorinux.mathom.svg"

if [ ! -s "$ICON" ]; then
    echo "Erreur : l'icône principale Mathom est absente :"
    echo "$ICON"
    exit 1
fi

echo
echo "=== 3/7 Déploiement Qt/KF6 ==="

flatpak-builder --run flatpak-build "$MANIFEST" \
    env \
    APPIMAGE_EXTRACT_AND_RUN=1 \
    PATH="$PACKAGING/tools:$PATH" \
    "$LINUXDEPLOY" \
        --appdir "$APPDIR" \
        --executable "$APPDIR/usr/bin/mathom" \
        --desktop-file "$APPDIR/usr/share/applications/fr.thorinux.mathom.desktop" \
        --icon-file "$ICON" \
        --plugin qt

echo
echo "=== 4/7 Correctifs du runtime natif ==="

flatpak-builder --run flatpak-build "$MANIFEST" \
    sh -c '
        set -eu

        APPDIR="'"$APPDIR"'"

        wayland="$(readlink -f /usr/lib/x86_64-linux-gnu/libwayland-client.so.0)"
        cp -a "$wayland" "$APPDIR/usr/lib/"
        ln -sf "$(basename "$wayland")" \
            "$APPDIR/usr/lib/libwayland-client.so.0"

        cp -a /usr/bin/kbuildsycoca6 \
            "$APPDIR/usr/bin/kbuildsycoca6"

        # KIO file worker required by Mathom for local file operations.
        # linuxdeploy does not automatically deploy the worker and plugin.
        mkdir -p \
            "$APPDIR/usr/lib/x86_64-linux-gnu/libexec/kf6" \
            "$APPDIR/usr/lib/plugins/kf6/kio"

        cp -a \
            /usr/lib/x86_64-linux-gnu/libexec/kf6/kioworker \
            "$APPDIR/usr/lib/x86_64-linux-gnu/libexec/kf6/"

        cp -a \
            /usr/lib/plugins/kf6/kio/kio_file.so \
            "$APPDIR/usr/lib/plugins/kf6/kio/"

        # BasKet/KF6 relies on many standard KDE icon names. Ubuntu MATE
        # does not provide all of them, so Mathom ships Breeze as a fallback
        # theme while still allowing the desktop theme to remain primary.
        if [ ! -d /usr/share/icons/breeze ]; then
            echo "Erreur : le thème Breeze est absent du SDK KDE."
            exit 1
        fi
        mkdir -p "$APPDIR/usr/share/icons"
        rm -rf "$APPDIR/usr/share/icons/breeze"
        cp -a /usr/share/icons/breeze "$APPDIR/usr/share/icons/"

        # The AppDir is executed outside Flatpak. Qt/KF6 libraries alone are
        # not enough: their standard actions (Cut/Copy/Paste/etc.) are
        # translated by framework catalogs supplied by the KDE runtime.
        # Ship runtime catalogs for the Mathom languages validated for
        # native Debian packages.
        for locale in fr ar tr uk; do
            source_dir="/usr/share/locale/$locale/LC_MESSAGES"
            destination_dir="$APPDIR/usr/share/locale/$locale/LC_MESSAGES"

            [ -d "$source_dir" ] || continue
            mkdir -p "$destination_dir"

            for mo in "$source_dir"/*.mo; do
                [ -f "$mo" ] || continue
                cp -a "$mo" "$destination_dir/"
            done
        done
    '

if env LD_LIBRARY_PATH="$APPDIR/usr/lib" \
    ldd "$APPDIR/usr/bin/kbuildsycoca6" | grep -q 'not found'; then
    echo "Erreur : bibliothèque manquante pour kbuildsycoca6 :"
    env LD_LIBRARY_PATH="$APPDIR/usr/lib" \
        ldd "$APPDIR/usr/bin/kbuildsycoca6" | grep 'not found'
    exit 1
fi

echo
echo "=== 5/7 Création de l'arborescence Debian ==="

rm -rf "$DEBROOT"

mkdir -p \
    "$DEBROOT/DEBIAN" \
    "$DEBROOT/opt" \
    "$DEBROOT/usr/bin" \
    "$DEBROOT/usr/share/applications" \
    "$DEBROOT/usr/share/metainfo"

cp -a "$APPDIR" "$DEBROOT/opt/mathom"

cat > "$DEBROOT/usr/bin/mathom" <<'WRAPPER'
#!/bin/sh

APPDIR="/opt/mathom"

if [ -z "${XDG_MENU_PREFIX:-}" ]; then
    if [ -f /etc/xdg/menus/mate-applications.menu ]; then
        export XDG_MENU_PREFIX="mate-"
    elif [ -f /etc/xdg/menus/gnome-applications.menu ]; then
        export XDG_MENU_PREFIX="gnome-"
    elif [ -f /etc/xdg/menus/plasma-applications.menu ]; then
        export XDG_MENU_PREFIX="plasma-"
    elif [ -f /etc/xdg/menus/kf5-applications.menu ]; then
        export XDG_MENU_PREFIX="kf5-"
    fi
fi

export XDG_DATA_DIRS="$APPDIR/usr/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"
export PATH="$APPDIR/usr/bin:$PATH"

# The self-contained KF6 runtime must search its own translations as well as
# the host locale tree. This keeps the installed .deb identical to the lab
# build for KStandardAction/KXmlGui strings.
export XLOCALEDIR="$APPDIR/usr/share/locale"

if [ -x "$APPDIR/usr/bin/kbuildsycoca6" ]; then
    env \
        LD_LIBRARY_PATH="$APPDIR/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
        XDG_DATA_DIRS="$XDG_DATA_DIRS" \
        XDG_MENU_PREFIX="${XDG_MENU_PREFIX:-}" \
        "$APPDIR/usr/bin/kbuildsycoca6" >/dev/null 2>&1 || true
fi

exec "$APPDIR/AppRun" "$@"
WRAPPER

chmod 755 "$DEBROOT/usr/bin/mathom"

cp "$APPDIR/usr/share/applications/fr.thorinux.mathom.desktop" \
    "$DEBROOT/usr/share/applications/"

cp "$APPDIR/usr/share/metainfo/fr.thorinux.mathom.metainfo.xml" \
    "$DEBROOT/usr/share/metainfo/"

# Install the validated Mathom launcher icon directly for native desktops.
# This happens after Flatpak/AppStream composition, so it cannot break the
# compose step. MATE reliably resolves the hicolor PNG, with pixmaps fallback.
install -Dm644 "$ROOT/resources/icons/app/mathom.png" \
    "$DEBROOT/usr/share/icons/hicolor/48x48/apps/fr.thorinux.mathom.png"
install -Dm644 "$ROOT/resources/icons/app/mathom.png" \
    "$DEBROOT/usr/share/pixmaps/fr.thorinux.mathom.png"

# Installer les icônes Mathom disponibles (PNG et SVG).
find "$APPDIR/usr/share/icons/hicolor" \
    -type f \( -name 'fr.thorinux.mathom.png' -o -name 'fr.thorinux.mathom.svg' \) |
while IFS= read -r icon; do
    relative="${icon#"$APPDIR/usr/share/"}"
    destination="$DEBROOT/usr/share/$(dirname "$relative")"
    mkdir -p "$destination"
    cp "$icon" "$destination/"
done

cat > "$DEBROOT/DEBIAN/control" <<CONTROL
Package: mathom
Version: ${VERSION}-${REVISION}
Section: office
Priority: optional
Architecture: ${ARCH}
Maintainer: Thorinux Systems
Depends: libc6
Description: Mathom - notes and information organizer
 Mathom is an application for recording ideas as mathoms and
 organizing them into Mathom-Houses and shelves.
 .
 Mathom is developed by Thorinux Systems and is based on
 BasKet Note Pads.
CONTROL

cat > "$DEBROOT/DEBIAN/postinst" <<'POSTINST'
#!/bin/sh
set -e

# MATE/GNOME can keep the old launcher icon in cache after an upgrade.
# Refresh the caches when the tools are available, without making them
# mandatory dependencies of Mathom.
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -q /usr/share/icons/hicolor || true
fi

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database -q /usr/share/applications || true
fi

exit 0
POSTINST

cat > "$DEBROOT/DEBIAN/postrm" <<'POSTRM'
#!/bin/sh
set -e

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -q /usr/share/icons/hicolor || true
fi

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database -q /usr/share/applications || true
fi

exit 0
POSTRM

chmod 755 "$DEBROOT/DEBIAN/postinst" "$DEBROOT/DEBIAN/postrm"

echo
echo "=== 6/7 Construction du paquet ==="

rm -f "$OUTPUT"

dpkg-deb --build --root-owner-group \
    "$DEBROOT" \
    "$OUTPUT"

echo
echo "=== 7/7 Contrôles ==="

test "$(dpkg-deb -f "$OUTPUT" Package)" = "mathom"
test "$(dpkg-deb -f "$OUTPUT" Version)" = "${VERSION}-${REVISION}"
test "$(dpkg-deb -f "$OUTPUT" Architecture)" = "$ARCH"

CONTENTS_LIST="$PACKAGING/mathom-deb-contents.txt"
dpkg-deb -c "$OUTPUT" > "$CONTENTS_LIST"

grep -q './opt/mathom/usr/share/icons/breeze/index.theme' "$CONTENTS_LIST"
for locale in fr ar tr uk; do
    grep -q "./opt/mathom/usr/share/locale/$locale/LC_MESSAGES/" "$CONTENTS_LIST"
done

# KIO local-file support
grep -q './opt/mathom/usr/lib/x86_64-linux-gnu/libexec/kf6/kioworker' "$CONTENTS_LIST"
grep -q './opt/mathom/usr/lib/plugins/kf6/kio/kio_file.so' "$CONTENTS_LIST"
grep -q './usr/share/icons/hicolor/scalable/apps/fr.thorinux.mathom.svg' "$CONTENTS_LIST"
grep -q './usr/share/icons/hicolor/48x48/apps/fr.thorinux.mathom.png' "$CONTENTS_LIST"
grep -q './usr/share/pixmaps/fr.thorinux.mathom.png' "$CONTENTS_LIST"

if grep -Eq '/opt/basket(/|$)|org\.kde\.basket\.desktop|Mathom \(Nightly\)' "$CONTENTS_LIST"; then
    echo "Erreur : ancienne identité BasKet/Nightly trouvée dans le paquet."
    exit 1
fi

rm -f "$CONTENTS_LIST"

echo
echo "Paquet créé avec succès :"
ls -lh "$OUTPUT"
