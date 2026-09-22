# Développement

## Base technique

Mathom repose actuellement sur :

- C++ ;
- Qt 6 ;
- KDE Frameworks 6 ;
- CMake / ECM ;
- LibGit2 pour les fonctions Git lorsque celles-ci sont activées.

Le fork est issu de BasKet Note Pads et conserve encore une partie importante de son architecture interne.

## Identité publique de Mathom

Les nouvelles évolutions doivent utiliser l'identité suivante lorsqu'elles sont visibles par l'utilisateur :

| Élément | Valeur |
|---|---|
| Nom d'application | Mathom |
| Version actuelle | 0.1.0 |
| Exécutable | `mathom` |
| Desktop ID | `fr.thorinux.mathom` |
| Fichier desktop | `fr.thorinux.mathom.desktop` |
| Metainfo | `fr.thorinux.mathom.metainfo.xml` |
| Catégorie de journal | `fr.thorinux.mathom` |
| Configuration | `mathomrc` |
| Données | `mathom/` |

Les nouveaux textes utilisateur ne doivent pas réintroduire « BasKet » comme nom de l'application, sauf lorsqu'il est question de compatibilité, de migration ou de crédits.

## Héritage interne

Les noms historiques internes ne doivent pas être renommés en masse sans nécessité.

Exemples actuels :

- `LibBasket` ;
- `basket_SRCS` ;
- `BASKET_VERSION` ;
- fichiers `basket*.cpp` ;
- certains modules KCM ;
- en-têtes du format d'archive.

La priorité est la stabilité fonctionnelle.

## Internationalisation

Mathom doit rester multilingue.

Toute nouvelle chaîne visible par l'utilisateur doit passer par l'infrastructure KDE i18n et ne doit pas être codée en dur uniquement en français.

Le français constitue actuellement la traduction de référence utilisée pour valider la terminologie Mathom, mais le comportement applicatif doit continuer à suivre la langue du système.

## Compatibilité

Avant de valider une modification touchant au stockage ou au profil utilisateur, vérifier au minimum :

- ouverture du profil Mathom existant ;
- migration depuis un profil BasKet natif ;
- migration depuis un profil BasKet Flatpak ;
- lecture des archives `.baskets` ;
- conservation des arrière-plans historiques ;
- absence de modification destructive du profil BasKet source.

## Packaging

Le script de référence est :

```text
scripts/build-mathom-deb.sh
```

Il doit rester capable de construire un paquet amd64 reproductible et de refuser un paquet contenant des traces utilisateur évidentes de l'ancienne identité, notamment `/opt/basket`, `org.kde.basket.desktop` ou `Mathom (Nightly)`.

## Méthode de travail recommandée

Pour chaque évolution :

1. isoler la modification dans une branche ;
2. conserver une modification fonctionnelle identifiable ;
3. compiler ;
4. tester l'usage concerné ;
5. vérifier les impacts de compatibilité ;
6. mettre à jour la documentation et le changelog ;
7. intégrer seulement lorsque l'état est reproductible.

## Documentation embarquée

Le dossier `doc/` contient encore le manuel KDE DocBook hérité.

La documentation Markdown du dossier `docs/` sert de référence de projet pendant la transformation. Les informations stabilisées pourront ensuite être reprises dans le manuel DocBook embarqué.
