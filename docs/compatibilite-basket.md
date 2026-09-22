# Compatibilité avec BasKet

## Principe

Mathom est un fork de BasKet Note Pads, pas une réécriture complète.

La compatibilité des données existantes est donc une contrainte de conception.

## Archives `.baskets`

Le format d'archive historique de BasKet doit rester lisible.

La documentation héritée décrit actuellement l'archive comme un flux contenant notamment :

- l'en-tête `BasKetNP:archive` ;
- une version de format historique `0.6.1` ;
- une image d'aperçu PNG ;
- une archive de contenu compressée.

Même si l'application s'appelle désormais Mathom, l'en-tête historique ne doit pas être renommé arbitrairement : il fait partie du format de fichier et donc de la compatibilité.

## Profil utilisateur

Mathom utilise son propre profil :

- données : espace XDG `mathom/` ;
- configuration : `mathomrc`.

Les profils BasKet restent séparés.

Le premier démarrage peut en copier le contenu automatiquement. Voir [migration-basket.md](migration-basket.md).

## Ressources historiques

Certaines ressources et certains noms internes restent volontairement compatibles avec BasKet :

- chemins de secours pour les arrière-plans ;
- classes C++ héritées ;
- noms de bibliothèques comme `LibBasket` ;
- variables CMake `BASKET_*` ;
- certains noms de fichiers ou de modules ;
- domaine de traduction historique tant que sa migration complète n'est pas validée.

## Règle de développement

Un renommage interne n'est pas considéré comme une amélioration s'il dégrade :

- la lecture des anciennes données ;
- l'import d'un profil BasKet ;
- les traductions ;
- les plugins ou modules KDE ;
- les archives `.baskets`.

Toute rupture volontaire de compatibilité devra être accompagnée d'une migration documentée et testée.
