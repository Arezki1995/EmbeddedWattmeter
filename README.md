# EmbeddedWattmeter
EECIP wattmeter software V.1.0

L’objectif in fine de ce projet est d’avoir un module embarqué multi-canal capable de fournir des mesures relativement précises de la consommation énergétique d’une plateforme IoT. Les implémentations classiques se basent généralement sur la mesure de la valeur moyenne des grandeures masquant ainsi un aspect dynamique important et ne permettant pas de voir les puissances dissipées à des fréquences relativement plus importantes (au-delà de 10KHz) dues à l’impact du software et des communications qui sont des événements souvent intense et courts. 
Le module devra donc se greffer sur un noeud du projet Qarpediem pour y faire les mesures. On s'affranchit pour ce prototype de l’optimisation énergétique du module en lui-même car le but est de vérifier la valeur des informations qui peuvent être relevées à partir du noeud.

## How to use
 1- Entrez dans le repertoire 'src' puis faire un 'make'. La compilation genère les binaires dans le repértoire 'bin'.
 
 2- Pour lancer l'API d'acquisition lancez l'executable 'AcquisitionAPI' et laissez tourner.
 
 3- Actuellement vous disposer de plusieures choix d'interface:
 
    -L'executable 'Interface' pour lancer des acquisitions en ligne de commande. 
    
    -L'interface Graphique 'GUI.sh'.
    
    -Votre propre programme en implementant la librairie 'IPC' développée plus bas.

Vous trouverez plus d'informations plus bas.

## Architecture logicielle
Contraintes matérielles mises à part, le software doit permettre une certaine flexibilité. Il doit fournir une interface d’acquisition pour de futurs programmes dans le cadre du projet EECIP ainsi qu’une interface graphique simple pour pouvoir configurer, lancer et observer des acquisitions. Il doit aussi offrir la possibilité d'effectuer des sauvegardes locales de données où de les envoyer sur une base de données sur le réseau via Ethernet/Wifi.
Cette architecture repose essentiellement sur les Files de Messages afin gérer les communication inter-processus. L’objectif est d’avoir une approche modulaire du développement.
## Composantes majeures:
L’implémentation repose sur les composantes suivantes:
### AcquisitionAPI
C’est le programme principal en charge des acquisitions. Il utilise plusieurs librairies développées dans le cadre de ce projet:
#### IPC
pour la gestion des files de messages et de la communication avec les applications clientes externes et le programme qui permet de dessiner les courbes.
#### USB
pour le traitement de la lecture de données à partir du port USB Natif de la DUE.
#### PINCTRL
pour la gestion du bus de communication avec la DUE et la transfert  des configurations.
#### SEN0291
pour la gestion des capteurs de tension (SEN0291) en I2C.
#### NETWORK
pour le traitement de l’envoi des données via le réseau.

Le programme peut ếtre lancé en arrière plan en lui indiquant le périphérique d’acquisition:

 ./AcquisitionAPI  /dev/ttyACM2
 Ou:
 ./AcquisitionAPI

Dans ce dernier cas ci, il prendra le périphérique /dev/ttyACM0 par défaut. En cas d’erreur le programme rentre dans une boucle où il teste /dev/ttyACM0 et /dev/ttyACM1 toutes les 4 secondes jusqu’à détection.

Une fois cette étape passée, il ouvre Deux files de messages:
-La file de configuration: pour interagir avec des applications externes qui souhaitent faire des acquisitions.
-La file de graphe: pour dessiner des courbes d’acquisition quand on en lui fait la requête.

Enfin, il affiche sa configuration actuelle (par défaut ) puis se met en écoute de requêtes. Il est maintenant prêt à recevoir et exécuter les demandes.


 ################################ CONFIGURATION ##############################
  
    ACQUISITION:
        device           :     /dev/ttyACM0
        Measure point    :     I0
        SamplingRate     :     666660 Sample/s
        NB of Blocks     :     1
        export option    :     CSV
        filename         :     data.csv

    NETWORK:
        server IP        :     127.0.0.1
        server port      :     9000

    MSG QUEUES:
        config key       :    1995
        grapher key      :    2019

    Listening For Requests...

Si une requête est reçue, le programme la prend en charge pour le traitement et un acknowledge est envoyé à l’application cliente sur la File de message de configuration. Le programme ne traite qu’une requête à la fois. Les requêtes reçues pendant un traitement seront traitées une fois que le programme soit revenu en mode écoute et ce dans l’ordre dans lequel elles lui sont soumises. Tant que l'application cliente n’a pas reçu d’acknowledge, sa requête n’a pas encore été traitée. 
Comme expliqué précédemment, nous avons deux files pour servir deux usages différents. Intéressons-nous d’abords à la file de configuration qui implémente le pilotage des acquisitions ainsi que les requêtes.
La structure adoptée pour ce message est  API_MSG.
Celle-ci regroupe plusieurs champs:
 -Type : identifiant du type de message.
 -APICommande : type de requête.
 -APIExport : format et manière d’exporter les résultats
 -Point : sélectionner le point de mesure.
 -SamplingRate : choisir la fréquence d’échantillonnage.
 -numberOfBlocks : nombre de blocs de données à acquérir.
 -fileName : dans le cas d’un export en CSV, le nom du fichier.
 -Host : dans le cas d’un export par réseau, l’adresse du serveur.
 -Port : dans le cas d’un export par réseau, le port sur le serveur.

Une application peut utiliser la librairie IPC pour formuler des requêtes en initialisant cette structure.

Le champs Type permet de filtrer les messages dans la liste selon leurs sources. Les champs APICommand et APIExport sont importants car ils définissent quels champs seront pris en considération par le programme d’acquisition. Ce comportement est décrit dans le tableau ci-dessous.


Vous pouvez lancer une acquisition en mode API_ACQUIRE qui définit une fenêtre d'acquisition. Le programme s’arrête et se remet en écoute à la fin de celle-ci. La fenêtre temporelle se calcule comme suit:
 Temps = (numberOfBlocks) * 256 * (1Fréquence d'échantillonnage)) 
Si vous utilisez le mode API_FREERUN, Le programme se met à faire des acquisitions non-stop mais vérifie sa boîte aux lettres tous les numberOfBlocks , en non-bloquant, pour  un message contenant la commande API_STOP.  Une fois qu'il recevra ce message, il s’arrête et se remet en écoute.
Il faut savoir que les deux commandes API_ACQUIRE et API_FREERUN lancent les acquisitions avec la configuration actuelle de l’Acquisition API  et ne permettent pas de modifier la configuration. Pour mettre en place une configuration on doit impérativement passer par la requête API_SET_CONFIG. 


### Grapher
Grapher est un petit program dont le rôle est de permettre la visualisation d’une acquisition. Il est basé sur le program Gnuplot qui est un programme de tracé de fonctions et de données en ligne de commande. L’approche est la suivante:
Grapher est lancé par le programme d’acquisition comme processus fils. Ce dernier ouvre un pipe vers le process Gnuplot en utilisant la fonction “popen”. Grâce à ça, nous pouvons exécuter  un script de tracé (codé en dur) sur les données d’acquisition qui ont été produites, au préalable, par l’API d’acquisition. Une fois le graphe produit, le processus Grapher termine mais le graphe généré persiste dans une fenêtre X11.  
La structure représentant ce type de requêtes est GRAPHER_MSG.

### Application cliente:
L’application cliente peut-être n’importe quel programme pouvant utiliser les files de message. Dans le cadre de ce projet une application exemple a été créée pour servir d’interface.
Interface est un programme très simple (qui peut servir d’exemple) qui implémente la librairie IPC et permet de servir d’interface client en ligne de commande avec l’API. 

  Usage: ./interface -c \<COMMAND\> -e \<EXPORT\> -m <MSR_POINT> -s <SAMPLING_RATE> -b <NB_BLOCKS> -f <FILE_NAME> -h <SERVER_IP> -p <SERVER_PORT>
 
 
Les valeurs des arguments sont mappées sur les options décrites dans la figure XXX. Tous les champs doivent être initialisés même si certains, selon le cas, seront ignorés. 
Un script shell ''GUI.sh'' fait office d'interface graphique en utilisant le programme de création d’interface graphique Zenity. Ce dernier lance le programme interface une fois que les paramètres ont été entrés. Les champs non renseignés sont mis aux valeurs par défauts.  
