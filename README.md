# 🔐 PWMan - Password Manager en C

Un gestionnaire de mots de passe minimaliste développé entièrement en C pour Linux x86_64, utilisant uniquement des appels système sans dépendances aux bibliothèques standard.

## 🎯 Caractéristiques

- **Sans bibliothèques standard** : Compilé avec `-nostdlib -fno-builtin`
- **Interface en ligne de commande** simple et intuitive
- **Chiffrement XOR** pour sécuriser les données stockées
- **Gestion de coffre-fort** avec mot de passe maître
- **Implémentation libc personnalisée** réutilisée du projet 0x41sh
- **Appels système directs** pour toutes les opérations

## 📋 Prérequis

- Linux x86_64
- GCC (GNU Compiler Collection)
- NASM (Netwide Assembler)

## 🚀 Installation

```bash
git clone https://github.com/kiperZZZ/pwman.git
cd pwman
make
```

## 💻 Utilisation

### Initialiser un nouveau coffre-fort
```bash
./pwman init vault.db
```

### Lister toutes les entrées
```bash
./pwman list vault.db
```

### Ajouter une nouvelle entrée
```bash
./pwman add vault.db
```

### Récupérer un mot de passe
```bash
./pwman get vault.db github.com
```

## 🏗️ Architecture

```
pwman/
├── src/
│   ├── main.c          # Programme principal et parsing CLI
│   ├── crypto.c        # Fonctions de chiffrement/déchiffrement
│   ├── database.c      # Opérations base de données (CRUD)
│   └── libc/           # Implémentation libc personnalisée
├── include/
│   ├── libc/           # Fichiers d'en-tête
│   └── pwman.h         # Définitions principales
├── crt0.asm           # Code de démarrage assembleur
└── Makefile           # Configuration de build
```

## 🔧 Fonctionnalités techniques

### Gestion mémoire
- `malloc()`, `free()`, `realloc()` - Allocation mémoire
- `sbrk()`, `brk()` - Gestion du tas

### Opérations I/O
- `read()`, `write()` - E/S fichier
- `printf()`, `puts()`, `putchar()` - Sortie
- `getline()` - Saisie de ligne

### Opérations chaînes
- `strcmp()`, `strncmp()` - Comparaison de chaînes
- `strlen()`, `strcpy()`, `strcat()` - Manipulation de chaînes

## 🔒 Sécurité

- **Pas de stockage en texte clair** : Tous les mots de passe sont chiffrés
- **Hachage du mot de passe maître** : Le mot de passe maître n'est jamais stocké en clair
- **Nettoyage mémoire** : Les données sensibles sont effacées après utilisation
- **Validation d'entrée** : Vérification des tailles de buffer pour éviter les débordements

## 🧪 Tests

```bash
# Test des fonctionnalités de base
make clean && make

# Test d'initialisation
./pwman init test_vault.db

# Test d'ajout d'entrées
./pwman add test_vault.db

# Test de listage
./pwman list test_vault.db

# Test de récupération
./pwman get test_vault.db example.com
```

## 🎓 Objectif éducatif

Ce projet a été développé dans un cadre éducatif pour comprendre :
- La programmation système en C
- Les appels système Linux
- L'implémentation d'une libc personnalisée
- La compilation sans bibliothèques standard
- La gestion bas niveau de la mémoire

## ⚠️ Limitations

- **Plateforme spécifique** : Linux x86_64 uniquement
- **Chiffrement basique** : XOR simple (à des fins éducatives)
- **Pas de réseau** : Stockage local uniquement
- **Fonctionnalités limitées** : Gestionnaire minimaliste

## 🤝 Contribution

Les contributions sont les bienvenues ! N'hésitez pas à ouvrir une issue ou proposer une pull request.

## 📄 Licence

Ce projet est développé à des fins éducatives. Consultez le fichier LICENSE pour plus de détails.

---

**Note** : Ce gestionnaire de mots de passe utilise un chiffrement simple à des fins éducatives. Pour un usage en production, utilisez des solutions de chiffrement plus robustes.