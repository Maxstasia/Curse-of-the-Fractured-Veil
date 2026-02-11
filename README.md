### Malédiction du Voile Fracturé  
**(Curse of the Fractured Veil)**

Un roguelite top-down action codé en C/C++ pur, inspiré de *The Binding of Isaac* et *Hades*.

Projet solo hardcore pour progresser en profondeur : gestion manuelle de la mémoire, ECS léger/custom, rendu 2D avec SFML, maths vectorielles from scratch, salles procédurales, synergies d’items, meta-progression, etc.

Objectif : un dungeon crawler nerveux run-based avec  
- combat orienté dash (style Hades)  
- système d’items combinatoires explosifs (style Isaac)  
- mort → upgrades permanents meta  
- 100 % sans moteur de jeu (C++17/20)

**Pourquoi ce projet ?**  
Maîtriser en profondeur : allocations dynamiques, object pools, partitionnement spatial (quadtree ou grid simple), optimisation 60 fps, state machines, sérialisation basique pour les saves meta, etc.

### État actuel (février 2026)
- [x] Setup build (Makefile + SFML)  
- [x] Fenêtre + input de base (clavier)  
- [x] Joueur : déplacement + dash + attaque de base  
- [x] Système entités ultra simple (struct + arrays)  
- [x] Collisions AABB  
- [ ] Premières salles statiques  
- [ ] Mort + restart du run  
- [ ] ...

### Roadmap (étapes progressives – MVP → cool → ambitieux)

#### Phase 0 – Setup & boucle principale ✅ (DONE)
1. ✅ Initialisation projet : Makefile + main + .gitignore  
2. ✅ Choix lib : SFML (simple et rapide)  
3. ✅ Boucle game : 60 fps fixe + delta time  
4. ✅ Input : WASD + dash (space)  
5. ✅ Joueur : position, vélocité, sprite basique (cercle)  
6. ✅ États du jeu : MENU → RUNNING → GAME_OVER → RESTART

#### Phase 1 – Combat & salles (3–6 semaines)
7. Système entités : pool fixe ou dynamique (struct Entity { … })  
8. Ennemis basiques (se dirigent vers le joueur, contact = dégâts)  
9. Attaque : auto ou clic (projectiles / hitscan)  
10. Collisions : AABB + résolution simple  
11. 4–5 salles faites à la main et connectées (via Tilemap)  
12. Ouverture porte suivante quand salle vidée

#### Phase 2 – Items & synergies (le vrai fun)
13. Système Item : struct Item { id, nom, callback d’effet }  
14. Ramassage → application effet (lambda ou pointeur de fonction)  
15. 10–15 items de base : +vitesse, +dégâts, tir triple, poison, explosion à l’impact, dash pierce…  
16. Quelques combos scriptés (ex : item A + B = nouveau comportement)

#### Phase 3 – Meta-progression (style Hades)
17. Monnaie persistante après mort  
18. Hub / meta-boutique : vie max, dégâts de base, cooldown dash, items de départ…  
19. Sauvegarde/chargement simple (JSON ou format binaire custom)

#### Phase 4 – Polish & procédural (si motivé)
20. Génération de salles (BSP, cellular automata, prefabs random)  
21. Sons (miniaudio ou raylib audio)  
22. Particules basiques  
23. Animations spritesheet  
24. Boss fight simple  
25. Équilibrage & facteur fun

### Build & Lancement
```bash
git clone ...
cd maledition-du-voile-fracture  # ou curse-of-the-fractured-veil
mkdir build && cd build
cmake ..
make -j
./jeu    # ou ./game
```

## Ressources & Assets tiers

Tous les assets graphiques proviennent de packs gratuits itch.io (utilisés dans un projet non-commercial / d’apprentissage) :

- **Dungeon AssetPuck** par Pixel_Poem  
  https://pixel-poem.itch.io/dungeon-assetpuck  
  (crédit apprécié mais non obligatoire)

- **Tiny RPG Character Asset Pack** par Zerie  
  https://zerie.itch.io/tiny-rpg-character-asset-pack

- **UI User Interface MEGA PACK** par ToffeeCraft  
  https://toffeecraft.itch.io/ui-user-interface-mega-pack  
  (licence gratuite : usage personnel uniquement)

- **Tiny Swords** par Pixel Frog  
  https://pixelfrog-assets.itch.io/tiny-swords  
  (crédit apprécié mais non obligatoire)

Tous les assets sont utilisés conformément aux conditions de licence indiquées sur itch.io : interdiction de redistribution / revente, modifications autorisées pour projets personnels.  
Un grand merci à ces talentueux pixel artists pour avoir rendu ces ressources gratuites disponibles !

---

## English version

### Curse of the Fractured Veil

A top-down action roguelite coded in pure C/C++, inspired by *The Binding of Isaac* and *Hades*.

Solo hardcore learning project: manual memory management, lightweight/custom ECS, 2D rendering with Raylib or SDL2, vector math from scratch, procedural rooms, item synergies, meta-progression, etc.

Goal: a fast-paced run-based dungeon crawler with  
- dash-oriented combat (Hades style)  
- explosive combinatory item system (Isaac style)  
- death → permanent meta-upgrades  
- 100% no game engine (C99 / C++17/20)

### Current status (February 2026)
- [ ] Build setup (Makefile + CMake)  
- [ ] Window + basic input (keyboard + mouse)  
- [ ] Player: move + dash + basic attack  
- [ ] Simple entity system (struct + arrays)  
- [ ] AABB collisions  
- [ ] First static rooms  
- [ ] Death + run restart  
- [ ] ...

### Roadmap

#### Phase 0 – Setup & Core loop (1–2 weeks)
1. Project init: CMake + main + .gitignore  
2. Lib choice: Raylib (fast prototyping) or SDL2 + custom GL  
3. Game loop: fixed 60 fps + delta time  
4. Input: WASD + mouse + space dash  
5. Player: pos, vel, basic sprite (colored rect at first)  
6. Game states: MENU → RUNNING → GAME_OVER → RESTART

#### Phase 1 – Baston & salles (3–6 semaines)
7. Système d’entités : pool fixe ou dynamic, genre un gros tableau de `struct Entity { … }`  
8. Ennemis de base : ils foncent direct sur ta tronche, tu les touches = bam, dégâts  
9. Attaque : soit full auto (comme Isaac), soit clic souris (projectiles ou hitscan)  
10. Collisions : AABB bien propre + petite résolution à l’arrache (pas de physique de ouf)  
11. 4–5 salles codées à la main et chainées (un array de Tilemap ou un truc simple)  
12. Salle clean → porte qui s’ouvre, next

#### Phase 2 – Items & combos de ouf (là ça devient chaud)
13. Système Item : `struct Item { id, nom, void (*effect)(Player*) }` ou lambda si tu kiffes C++  
14. Tu ramasses → boom, l’effet s’applique direct  
15. 10–15 items de base : +speed, +dmg, triple shot, poison, explode on hit, dash qui traverse, etc.  
16. Quelques combos codés en dur (genre si t’as A + B → nouveau délire chelou)

#### Phase 3 – Meta-progression à la Hades
17. Thune qui reste après ta mort (genre des gemmes ou des âmes)  
18. Hub / boutique meta : tu up ta vie max, tes dégats de base, cooldown dash, unlock des starters…  
19. Save/load rapide (un petit JSON ou un fichier binaire maison)

#### Phase 4 – Polish & procédural (si t’es encore chaud)
20. Génération de salles (BSP, cellular automata, ou juste des prefabs randomisés)  
21. Bruitages (miniaudio ou le son de Raylib, simple et efficace)  
22. Particules de ouf (genre quand tu prends un dégât ou que tu exploses un truc)  
23. Animations sur spritesheet (walk, attack, hit, death…)  
24. Un boss de base (pas trop con au début)  
25. Équilibrage + feeling global (le truc qui fait que tu relances « juste une run »)

### Build & Lancement (vite fait)
```bash
git clone ton_repo
cd maledition-du-voile-fracture   # ou curse-of-the-fractured-veil si t’as gardé l’anglais
mkdir build && cd build
cmake ..
make -j$(nproc)
./jeu
```

## Assets & Third-party resources

All graphical assets are from free itch.io packs (used in a non-commercial / learning project):

- **Dungeon AssetPuck** by Pixel_Poem  
  https://pixel-poem.itch.io/dungeon-assetpuck  
  (Credit appreciated – not required)

- **Tiny RPG Character Asset Pack** by Zerie  
  https://zerie.itch.io/tiny-rpg-character-asset-pack

- **UI User Interface MEGA PACK** by ToffeeCraft  
  https://toffeecraft.itch.io/ui-user-interface-mega-pack  
  (Free license: personal use)

- **Tiny Swords** by Pixel Frog  
  https://pixelfrog-assets.itch.io/tiny-swords  
  (Credit appreciated – not required)

All assets used according to their respective itch.io license terms: no redistribution/resale, modifications allowed for personal projects.  
Big thanks to these talented pixel artists for making free resources available!
