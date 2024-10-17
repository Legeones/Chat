# Utiliser une image de base légère contenant GCC
FROM gcc:latest

# Créer un répertoire de travail
WORKDIR /app

# Copier le code source du client dans le conteneur
COPY client.c .

# Compiler le client
RUN gcc -o client client.c -lpthread

# Exécuter le client. Remplacer l'IP par l'IP réelle du serveur au moment de l'exécution
CMD ["./client", "172.17.0.1", "8080"]
