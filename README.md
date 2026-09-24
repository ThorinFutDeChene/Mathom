# Mathom

Mathom est un gestionnaire de notes et de connaissances développé par **Thorinux Systems** à partir de **BasKet Note Pads**.

L'application organise les informations selon trois niveaux :

- **Mathom-House** : espace principal ;
- **Étagère / sous-étagère** : classement hiérarchique ;
- **Mathom** : note ou contenu.

## Version

Version publiée : **0.1.1**

## Nouveautés de la 0.1.1

- nouvelle identité graphique Mathom ;
- nouveau logo de l'application ;
- icône coffre pour les Mathom-Houses ;
- icône parchemin + plume pour les étagères et sous-étagères ;
- conservation des icônes personnalisées ;
- restauration des icônes historiques des marques BasKet ;
- catalogue central d’icônes Mathom et restauration autonome des icônes de Bienvenue ;
- interface française ;
- paquet Debian autonome pour Ubuntu 24.04 LTS amd64.

## Installation Ubuntu / Debian

Télécharger le paquet `.deb` depuis la page des Releases GitHub, puis :

```bash
sudo apt install ./mathom_0.1.1-8_amd64.deb
```

Le paquet installe Mathom dans `/opt/mathom` avec les bibliothèques Qt/KF6 nécessaires à son exécution.

## Construction du paquet Debian

Le script de construction officiel est :

```bash
./scripts/build-mathom-deb.sh
```

La compilation utilise le SDK KDE 6.9 via Flatpak, puis construit un paquet Debian autonome.

Prérequis principaux :

- `flatpak-builder` ;
- `org.kde.Sdk//6.9` ;
- `org.kde.Platform//6.9` ;
- les outils `linuxdeploy` présents dans `packaging/tools`.

Le paquet produit est :

```text
packaging/mathom_0.1.1-8_amd64.deb
```

## Développement

Le binaire est `mathom` et l'identifiant d'application est :

```text
fr.thorinux.mathom
```

Mathom utilise Qt 6 et KDE Frameworks 6.

## Origine et licence

Mathom est un fork de **BasKet Note Pads** et conserve l'historique, les auteurs et les licences du projet d'origine.

Licence : **GPL-2.0-or-later**.
