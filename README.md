# Curse of the Fractured Veil

**Un roguelite top-down action en C/C++ inspiré de The Binding of Isaac et Hades.**

Solo project pour monter en compétence hardcore : gestion manuelle de mémoire, ECS light/custom, rendu 2D avec [Raylib ou SDL2], maths vectorielles from scratch, procedural rooms, item synergies, meta-progression, etc.

Objectif : un run-based dungeon crawler nerveux avec :
- Combat dash-oriented (Hades style)
- Système d’items combinatoires explosifs (Isaac style)
- Mort → meta-upgrades permanents
- Tout en C99/C++17/20 sans engine

**Pourquoi ce projet ?**  
Apprendre en profondeur : allocation dynamique, pools d’objets, spatial partitioning (quadtree ou simple grid), optimisation 60 fps, state machines, sérialisation basique pour les saves meta, etc.

### État actuel (février 2026)
- [ ] Setup build (Makefile + CMake)
- [ ] Fenêtre + input basique (clavier + souris)
- [ ] Player : move + dash + attaque de base
- [ ] Système entités ultra simple (struct + arrays)
- [ ] Collisions AABB
- [ ] Premières salles statiques
- [ ] Mort + restart run
- [ ] ...

### Roadmap (étapes progressives – MVP → cool → fou)

#### Phase 0 – Setup & Core loop (1-2 semaines)
1. Init projet : CMakeLists.txt + main.c/cpp + .gitignore
2. Choisir lib : Raylib (le + rapide pour proto) ou SDL2 + OpenGL custom
3. Boucle game : 60 fps fixe, delta time
4. Input : WASD + souris + space dash
5. Player : position, velocity, sprite basique (rectangle coloré au début)
6. Game states : MENU → RUNNING → GAME_OVER → RESTART

#### Phase 1 – Combat & Rooms (3-6 semaines)
7. Système entités : array fixe ou dynamic pool (struct Entity { ... })
8. Ennemis simples (marchent vers toi, te touchent → dégâts)
9. Attaque auto ou clic (projectiles / hitscan)
10. Collisions : AABB → résolution basique
11. 4-5 salles hand-made connectées (array de Tilemap)
12. Porte suivante quand salle clear

#### Phase 2 – Items & Synergies (le fun commence)
13. Système Item : struct Item { id, name, effect callback }
14. Pick-up → apply effect (lambda ou function pointer)
15. 10-15 items de base : +vitesse, +dmg, tir triple, poison, explode on hit, dash pierce...
16. Quelques combos scriptés (ex : item A + B = nouveau comportement)

#### Phase 3 – Meta-progression Hades-like
17. Currency persistante (après mort)
18. Hub/Meta-shop : upgrader vie max, dmg base, dash cooldown, unlock starting items...
19. Save/load simple (JSON ou binaire custom)

#### Phase 4 – Polish & Procedural (si motivé)
20. Génération salles (BSP, cellular automata, prefabs random)
21. Sons (SFX via miniaudio ou raylib audio)
22. Particules basiques
23. Animations spritesheet
24. Boss fight simple
25. Équilibrage + fun factor

### Build & Run

```bash
# (exemple avec Raylib)
git clone ...
cd curse-of-the-fractured-veil
mkdir build && cd build
cmake ..
make -j
./game
```
