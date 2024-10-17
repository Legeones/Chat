# Utiliser une image de base légère contenant GCC
FROM gcc:latest

# Créer un répertoire de travail
WORKDIR /app

# Copier le code source du serveur dans le conteneur
COPY serveur.c .

# Compiler le serveur
RUN gcc -o serveur serveur.c -lpthread

# Exposer le port sur lequel le serveur écoutera
EXPOSE 8080

# Commande d'exécution par défaut
CMD ["./serveur", "8080"]