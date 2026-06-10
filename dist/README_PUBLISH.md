# Remplir `youkorr/esphome-lvgl-kawaii`

Le contenu prêt est dans **`dist/esphome-lvgl-kawaii/`** (et une archive
`esphome-lvgl-kawaii.tar.gz` t'a été envoyée dans le chat). Le dépôt public
`youkorr/esphome-lvgl-kawaii` existe déjà (avec un README auto‑généré) — il
reste juste à y **pousser** ce contenu. `--force` est sûr ici (le dépôt vient
d'être créé).

## Depuis le téléphone — Codespaces (recommandé)

1. Ouvre **youkorr/lvgl_9.5**, branche `claude/jolly-lamport-ERZOC`
2. **Code → Codespaces → Create codespace**
3. Dans le terminal :

```bash
gh auth setup-git
cp -r dist/esphome-lvgl-kawaii ~/kawaii && cd ~/kawaii
git init -b main && git add -A && git commit -m "ESPHome LVGL Kawaii Face"
git remote add origin https://github.com/youkorr/esphome-lvgl-kawaii.git
git push -u --force origin main
```

## Sur un PC

```bash
git clone https://github.com/youkorr/lvgl_9.5 -b claude/jolly-lamport-ERZOC
cd lvgl_9.5/dist/esphome-lvgl-kawaii
git init -b main && git add -A && git commit -m "ESPHome LVGL Kawaii Face"
git remote add origin https://github.com/youkorr/esphome-lvgl-kawaii.git
git push -u --force origin main
```

## Depuis l'archive du chat

```bash
tar xzf esphome-lvgl-kawaii.tar.gz && cd esphome-lvgl-kawaii   # contient déjà un commit
git remote add origin https://github.com/youkorr/esphome-lvgl-kawaii.git
git push -u --force origin main
```

## Utilisation (dépôt public)

```yaml
external_components:
  - source: github://youkorr/esphome-lvgl-kawaii
    components: [lvgl_kawaii_face]
  - source:
      type: git
      url: https://github.com/youkorr/lvgl_9.5
      ref: claude/jolly-lamport-ERZOC
    components: [lvgl]
```
