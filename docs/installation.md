# Installation et construction

## État actuel

Mathom 0.1.0 est encore en développement. Le dépôt contient un script reproductible destiné à construire un paquet Debian autonome pour architecture **amd64**.

## Construction du paquet Debian

Le script est :

```sh
./scripts/build-mathom-deb.sh
```

Il utilise par défaut :

```text
MATHOM_VERSION=0.1.0
MATHOM_DEB_REVISION=1
ARCH=amd64
```

Le paquet produit est :

```text
packaging/mathom_0.1.0-1_amd64.deb
```

Les variables peuvent être surchargées :

```sh
MATHOM_VERSION=0.1.0 MATHOM_DEB_REVISION=2 ./scripts/build-mathom-deb.sh
```

## Dépendances du processus de packaging

Le script actuel s'appuie sur :

- Flatpak ;
- `flatpak-builder` ;
- le SDK KDE défini dans `.flatpak-manifest.json` ;
- `linuxdeploy-x86_64.AppImage` ;
- `linuxdeploy-plugin-qt-x86_64.AppImage`.

Les deux AppImage de linuxdeploy sont attendues dans :

```text
packaging/tools/
```

Le script :

1. compile Mathom dans l'environnement Flatpak KDE ;
2. crée `packaging/Mathom.AppDir` ;
3. déploie les dépendances Qt/KF6 ;
4. applique les correctifs de runtime nécessaires ;
5. construit l'arborescence Debian ;
6. génère le paquet avec `dpkg-deb` ;
7. contrôle l'identité du paquet et recherche d'anciennes traces utilisateur de BasKet/Nightly.

## Installation du paquet construit

Après construction :

```sh
sudo apt install ./packaging/mathom_0.1.0-1_amd64.deb
```

L'exécutable installé est :

```text
/usr/bin/mathom
```

L'application autonome est placée sous :

```text
/opt/mathom
```

## Construction directe depuis les sources

La base de compilation reste celle de BasKet : CMake, Qt 6 et KDE Frameworks 6.

Les versions minimales actuellement déclarées sont :

- CMake 3.16 ;
- Qt 6.5.0 ;
- KDE Frameworks 6.9.0 ;
- LibGit2 1.8.4 lorsque le support Git est activé.

Exemple de construction locale :

```sh
mkdir -p build
cd build
cmake .. \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local/kde" \
  -DKDE_INSTALL_PLUGINDIR="$HOME/.local/kde/lib64/qt6/plugins"
cmake --build . -j8
cmake --install .
```

Le binaire généré par le fork est `mathom`.

## Remarque sur les noms internes

Le fichier CMake racine déclare encore historiquement `project(Basket ...)` et certaines variables restent préfixées `BASKET_`.

Ce n'est pas l'identité utilisateur de l'application. Ces noms internes seront traités séparément afin de ne pas provoquer de régression inutile pendant la stabilisation du fork.
