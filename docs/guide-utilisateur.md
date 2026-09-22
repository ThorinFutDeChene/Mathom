# Guide utilisateur

Ce guide décrit le comportement actuellement validé de Mathom 0.1.0.

## Créer et organiser une Mathom-House

Les structures peuvent être créées depuis les commandes de création de l'interface, notamment via le bouton **Nouveau** ou le menu contextuel.

Une Mathom-House peut contenir des étagères et des Mathoms.

## Créer des Mathoms

La base héritée de BasKet permet de conserver plusieurs types de contenus, notamment :

- texte ;
- liens et URL ;
- images ;
- fichiers ;
- sons ;
- lanceurs d'applications ou de commandes.

Le type exact disponible dépend du contexte et des fonctions encore présentes dans l'interface.

## Déplacer un Mathom

Les Mathoms sont réorganisables par glisser-déposer.

Il est possible de :

- déplacer un Mathom avant ou après un autre ;
- déplacer un Mathom vers une autre étagère ;
- déplacer un Mathom vers une autre Mathom-House ;
- extraire un élément d'un groupe.

Lorsqu'un seul élément d'un groupe est déplacé hors de ce groupe, il est automatiquement dissocié du groupe d'origine.

## Groupes

Plusieurs Mathoms peuvent être organisés ensemble afin de former un groupe visuel.

Le groupement ne transforme pas les Mathoms en un contenu unique : chaque élément reste manipulable.

## Lanceurs

Mathom 0.1.0 contient un correctif spécifique du mécanisme de lanceur.

La commande d'un lanceur peut être définie manuellement. Le sélecteur d'applications reste également utilisable.

Le traitement des entrées `Exec=` issues des fichiers `.desktop` a été corrigé afin que la commande réellement exécutable soit utilisée.

Pour les commandes destinées à un terminal, Mathom s'appuie sur le terminal configuré par l'environnement.

## Images

Les images peuvent être insérées comme Mathoms et déplacées comme les autres éléments.

La gestion de leur redimensionnement doit encore faire l'objet d'une validation fonctionnelle complète dans Mathom ; la documentation ne considère donc pas encore ce comportement comme stabilisé.

## Langue de l'interface

Mathom utilise l'infrastructure de traduction KDE. L'interface suit la langue de l'environnement lorsqu'une traduction correspondante est disponible.

La traduction française et la terminologie Mathom ont été intégrées au fork.

## Sauvegarde et restauration

Mathom conserve les mécanismes de sauvegarde issus de BasKet. Le profil utilisateur de Mathom est cependant séparé de celui de BasKet.

Sous Linux, le profil par défaut utilise le répertoire XDG de données sous le nom `mathom`.

Pour une migration depuis une installation existante de BasKet, voir [migration-basket.md](migration-basket.md).
