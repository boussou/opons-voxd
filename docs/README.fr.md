# opons-voxd

> Note: opons-voxd is unrelated to jakovius/voxd. The name was chosen independently — `opons` = O. Pons (author), `voxd` = vox + daemon (Latin/Unix tradition).

> **⚠ Rétrocompatibilité cassée volontairement.** Ce projet s'appelait précédemment `voice_in_linux`. Le renommage touche le binaire, le fichier source, toutes les variables d'environnement (`VOICE_IN_*` → `OPONS_VOXD_*`), et la dépendance runtime à `xdotool` (remplacée par XTest direct via `libxtst-dev`). Aucun alias, aucune compatibilité ascendante — mettez à jour vos scripts de lancement, vos entrées d'autostart et vos lignes `apt install`.

**Dictation vocale 100% locale pour Linux — votre voix ne quitte jamais votre machine.**

🌍 **Langues :**
[English](../README.md) · [Deutsch](README.de.md) · [中文](README.zh.md) · [日本語](README.ja.md) · [Español](README.es.md) · [Italiano](README.it.md)

---

## Pourquoi ?

Je voulais pouvoir dicter du texte dans n'importe quelle application sous Linux sans envoyer ma voix à un serveur tiers. Toutes les solutions existantes — extensions de navigateur, API cloud, plateformes SaaS — transmettent votre audio à des serveurs distants pour la transcription. Cela signifie que chaque mot prononcé transite par Internet : emails, documents confidentiels, données clients, notes personnelles.

Ce projet est une alternative légère et entièrement locale. Il tourne exclusivement sur votre machine grâce à [whisper.cpp](https://github.com/ggerganov/whisper.cpp), le modèle de reconnaissance vocale open source d'OpenAI. Aucune connexion réseau n'est nécessaire après l'installation initiale. Aucune donnée ne quitte votre ordinateur.

Réalisé avec l'aide de l'intelligence artificielle.

---

## Fonctionnement

1. Une icône apparaît dans la barre des tâches (system tray)
2. **Clic gauche** → l'enregistrement démarre (l'icône devient rouge)
3. **Clic gauche** à nouveau → arrêt, transcription locale via Whisper
4. Le texte transcrit est copié dans le presse-papier (les deux sélections X11)
5. Une notification affiche le texte pendant 10 secondes
6. Collez avec `Ctrl+Shift+V`, `Shift+Insert` ou clic milieu

**Clic droit** sur l'icône pour un menu Toggle / Quitter.

### Captures d'écran

| Inactif | Enregistrement |
|:---:|:---:|
| ![Inactif](../screenshots/opons_voxd_inactive.png) | ![Enregistrement](../screenshots/opons_voxd_active.png) |

---

## Fonctionnalités

- **100% local** — l'audio est traité sur votre CPU ou GPU, jamais envoyé
- **Accélération GPU** — support NVIDIA CUDA pour une transcription quasi-instantanée
- **99 langues** — propulsé par OpenAI Whisper
- **Léger** — binaire C unique (~100 Ko), pas de Python, pas de dépendances runtime
- **Intégration system tray** — icône discrète dans la barre des tâches
- **Double presse-papier** — texte copié dans PRIMARY et CLIPBOARD
- **Notifications bureau** — le texte transcrit est affiché en notification
- **Commandes vocales** — fichiers de commandes par langue dans `commands/`, désactivées par défaut (`OPONS_VOXD_COMMANDS=1`). Ajoutez une langue en créant `commands/xx.txt`, sans recompiler.
- **Notifications transitoires** — par défaut les notifications apparaissent puis disparaissent complètement. `OPONS_VOXD_NOTIFY_PERSIST=1` pour les garder dans l'historique.
- **Mode de notification** — `OPONS_VOXD_NOTIFY=normal|quiet|silent|off` contrôle les notifications de succès (défaut `quiet` : bulle sans son). Les erreurs s'affichent et sonnent toujours, dans tous les modes, sans exception. Voir le README principal pour le tableau complet.
- **Majuscules automatiques** — les phrases sont capitalisées automatiquement
- **Démarrage automatique** — configurable au login

---

## Prérequis

### Système d'exploitation

| Prérequis | Minimum | Vérification |
|---|---|---|
| Ubuntu / Linux Mint | 22.04 / 21 | `lsb_release -a` |
| Noyau Linux | 5.15+ | `uname -r` |
| Serveur d'affichage | X11 | `echo $XDG_SESSION_TYPE` |

### Compilation (obligatoire)

| Paquet | Minimum | Installation |
|---|---|---|
| gcc | 9.0+ | `sudo apt install build-essential` |
| cmake | 3.14+ | `sudo apt install cmake` |
| pkg-config | 0.29+ | `sudo apt install pkg-config` |
| git | 2.25+ | `sudo apt install git` |

### Bibliothèques de développement (obligatoire)

| Paquet | Minimum | Installation |
|---|---|---|
| libgtk-3-dev | 3.22+ | `sudo apt install libgtk-3-dev` |
| libnotify-dev | 0.7+ | `sudo apt install libnotify-dev` |
| libportaudio-dev | 19.6+ | `sudo apt install libportaudio-dev` |
| libcairo2-dev | 1.14+ | `sudo apt install libcairo2-dev` |
| libxtst-dev | 1.2+ | `sudo apt install libxtst-dev` |

### Outils runtime (obligatoire)

| Outil | Rôle | Installation |
|---|---|---|
| xclip | Presse-papiers X11 | `sudo apt install xclip` |
| notify-send | Notifications bureau | `sudo apt install libnotify-bin` |

### GPU NVIDIA (optionnel mais critique pour les performances)

Sans CUDA, comptez **10 à 15 secondes** de traitement pour 15 secondes de parole. Avec CUDA, comptez **moins d'1 seconde**. C'est un facteur **10 à 50x**.

| Composant | Minimum | Vérification |
|---|---|---|
| Driver NVIDIA | 570+ | `nvidia-smi` |
| CUDA Toolkit | 12.0+ | `nvcc --version` |

Si `nvidia-smi` fonctionne mais pas `nvcc`, installez le toolkit CUDA :

```bash
sudo apt install nvidia-cuda-toolkit
```

Si la version CUDA affichée par `nvidia-smi` est **inférieure** à celle de `nvcc`, mettez à jour le driver :

```bash
ubuntu-drivers devices
sudo apt install nvidia-driver-590
sudo reboot
```

---

## Installation

```bash
# 1. Dépendances système
sudo apt update
sudo apt install -y \
    build-essential cmake pkg-config git \
    libgtk-3-dev libnotify-dev libportaudio-dev libcairo2-dev libxtst-dev \
    xclip libnotify-bin

# 2. Cloner le projet
git clone https://github.com/olivierpons/opons-voxd.git
cd opons-voxd

# 3. Compiler whisper.cpp + télécharger le modèle
make setup

# 4. Compiler le binaire
make

# 5. Lancer
./opons-voxd
```

---

## Licence

Tout est sous licence MIT : le modèle Whisper (OpenAI), whisper.cpp (Georgi Gerganov), et ce projet.

Documentation complète : voir le [README principal](../README.md).

---

## Style de code

Ce projet suit le **style du noyau Linux** (`Documentation/process/coding-style.rst`) avec des adaptations par Olivier Pons : accolades K&R, indentation 4 espaces, `struct name` sans typedef, pas de préfixes (`s_`, `t_`, `e_`), `return value;` sans parenthèses, commentaires kernel-doc, 80 caractères par ligne maximum. Détails complets dans le [README principal](../README.md).

## Raccourci push-to-talk

Le fichier de configuration généré `~/.config/opons-voxd.conf` utilise `ctrl+alt+w` (recommandé sur AZERTY) ou `ctrl+alt+z` (recommandé sur QWERTY). La touche est juste à côté de `A`, donc tout le combo est atteignable avec la seule main gauche (auriculaire sur `Ctrl`, pouce sur `Alt`, annulaire/majeur sur `W`/`Z`) sans casser la posture de frappe, et n'est pris par aucun raccourci système par défaut sur la plupart des environnements de bureau. À configurer via `OPONS_VOXD_PTT_HOTKEY`. Détails complets dans le [README principal](../README.md#push-to-talk-hotkey).
