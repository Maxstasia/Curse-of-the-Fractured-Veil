# 🎮 Setup & Build (SFML)

## Premier lancement (one-time)

### 1️⃣ Installer SFML

**Avec apt (le plus simple) :**
```bash
apt install -y libsfml-dev
```

**Ou si tu veux compiler depuis source :**
```bash
git clone https://github.com/SFML/SFML.git
cd SFML
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install  # ou juste utiliser localement
```

### 2️⃣ Compiler le jeu

```bash
cd /home/mstasiak/Downloads/Curse-of-the-Fractured-Veil
make
```

### 3️⃣ Lancer

```bash
make run
# ou simplement
./game
```

---

## Après le premier setup

Juste utilise :
```bash
make          # Recompile si changements
make run      # Compile + lance
make clean    # Nettoie les .o
```

---

## Troubleshooting

### ❌ "SFML/Graphics.hpp: No such file"
```bash
apt install -y libsfml-dev
# ou compile SFML depuis source (voir section 1)
```

### ❌ "undefined reference to `sfml_graphics`"
→ SFML pas installé correctement. Réessaie l'installation apt.

### ❌ La compilation marche pas
```bash
# Debug
g++ -std=c++17 -c src/game.cpp $(pkg-config --cflags sfml-graphics)
# Ça va te montrer l'erreur exacte
```

---

## Notes de développement

- **SFML 2.5+** : Graphics/Window/System
- **Architecture** : Structs Player/Entity + Game state machine
- **Rendering** : sf::RenderWindow avec shapes
- **Prochaine étape** (Phase 1) : Salles procédurales + meilleur entity manager

