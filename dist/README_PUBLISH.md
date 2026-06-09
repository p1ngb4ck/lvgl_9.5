# Publier `esphome-lvgl-kawaii-face`

Le dépôt complet et prêt à pousser est dans `dist/esphome-lvgl-kawaii-face/`
(et une archive `.tar.gz` t'a été envoyée dans le chat).

> Je n'ai pas pu créer le dépôt GitHub automatiquement : l'intégration GitHub
> de la session n'a pas la permission de créer des dépôts (`403 Resource not
> accessible by integration`) et le proxy git est limité à `lvgl_9.5`.
> Voici donc les étapes manuelles (≈ 1 min).

## Option A — avec le CLI `gh` (le plus rapide)

```bash
cd dist/esphome-lvgl-kawaii-face
git init -b main
git add -A
git commit -m "Initial commit: ESPHome LVGL Kawaii Face"
gh repo create youkorr/esphome-lvgl-kawaii-face --public --source=. --remote=origin --push
```

## Option B — sans `gh` (web + git)

1. Crée le dépôt vide sur GitHub : https://github.com/new
   - Owner: `youkorr` · Name: `esphome-lvgl-kawaii-face` · Public
   - **Ne coche pas** « Add a README » (le dépôt en contient déjà un).
2. Pousse le contenu :

```bash
cd dist/esphome-lvgl-kawaii-face
git init -b main
git add -A
git commit -m "Initial commit: ESPHome LVGL Kawaii Face"
git remote add origin git@github.com:youkorr/esphome-lvgl-kawaii-face.git
git push -u origin main
```

## Option C — depuis l'archive du chat

```bash
tar xzf esphome-lvgl-kawaii-face.tar.gz
cd esphome-lvgl-kawaii-face          # contient déjà un commit git
git remote add origin git@github.com:youkorr/esphome-lvgl-kawaii-face.git
git push -u origin main
```

## Après publication

Utilisable dans n'importe quelle config ESPHome :

```yaml
external_components:
  - source: github://youkorr/esphome-lvgl-kawaii-face
    components: [lvgl_kawaii_face]
  - source: github://youkorr/lvgl_9.5
    components: [lvgl]
```
