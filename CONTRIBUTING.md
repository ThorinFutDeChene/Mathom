# Contribuer à Mathom

Mathom est un fork de BasKet Note Pads maintenu par Thorinux Systems.

## Principes

Une contribution doit privilégier :

- la stabilité ;
- la compatibilité des données ;
- la clarté de l'interface ;
- le multilingue ;
- une évolution progressive plutôt qu'un renommage massif risqué.

## Avant de proposer une modification

Vérifier que la modification :

- compile avec Qt 6 et KDE Frameworks 6 ;
- n'introduit pas de nouveau texte utilisateur uniquement codé en dur ;
- n'altère pas un profil BasKet source pendant une migration ;
- ne change pas le format `.baskets` sans mécanisme de migration ;
- n'introduit pas une nouvelle identité utilisateur BasKet à la place de Mathom.

## Documentation

Toute évolution visible ou structurante doit mettre à jour au moins l'un des éléments suivants :

- `README.md` ;
- `docs/` ;
- `CHANGELOG.md`.

## Branches

Une branche par évolution est recommandée. Le nom doit décrire le sujet, par exemple :

```text
feature/...
fix/...
docs/...
packaging/...
```

## Tests minimaux

Selon la modification, vérifier :

- démarrage de Mathom ;
- création et ouverture d'un profil ;
- déplacement et organisation de Mathoms ;
- migration BasKet lorsque le stockage est concerné ;
- langue de l'interface lorsque des textes sont modifiés ;
- construction du paquet avec `scripts/build-mathom-deb.sh` lorsque le packaging est affecté.
