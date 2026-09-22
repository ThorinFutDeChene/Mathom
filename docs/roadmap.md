# Feuille de route

Cette feuille de route décrit l'ordre de travail actuel sans figer les futures versions.

## Socle Mathom — réalisé

Les éléments suivants sont déjà présents dans la branche principale :

- fork opérationnel de BasKet Note Pads ;
- correction du lanceur ;
- identité utilisateur Mathom ;
- version applicative 0.1.0 ;
- terminologie Mathom et traduction française ;
- exécutable `mathom` ;
- Desktop ID `fr.thorinux.mathom` ;
- profil `mathom/` et configuration `mathomrc` ;
- migration automatique depuis BasKet natif ou Flatpak ;
- conservation de la compatibilité des ressources historiques ;
- script de construction d'un paquet Debian autonome.

## Phase actuelle — identité et documentation

Priorités :

- finaliser le **logo Mathom** et son intégration dans toutes les tailles nécessaires ;
- disposer d'une documentation GitHub propre ;
- formaliser le vocabulaire Mathom-House / Étagère / Mathom ;
- documenter l'installation, la migration et la compatibilité ;
- continuer à éliminer les anciennes mentions utilisateur de BasKet sans casser les formats historiques.

## Stabilisation 0.1.x

Avant de considérer la base comme stable :

- tester une installation propre du paquet `.deb` ;
- tester une mise à jour d'un paquet Mathom existant ;
- rejouer la migration BasKet sur plusieurs profils ;
- vérifier les principaux types de Mathoms ;
- vérifier le comportement des groupes et déplacements ;
- valider les lanceurs ;
- compléter les tests de l'interface française ;
- identifier les fonctions héritées encore non validées, par exemple certains comportements liés aux images.

## Après stabilisation

Les évolutions fonctionnelles propres à Mathom devront être ajoutées progressivement, en conservant trois règles :

1. ne pas casser les données de l'utilisateur ;
2. maintenir le multilingue ;
3. préserver la compatibilité BasKet tant qu'aucune migration explicite ne la remplace.
