# Passation — PR LVGL « PPA image SRM » (pour une nouvelle session)

> Colle le contenu de la section **« PROMPT À COLLER »** (tout en bas) dans la
> nouvelle session. Le reste est le détail technique que la session lira ici.

---

## 0. Objectif

Ouvrir **une PR sur `lvgl/lvgl`** qui ajoute l'accélération **PPA image
scale / rotate / mirror (SRM)** pour l'**ESP32-P4** à la draw unit existante.
**La feature est DÉJÀ approuvée par un mainteneur LVGL** :
issue `lvgl/lvgl#10260` → réponse : *« Yes, it makes sense. Would you like to
open a PR? »* (label `🛠️ PR needed`).

## 1. Contexte (ce qui a déjà été fait)

- Première session : a remonté le composant caméra `esp_video_camera` vers
  ESPHome — PR `esphome/esphome#16944` + doc `esphome/esphome.io#6787`. (En revue.)
- Le PPA : le composant `lvgl` d'ESPHome a déjà une draw unit PPA basique
  (par clydebarrow). clydebarrow **refuse** d'ajouter plus de code LVGL dans
  ESPHome (maintenance) → *« raise PRs on lvgl/lvgl »*. Donc **tout va dans LVGL.**
- LVGL master a déjà la draw unit PPA (PR #9162) dans `src/draw/espressif/ppa/`,
  **mais son chemin image est un STUB** (pas de scale/rotate/mirror).
- **youkorr a une implémentation SRM qui MARCHE** (en prod sur ESP32-P4,
  M5Tab5 / Waveshare P4) dans `youkorr/lvgl_9.5` →
  `components/lvgl/ppa/lv_draw_ppa_img.c` (fonctions `lv_draw_ppa_img_srm` +
  `lv_draw_ppa_img_rotate`).
- ⚠️ **Attribution** : `kyvaith` a forké `youkorr/lvgl_9.5` et copié ce code dans
  `esphome/esphome#16863` **sans crédit** (en-têtes verbatim « Fixed PPA… »).
  Sujet séparé, traité côté ESPHome. Le SRM, lui, n'est PAS dans #16863.

## 2. Ce que la nouvelle session doit faire

Créer la PR LVGL du SRM. **3 fichiers** à modifier dans
`src/draw/espressif/ppa/` d'un **fork de `lvgl/lvgl`**.

### Prérequis d'accès (IMPORTANT)
La session a besoin d'un **accès EN ÉCRITURE à un fork de `lvgl/lvgl`**
(ex. `youkorr/lvgl`). → **Forker `lvgl/lvgl` d'abord**, puis ajouter
`youkorr/lvgl` au périmètre de la session. (Le push de la PR ne peut PAS se
faire depuis `youkorr/lvgl_9.5` — pas d'historique commun avec `lvgl/lvgl`.)

### Les 3 fichiers (dans `src/draw/espressif/ppa/`)

**1) `lv_draw_ppa_img.c`** — ajouter les 2 fonctions
- Reprendre `lv_draw_ppa_img_srm()` + `lv_draw_ppa_img_rotate()` depuis
  `youkorr/lvgl_9.5/components/lvgl/ppa/lv_draw_ppa_img.c` (≈ lignes 119–413),
  sous `#if LV_USE_PPA_IMG`.
- **RETIRER l'enrobage ESPHome** : `#include "sdkconfig.h"` et
  `#ifdef CONFIG_SOC_PPA_SUPPORTED`. Dans LVGL, le fichier est gardé par
  `#if LV_USE_PPA` (déjà présent dans le stub).
- Le helper `lv_color_format_to_ppa_srm` **existe déjà** dans le `private.h` de
  LVGL → ne pas le redéfinir.

**2) `lv_draw_ppa.h`** — déclarer
```c
#if LV_USE_PPA_IMG
void lv_draw_ppa_img_srm(lv_draw_task_t * t, const lv_draw_image_dsc_t * dsc, const lv_area_t * coords);
void lv_draw_ppa_img_rotate(lv_draw_task_t * t, const lv_draw_image_dsc_t * dsc, const lv_area_t * coords);
#endif
```

**3) `lv_draw_ppa.c`** — router les images transformées (LE changement clé)
- Dans l'**évaluation** (`case LV_DRAW_TASK_TYPE_IMAGE`, sous `#if LV_USE_PPA_IMG`,
  ≈ ligne 109) : la condition exige aujourd'hui
  `scale_x==256 && scale_y==256 && rotation==0` → **l'assouplir** pour accepter
  aussi les cas que le SRM gère (scale ≠ 256, rotation ∈ {900,1800,2700}, RGB565/888).
- Dans le **dispatch de dessin** (`case LV_DRAW_TASK_TYPE_IMAGE`, ≈ ligne 192) :
```c
case LV_DRAW_TASK_TYPE_IMAGE: {
    lv_draw_image_dsc_t * dsc = (lv_draw_image_dsc_t *)t->draw_dsc;
    if(dsc->rotation != 0)
        lv_draw_ppa_img_rotate(t, dsc, &area);
    else if(dsc->scale_x != 256 || dsc->scale_y != 256)
        lv_draw_ppa_img_srm(t, dsc, &area);
    else
        lv_draw_ppa_img(t, dsc, &area);
    lv_draw_buf_invalidate_cache(buf, &area);
    break;
}
```
(La condition exacte d'évaluation = la logique testée de youkorr ; à aligner sur
ce que le SRM supporte vraiment.)

### Faits vérifiés sur LVGL master (baseline)
- `lv_draw_ppa_img.c` = stub 114 lignes (`lv_draw_ppa_img` + `lv_draw_img_ppa_core`,
  blend basique uniquement, pas de SRM).
- `lv_draw_ppa.c` : éval IMAGE ≈ ligne 109 (sous `#if LV_USE_PPA_IMG`) ; dispatch
  dessin ≈ ligne 192 appelle `lv_draw_ppa_img`.
- `lv_draw_ppa_private.h` a DÉJÀ : `LV_USE_PPA_IMG`, `lv_color_format_to_ppa_srm`
  (≈ ligne 143), `PPA_ALIGN_UP`.
- `#9868` (fill buffer align) est DÉJÀ corrigé dans LVGL master → ne pas le re-PR.

### Étapes
1. **Fork `lvgl/lvgl`** → `youkorr/lvgl` ; ajouter au périmètre de la session.
2. Branche depuis `master` (ex. `ppa-image-srm`).
3. Lire `youkorr/lvgl_9.5` `components/lvgl/ppa/lv_draw_ppa_img.c`, extraire les
   fonctions SRM, produire le `lv_draw_ppa_img.c` LVGL nettoyé.
4. Modifier `lv_draw_ppa.h` + `lv_draw_ppa.c` comme ci-dessus.
5. Commit avec **sign-off** (`git commit -s` — LVGL exige le DCO), push sur `youkorr/lvgl`.
6. **L'utilisateur ouvre la PR** `youkorr/lvgl` → `lvgl/lvgl:master`, référence
   l'issue #10260, ping le mainteneur.

### ⚠️ Limites à assumer
- **Pas de compilation/test possible sans ESP32-P4** (l'utilisateur est en
  déplacement ~2 semaines). → Ouvrir la PR en **DRAFT**, avec la note :
  *« proven downstream; verifying the LVGL-integrated build on-device when I'm
  back (~2 weeks) »*.
- Respecter `CONTRIBUTING.rst` + `CODING_STYLE.rst` de LVGL, le **DCO sign-off**,
  et ne toucher à aucun fichier non lié.

### Liens de référence
- Feature approuvée : https://github.com/lvgl/lvgl/issues/10260
- Source (impl qui marche) : https://github.com/youkorr/lvgl_9.5/tree/main/components/lvgl/ppa
- Draw unit LVGL : `src/draw/espressif/ppa/` (dans `lvgl/lvgl`)
- ESPHome (copie, attribution) : https://github.com/esphome/esphome/pull/16863
- Caméra (contexte) : https://github.com/esphome/esphome/pull/16944

## 3. Actions mobiles en attente (rappel)
- Répondre au mainteneur sur #10260 (« I'm travelling ~2 weeks, will open the PR
  on return and verify on hardware »).
- Poster le commentaire d'attribution sur `esphome/esphome#16863`.

---

## PROMPT À COLLER dans la nouvelle session

```
Je veux ouvrir une PR sur lvgl/lvgl qui ajoute le PPA image scale/rotate/mirror
(SRM) pour ESP32-P4. La feature est déjà approuvée : lvgl/lvgl#10260.

Lis d'abord le fichier LVGL_SRM_PR_HANDOFF.md sur la branche main de
youkorr/lvgl_9.5 — il contient tout le détail technique (les 3 fichiers à
modifier dans src/draw/espressif/ppa/, le code source qui marche dans
youkorr/lvgl_9.5/components/lvgl/ppa/, et les points d'intégration).

Périmètre nécessaire : accès en écriture à mon fork youkorr/lvgl (je vais forker
lvgl/lvgl). Prépare les 3 fichiers nettoyés (sans les bouts ESPHome
sdkconfig.h / CONFIG_SOC_PPA_SUPPORTED, sous #if LV_USE_PPA), commite-les sur une
branche de youkorr/lvgl avec sign-off (git commit -s), et donne-moi le lien
« Create pull request » vers lvgl/lvgl:master. Je n'ai pas ma carte ESP32-P4
pendant ~2 semaines : la PR sera ouverte en DRAFT, non testée sur hardware
(le code tourne déjà en prod en aval), à valider à mon retour.
```
