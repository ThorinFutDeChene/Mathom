# Mathom

![Logo Mathom](logo.png)

**Mathom** est un logiciel libre de prise de notes et d'organisation d'informations pour Linux, développé par **Thorinux Systems** à partir de **BasKet Note Pads**.

Le principe de Mathom est de conserver des informations hétérogènes — texte, liens, images, fichiers, lanceurs et autres contenus — puis de les organiser librement afin de pouvoir les retrouver et les réutiliser rapidement.

> État du projet : **Mathom 0.1.0 — développement actif**.

## Concepts

Mathom adopte un vocabulaire propre tout en conservant la compatibilité avec les données de BasKet :

- **Mathom-House** : espace principal de classement ;
- **Étagère** : subdivision hiérarchique d'une Mathom-House ;
- **Mathom** : information ou objet conservé dans une Mathom-House.

L'interface est multilingue et utilise la langue de l'environnement de bureau lorsqu'une traduction est disponible.

## État actuel

La base fonctionnelle de BasKet est conservée et sert de socle à Mathom. Les premiers travaux propres au fork sont déjà intégrés :

- identité applicative **Mathom 0.1.0** ;
- exécutable `mathom` ;
- identifiant de bureau `fr.thorinux.mathom` ;
- terminologie Mathom et traduction française ;
- correction du fonctionnement des lanceurs ;
- profil natif Mathom distinct du profil BasKet ;
- migration automatique des données BasKet au premier démarrage ;
- script reproductible de construction d'un paquet Debian `.deb` autonome ;
- maintien de la compatibilité avec les archives et données BasKet.

Certaines structures internes portent encore des noms historiques comme `basket`, `LibBasket` ou `BASKET_VERSION`. Elles ne constituent pas l'identité publique du logiciel et sont conservées pour limiter les régressions pendant la transition.

## Installation

La méthode de construction actuelle du paquet Debian est documentée dans [docs/installation.md](docs/installation.md).

Le script principal est :

```sh
./scripts/build-mathom-deb.sh
```

Par défaut, il produit :

```text
packaging/mathom_0.1.0-1_amd64.deb
```

## Documentation

La documentation du projet est organisée dans le dossier [docs](docs/README.md) :

- [Concepts et vocabulaire](docs/concepts.md)
- [Guide utilisateur](docs/guide-utilisateur.md)
- [Installation et construction](docs/installation.md)
- [Migration depuis BasKet](docs/migration-basket.md)
- [Compatibilité avec BasKet](docs/compatibilite-basket.md)
- [Développement](docs/developpement.md)
- [Feuille de route](docs/roadmap.md)

L'ancien dossier `doc/` provient de BasKet et contient notamment le manuel KDE au format DocBook. Il reste utile comme source technique, mais n'est pas encore la documentation canonique de Mathom.

## Identité technique

| Élément | Valeur |
|---|---|
| Nom | Mathom |
| Version | 0.1.0 |
| Mainteneur du fork | Thorinux Systems |
| Exécutable | `mathom` |
| Desktop ID | `fr.thorinux.mathom` |
| Configuration | `mathomrc` |
| Données utilisateur | `mathom/` dans le répertoire XDG de données |
| Licence principale | GPL-2.0-or-later |

## Compatibilité BasKet

Mathom est un fork de BasKet Note Pads et conserve volontairement la compatibilité avec les données existantes. Le format d'archive historique `.baskets` et son en-tête BasKet ne doivent pas être renommés sans mécanisme de migration explicite.

Au premier démarrage, si aucun profil Mathom n'existe encore, Mathom peut importer automatiquement le profil BasKet natif ou Flatpak le plus récent **sans modifier le profil d'origine**.

Voir [docs/migration-basket.md](docs/migration-basket.md).

## Licence et crédits

Le code hérité conserve ses mentions de copyright et ses identifiants SPDX. Le code principal est distribué sous **GPL-2.0-or-later** ; certains composants hérités utilisent des licences LGPL, présentes dans le dossier `LICENSES/`.

Mathom est développé par Thorinux Systems sur la base du travail historique de **Sébastien Laoût**, **Gleb Baryshev** et des autres contributeurs de BasKet Note Pads.
