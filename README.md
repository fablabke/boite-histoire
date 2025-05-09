![Une boite à histoire open hardware](./photos/boite-a-histoire.jpg)

## Boite à histoire

Une boite à histoire complètement open source et open hardware, un projet du Fablab'ke.

Il existe sur le marché une série de “boîtes à histoires” qui permettent aux enfants d’écouter des histoires en audio. C’est un système qui fonctionne très bien et permet d’éviter les écrans tout en découvrant des univers assez variés. Écouter des histoires permet aussi de développer le langage et l’imaginaire.

Il y a par exemple la [boîte Lunii](https://lunii.com/fr-be/) et [pleins d’autres modèles](https://www.madmoizelle.com/quelles-sont-les-meilleures-boites-a-histoires-pour-les-enfants-1392363)

L’inconvénient majeur de toutes ces boîtes est le coût élevé de l’appareil (entre 70 et 100 euros) et le coût des packs d’histoires (une dizaine d’euros à chaque fois que l’on veut ajouter quelques histoires sur sa boîte sans pouvoir les partager avec d’autres).

Un autre inconvénient est que les bibliothèques d’histoires sont spécifiques à chaque fabricant, avec un contenu souvent assez lisse et peu multiculturel.

Nous avons décidé au Fablab'ke de développer et de fabriquer “notre” boîte à histoire maison, de créer en partenariat avec les autres cellules de la maison des cultures plusieurs projets autour de celle-ci, et d’en faire bénéficier les familles de la maison des cultures et au-delà.

Cette boite à histoire permet d'y installer autant d'histoire qu'on le souhaite, sans devoir passer par un "app-store" et permet du coup d'inventer et de se partager des histoires entre participant.e.s.

Une première série de 30 boites à histoire a été construite avec les classes d'informatique de Campus Saint Jean.

Ce projet est en cours de conception. [Contactez le fablab'ke](https://fablabke.be) pour plus d'informations.

![](./guides/assets/resultat.jpg)

## Mais comment fabriquer sa boite à histoire?

Lisez les documents suivants afin de comprendre les différentes étapes de fabrication et comment fabriquer la vôtre : 

- Principe de fonctionnement (todo)
- Carte électronique chez Aisler (todo)
- [Assemblage de la carte électronique](./guides/pcb.md)
- Code source arduino, facilement compilable avec platformio ou l'ide arduino : [/dossier arduino](./arduino)
- Code circuitpython (todo)

## Liste des composants nécessaires
- une plaquette d'expérimentation ou le circuit imprimé que l'on peut commander en ligne chez aisler.
- carte raspberry pi pico
- carte ampli i2s adafruit https://www.adafruit.com/product/3006 (clone : https://www.aliexpress.com/item/1005005985389931.html )
- carte chargeur (si vous voulez connecter une batterie) https://www.aliexpress.com/item/1005005982385924.html
- carte breakout sd adafruit : https://www.adafruit.com/product/4682
- potentiometre à souder trough hole 10k (par exemple : https://www.mouser.fr/ProductDetail/Same-Sky/PTN16-A10220K1B1?qs=VJzv269c%252BPbZP2voFoR5Tw%3D%3D )
- prise jack pour casque de ce type : https://www.mouser.be/ProductDetail/Same-Sky/SJ1-3525NG-GR?qs=WyjlAZoYn51BeLQaUbXH3w%3D%3D
- 3 résistances de 100k

