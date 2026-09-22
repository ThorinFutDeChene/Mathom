# Concepts et vocabulaire

## Pourquoi « Mathom » ?

Dans Mathom, une information n'est pas seulement une « note ». Elle peut être un texte, un lien, une image, un fichier, un lanceur ou un autre élément que l'on souhaite conserver parce qu'il pourra être utile plus tard.

Le mot **Mathom** est utilisé pour représenter cet objet conservé.

## Hiérarchie

### Mathom-House

Une **Mathom-House** est l'espace principal dans lequel sont rangés les Mathoms.

Elle correspond au niveau fonctionnel historiquement appelé « basket » dans BasKet.

### Étagère

Une **Étagère** est une subdivision hiérarchique d'une Mathom-House.

Elle sert à structurer un ensemble de Mathoms sans imposer une organisation rigide. Les éléments peuvent être déplacés d'une étagère à une autre par glisser-déposer.

### Mathom

Un **Mathom** est l'unité d'information manipulée dans l'application.

Un Mathom peut notamment contenir du texte, un lien, une image, un fichier ou un lanceur.

## Organisation libre

Mathom conserve la logique de disposition libre héritée de BasKet :

- les Mathoms peuvent être déplacés ;
- leur ordre peut être modifié ;
- ils peuvent être groupés ;
- un élément peut être extrait d'un groupe ;
- un Mathom peut être déplacé vers une autre étagère ou une autre Mathom-House.

L'objectif est de permettre une organisation visuelle rapide sans obliger l'utilisateur à passer par une structure de base de données ou une hiérarchie de fichiers classique.

## Terminologie et compatibilité

Le vocabulaire affiché à l'utilisateur est celui de Mathom. En revanche, certains noms internes restent hérités de BasKet.

Exemples :

| Interface Mathom | Héritage interne possible |
|---|---|
| Mathom-House | basket |
| Étagère | sous-basket / structure hiérarchique |
| Mathom | note |
| Mathom | noms de classes ou fichiers contenant encore `basket` |

Cette coexistence est volontaire pendant la phase de transition : la priorité est d'obtenir un logiciel stable et compatible avant d'effectuer les renommages internes non indispensables.
