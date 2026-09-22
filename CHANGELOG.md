# Changelog

Toutes les évolutions propres à Mathom doivent être consignées ici.

## [0.1.0] - Unreleased

### Added

- identité applicative Mathom ;
- exécutable `mathom` ;
- Desktop ID `fr.thorinux.mathom` ;
- terminologie Mathom et traduction française ;
- profil de données natif Mathom ;
- migration au premier démarrage depuis BasKet natif ou Flatpak ;
- sélection automatique du profil BasKet le plus récent lorsqu'il en existe plusieurs ;
- conservation du profil BasKet source pendant la migration ;
- script reproductible `scripts/build-mathom-deb.sh` ;
- génération d'un paquet Debian autonome amd64.

### Changed

- configuration utilisateur déplacée vers `mathomrc` ;
- données utilisateur déplacées vers l'espace XDG `mathom/` ;
- ressources Mathom prioritaires avec chemins de compatibilité BasKet ;
- nom de bureau normalisé en « Mathom » sans suffixe Nightly.

### Fixed

- édition manuelle des commandes de lanceur ;
- traitement des commandes `Exec=` ;
- exécution des lanceurs et intégration avec le terminal ;
- plusieurs chaînes et dialogues encore liés à l'ancienne terminologie.

### Compatibility

- conservation de la lecture des données et archives BasKet ;
- maintien des anciens chemins de ressources lorsqu'ils sont nécessaires à la compatibilité.
