# Building LYRA


assicurati che gli script siano eseguibili
``` chmod 777 berkeleydb.sh boost.sh install.sh```

Controllare se è presente Berkeley DB e installarlo se necessario
``` ./berkeleydb.sh```

Controllare se è presente boost e installarlo se necessario
``` ./boost.sh```

Se è tutto a posto, eseguire install.sh fuori dalla cartella del progetto

``` cp install.sh .. && cd .. && ./install.sh```