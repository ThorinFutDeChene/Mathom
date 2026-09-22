# Migration depuis BasKet

Mathom 0.1.0 possède un mécanisme de migration automatique au premier démarrage.

## Objectif

La migration permet de démarrer Mathom avec les données d'une installation BasKet existante tout en laissant cette installation **intacte**.

Mathom ne déplace pas les données BasKet : il les copie vers son propre profil.

## Conditions de déclenchement

La migration automatique est exécutée uniquement lorsque :

- aucun dossier de données personnalisé n'est fourni avec l'option `--data-folder` ;
- le dossier de données Mathom n'existe pas encore ;
- au moins un profil BasKet valide est détecté.

Une fois un profil Mathom créé, cette migration initiale n'est plus rejouée automatiquement.

## Profils recherchés

Mathom recherche actuellement deux sources.

### BasKet Flatpak

Profil de données typique :

```text
~/.var/app/org.kde.basket/data/basket
```

Configuration :

```text
~/.var/app/org.kde.basket/config/basketrc
```

Le profil est considéré comme valide lorsque le fichier suivant existe :

```text
baskets/baskets.xml
```

### BasKet natif

Sous une configuration XDG Linux classique, le profil natif correspond à :

```text
~/.local/share/basket
```

et la configuration à :

```text
~/.config/basketrc
```

Les emplacements réels sont calculés avec `QStandardPaths`, donc ils peuvent différer si les variables XDG de l'utilisateur sont personnalisées.

## Choix entre plusieurs profils

Lorsque le profil Flatpak et le profil natif existent tous les deux, Mathom compare la date de modification de :

```text
baskets/baskets.xml
```

Le profil contenant le fichier le plus récemment modifié est choisi comme source.

## Destination

Les données sont copiées dans le répertoire XDG de données Mathom, typiquement :

```text
~/.local/share/mathom
```

La configuration est copiée vers :

```text
~/.config/mathomrc
```

si une configuration source existe et qu'aucun `mathomrc` n'existe encore.

## Sécurité de la migration

Le profil BasKet source reste inchangé.

Si la copie récursive des données échoue, Mathom supprime le dossier Mathom partiellement créé afin d'éviter de conserver un profil incomplet.

## Compatibilité des arrière-plans

Mathom recherche prioritairement ses arrière-plans dans son propre espace `mathom/backgrounds`, mais conserve aussi la lecture des ressources historiques `basket/backgrounds`.

Cette compatibilité facilite la reprise d'un profil BasKet existant sans casser ses éléments visuels.
