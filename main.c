/*
 * Descrizione progetto: questo progetto mette in pratica l'algoritmo di Dijkstra
 * per costruire una tabella di instradamento per router selezionati dall'utente.
 * Le connessioni tra router possono essere prese da file o inserite (e rimosse)
 * a mano dall'utente.
 * Ho voluto rendere questo programma abbastanza dinamico quindi si possono inserire
 * anche più file che rappresentino la topologia di rete, le connessioni possono
 * essere aggiunte e rimosse a piacere e si può vedere la tabella di ogni router
 * presente nella rete, per questo la tabella viene creata sul momento.
 *
 * La struttura del file deve essere la seguente:
 * Router-A;Router-B;costoLink // Rappresenta una connessione
 * Router-A;Router-C;costoLink // Rappresenta un'altra connessione
 *
 * Le connessioni possono essere inserite anche solo una volta, cioè basta mettere:
 * <A;B;3> non serve anche <B;A;3>
 *
 * Avvio: per far partire il programma avviarlo, inserire una scelta dal menu, prima di
 * tutto va creata almeno una connessione, e poi seguire le istruzioni.
 *
 * Problemi: il controllo dell'input non è ancora ottimale e se quando si vuole
 * inserire un nuovo file il programma non controlla se il file esiste nella cartella
 * perchè non so come controllarlo sia su Linux che su Windows.
 *
 * Autore: Gabriel Duta
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Link che connette due router */
struct connessione {
    char routerPartenza[15];
    char routerArrivo[15];
    int costoLink;
};
/* Linked list per le connessioni tra router */
struct Links {
    struct connessione connessione;
    struct Links *next;
};

/* Entry della tabella di instradamento */
struct entry {
    char nextHop[15];
    char destinazione[15];
    int costo;
};
/* Linked list per la tabella di instradamento*/
struct llTabella {
    struct entry riga;
    struct llTabella *next;
};

/* Linked list per gestire la priority queue */
struct priority {
    char nodo[15];
    int valore;
    struct priority *next;
};

/* Dichiarazione funzioni */
void aggiungiFile();
struct Links *inserisciConnessione(struct Links *l, struct connessione connessione);
struct llTabella *inizializzaTabella(struct llTabella *tabella, char start[15]);
struct priority *inserisciPriority(struct priority *p, char nodo[15], int valore);
struct priority *rimuoviPriority(struct priority *p, char nodo[15]);
struct llTabella *sostituisciTabella(struct llTabella *t, struct entry e);
int costoRouterPrecedente(struct llTabella *tabella, char nodo[10]);
bool containsPriority(struct priority *p, char nodo[10]);
bool containsTabella(struct llTabella *t, char nodo[10]);
bool contieneContrario(struct Links *l, struct connessione conn);
void mostraTabella();
int menu();
bool containsStart(char start[15]);
void aggiuntaConnessione();
bool rimuoviConnessione();
void mostraNodi();
const char * getNextHop(struct llTabella *t, char dest[15]);

/* Rapresentazione di un database con le connessioni di rete (topologia) */
struct Links *links;

int main(int argc, char** argv) {
    int scelta;
    struct Links *t;

    printf("Algoritmo di Dijkstra\nCon gestione dinamica della topologia di rete");
    do{
        scelta = menu();
        switch(scelta) {
            case 1: aggiungiFile();
                break;
            case 2: mostraNodi();
                break;
            case 3: mostraTabella();
                break;
            case 4: aggiuntaConnessione();
                break;
            case 5: rimuoviConnessione();
                break;
        }
    }while(scelta != 6);

    return (EXIT_SUCCESS);
}

/*
 * Funzione per stampare il menu
 *
 * @return scelta effettuata
 */
int menu() {
    int scelta;
    bool controllo = true; // Usato per controllare se la scelta fatta nel menu è valida

    printf("\n\nMenu:");
    printf("\n1) Aggiungi file che descrive la topologia della rete;");
    printf("\n2) Visualizza tutte le connessioni della rete;");
    printf("\n3) Visualizza tabella di un nodo;");
    printf("\n4) Aggiungi una nuova connessione alla rete;");
    printf("\n5) Rimuovi una connessione dalla rete;");
    printf("\n6) Esci.\nInserisci scelta: ");
    do{
        scanf("%d", &scelta);
        controllo = true;
        /* Controllo che il database delle connessioni non sia vuoto */
        if((scelta == 2 || scelta == 5) && links == NULL) {
            printf("Nessuna entry ancora presente, reinserire scelta: ");
            controllo = false;
        }
        else if(scelta < 1 || scelta > 6) {
            printf("Scelta non valida, reinserire: ");
            controllo = false;
        }
    }while(!controllo);
    printf("\n");

    return scelta;
}

void mostraNodi() {
  struct Links *stampaLinks = links;
  printf("Nodi:\n");
  while (stampaLinks != NULL) {
    printf("%s; ", stampaLinks->connessione.routerPartenza);
    printf("%s; ", stampaLinks->connessione.routerArrivo);
    printf("%d.\n", stampaLinks->connessione.costoLink);
    stampaLinks = stampaLinks->next;
  }
  printf("\nNextHop; destinazione; costo");
  printf("\n\nPremi invio per continuare...\n");
  getchar();
  getchar();
}

/*
 * Funzione che crea e stampa la tabella di un determinato nodo
 */
void mostraTabella() {
    struct llTabella *tabella = NULL;
    char start[15];
    bool controllo = true;

    /* Inserimento e controllo del nodo di partenza */
    printf("Inserisci nodo di partenza: ");
    do{
        scanf("%s", start);
        controllo = true;
        if(!containsStart(start)) {
            printf("Nodo non esistente, reinserire: ");
            controllo = false;
        }
    }while(!controllo);
    /* Riempimento tabella */
    tabella = inizializzaTabella(tabella, start);
    /* Stampa tabella */
    printf("\nTabella di %s:\n", start);
    while(tabella != NULL) {
        printf("%s; %s;", tabella->riga.nextHop, tabella->riga.destinazione);
        printf(" %d\n", tabella->riga.costo);
        tabella = tabella->next;
    }
    printf("\nNextHop; destinazione; costo");
    printf("\n\nPremi invio per continuare...\n");
    getchar();
    getchar();
}

/*
 * Funzione che permettere di aggiungere al database una connessione creata a mano
 * dall'utente.
 */
void aggiuntaConnessione() {
    struct connessione connessione;
    char temp[15];
    struct Links *tempLink = links;
    bool aggiungi = true;

    printf("Creazione di una nuova connessione: \n");
    /* Creazione dati link */
    printf("Inserisci nodo di partenza: ");
    scanf("%s", connessione.routerPartenza);
    printf("Inserisci nodo di arrivo: ");
    scanf("%s", connessione.routerArrivo);
    printf("Inserisci costo link: ");
    scanf("%d", &connessione.costoLink);
    /* Controllo che il link non sia già esistente*/
    while(tempLink != NULL) {
        if(strcmp(tempLink->connessione.routerPartenza, connessione.routerPartenza) == 0
            && strcmp(tempLink->connessione.routerArrivo, connessione.routerArrivo) == 0
            && tempLink->connessione.costoLink == connessione.costoLink) {
            aggiungi = false;
            printf("\nConnessione già esistente!\n");
            break;
        }
        tempLink = tempLink->next;
    }
    /* Inserimento del link e del suo contrario */
    if(aggiungi) {
        links = inserisciConnessione(links, connessione);
        strcpy(temp, connessione.routerPartenza);
        strcpy(connessione.routerPartenza, connessione.routerArrivo);
        strcpy(connessione.routerArrivo, temp);
        links = inserisciConnessione(links, connessione);
        printf("\nConnessione aggiunta con successo!\n");
    }
}

/*
 * Funzione che controlla che il nodo di partenza selezionato esista
 *
 * @param start nodo di partenze
 * @return true se il nodo esiste, false altrimenti
 */
bool containsStart(char start[15]) {
    struct Links *temp = links;

    while(temp != NULL) {
        if(strcmp(temp->connessione.routerPartenza, start) == 0)
            return true;
        temp = temp->next;
    }
    return false;
}

/*
 * Aggiunta dati alla Linked list che rappresenta il database delle connessioni.
 * I nuovi dati vengono presi da file in questo caso.
 * Non c'è nessun controllo per l'esistenza di un file.
 */
void aggiungiFile() {
    char buffer[34], dati[3][15];
    struct connessione connessione;
    int i, y, fine;
    FILE *fl;
    char nomeFile[30];
    char temp[15];

    /* Nessun controllo per l'esistenza di un file */
    printf("Inserisci nome file: ");
    scanf("%s", nomeFile);
    printf("\nAggiunta file....");
    fl = fopen(nomeFile, "r");
    /* Scorrimento file e inizializzazione lista dei link */
    while(fgets(buffer, 34, fl) != NULL) {
        y = 0;
        fine = 0;
        i = 0;
        while(buffer[i] != '\0') {
            if(buffer[i] != ';')
                dati[y][i - fine] = buffer[i];
            else {
                dati[y][i - fine] = '\0';
                y++;
                fine = i + 1;
            }
            i++;
        }
        strcpy(connessione.routerPartenza, dati[0]);
        strcpy(connessione.routerArrivo, dati[1]);
        connessione.costoLink = atoi(dati[2]);

        /* Controllo che la connessione non sia già esistente, necessario se
          vengono aggiunti 2 file che hanno anche connessioni comuni */
        if(!contieneContrario(links, connessione)) {
            links = inserisciConnessione(links, connessione);
            /* Le prossime righe permettono di salvare le connessioni in modo bilaterale
             (se esiste A;B;3 esiste anche B;A;3) in modo da far funzionare la creazione
             della tabella senza errori e inoltre permette di dimezzare la lunghezza dei
             file in quanto ogni connessione può essere scritta una sola volta */
            strcpy(temp, connessione.routerPartenza);
            strcpy(connessione.routerPartenza, connessione.routerArrivo);
            strcpy(connessione.routerArrivo, temp);
            links = inserisciConnessione(links, connessione);
        }
    }
    fclose(fl);
    printf("\nFile aggiunto con successo!");
}

/*
 * Funzione per inserire un nuovo elemento nella Linked list.
 * Gli elementi vengono direttamente messi in ordine alfabetico
 *
 * @param l Inizio della linked list
 * @param connessione Nuovo elemento da inserire nella lista
 * @return Inizio della nuova linked list
 */
struct Links *inserisciConnessione(struct Links *l, struct connessione connessione) {
    /* Puntatori d'appoggio */
    struct Links *pTemp, *testa = l;

    /* Se la lista passata non è vuota viene aggiunto il nuovo elemento */
    if(l != NULL) {
        /* Crazione elemento da inserire */
        pTemp = (struct Links *)malloc(sizeof(struct Links));
        pTemp->connessione = connessione;

        /* Modifica del primo elemento della lista */
        if(strcmp(pTemp->connessione.routerPartenza, l->connessione.routerPartenza) < 0) {
            pTemp->next = l;
            return(pTemp);
        }
        else {
            /* Scorrimento lista */
            while(l->next != NULL && strcmp(pTemp->connessione.routerPartenza, l->next->connessione.routerPartenza) > 0)
                l = l->next;
            /* Inserimento dell'elemento in mezzo o a fine lista */
            pTemp->next = l->next;
            l->next = pTemp;
            return(testa);
        }
    }
    /* Se la lista passata è vuota viene inizializzata */
    else {
         /* creazione primo elemento */
         l = (struct Links *)malloc(sizeof(struct Links));
         l->connessione = connessione;
         l->next = NULL;
         testa = l;
    }
    return(testa);
}

/*
 * Funzione che controlla se nella linked list delle connessioni è già presente
 * il contrario di una connessione (A; B) (B; A).
 * Serve per evitare di aggiungere connessioni inutilmente.
 *
 * @param l Linked list delle connessioni
 * @param conn connessione da controllare
 * @return true se la connessione contraria esiste, false altrimenti
 */
bool contieneContrario(struct Links *l, struct connessione conn) {
    while(l != NULL) {
        if(strcmp(l->connessione.routerPartenza, conn.routerArrivo) == 0
            && strcmp(l->connessione.routerArrivo, conn.routerPartenza) == 0)
            return true;
        l = l->next;
    }
    return false;
}

/*
 * Funzione cha inizializza la tabella di instradamento del router selezionato.
 *
 * @param tabella Inizio della tabella di instradamento vuota
 * @return Inizio della tabella di instradamento riempita
 */
struct llTabella *inizializzaTabella(struct llTabella *tabella, char start[15]) {
    struct llTabella *testa = tabella, *tTemp;
    struct priority *priorityQueue;
    struct Links *linksTemp = links;
    int costo;
    struct connessione tempLink;
    struct entry entryTemp;
    char nodoTemp[15];
    const char *nextHop;

    /* Inizializzazione del primo elemento della priority queue */
    priorityQueue = (struct priority *)malloc(sizeof(struct priority));
    strcpy(priorityQueue->nodo, start);
    priorityQueue->valore = 0;
    priorityQueue->next = NULL;

    /* Inserimento primo elemento nella tabella */
    tabella = (struct llTabella *)malloc(sizeof(struct llTabella));
    strcpy(tabella->riga.nextHop, start);
    strcpy(tabella->riga.destinazione, start);
    tabella->riga.costo = 0;
    tabella->next = NULL;
    testa = tabella;

    while(priorityQueue != NULL) {
        /* Questo while trova il nodo che bisogna analizzare nella linked list
           delle connessioni */
        while(linksTemp != NULL && strcmp(linksTemp->connessione.routerPartenza, priorityQueue->nodo) != 0)
            linksTemp = linksTemp->next;
        /* In questo while i nodi direttamente connessi a quello preso in considerazione
           vengono inseriti nella tabella o se già presenti vengono confrontati
           con il percorso già esistente */
        while(linksTemp != NULL && strcmp(linksTemp->connessione.routerPartenza, priorityQueue->nodo) == 0) {
            tempLink = linksTemp->connessione;
            /* Calcolo costo per raggiungere un nodo connesso al nodo
               preso in considerazione */
            costo = costoRouterPrecedente(testa, tempLink.routerPartenza);
            costo += tempLink.costoLink;
            /* Righe per trovare in next hop per la destinazione, se la destinazione
               è direttamente connessa al router di partenza allora la destinazione
               stessa viene inserita come next hop */
            if(strcmp(tempLink.routerPartenza, start) != 0)
                nextHop = getNextHop(testa,tempLink.routerPartenza);
            else
                nextHop = tempLink.routerArrivo;
            /* Inizializzazione entry da inserire nella tabella */
            strcpy(nodoTemp, tempLink.routerArrivo);
            strcpy(entryTemp.nextHop, nextHop);
            strcpy(entryTemp.destinazione, nodoTemp);
            entryTemp.costo = costo;
            /* Se il nodo di destinazione non è già presente nella tabella viene inserita per la
               prima volta */
            if(!containsTabella(testa, nodoTemp)){
                tTemp = (struct llTabella *)malloc(sizeof(struct llTabella));
                tTemp->riga = entryTemp;
                tTemp->next = NULL;
                while(tabella->next != NULL)
                    tabella = tabella->next;
                tabella->next = tTemp;
                tabella = tabella->next;
                priorityQueue = inserisciPriority(priorityQueue, nodoTemp, costo);
            }
            /* Se il nodo di destinazione è già presente nella tabella allora
               viene confrontato con quello temporaneo */
            else {
                if(costoRouterPrecedente(testa, nodoTemp) > costo) {
                    tabella = sostituisciTabella(testa, entryTemp);
                    priorityQueue = inserisciPriority(priorityQueue, nodoTemp, costo);
                }
            }
            linksTemp = linksTemp->next;
        }
        /* Viene rimosso il primo router della tabella in quento è stato completamente
           analizzato */
        priorityQueue = rimuoviPriority(priorityQueue, priorityQueue->nodo);
        linksTemp = links;
    }

    return testa;
}

/*
 * Funzione per trovare il next hop di una destinazione
 */
const char * getNextHop(struct llTabella *t, char dest[15]) {
    while(t != NULL) {
        if(strcmp(t->riga.destinazione, dest) == 0)
            return t->riga.nextHop;
        t = t->next;
    }
}

/*
 * Funzione che sostituisce una entry nella tabella con quella passata come
 * parametro
 *
 * @param t Linked list
 * @param e entry da inserire al posto di quella vecchia
 * @return la nuova linked list
 */
struct llTabella *sostituisciTabella(struct llTabella *t, struct entry e) {
    struct llTabella *testa = t;

    while(strcmp(t->riga.destinazione, e.destinazione) != 0)
        t = t->next;
    t->riga = e;
    return testa;
}

/*
 * Funzione che controlla se una destinazione è già presente nella tabella,
 * in modo da poter sapere se bisogna inserirla per la prima volta o sostituire
 * quella vecchia.
 *
 * @param t Linked list
 * @param nodo nome del nodo da controllare
 * @return true se la destinazione è già presente, false altrimenti
 */
bool containsTabella(struct llTabella *t, char nodo[15]) {
    while(t != NULL && strcmp(t->riga.destinazione, nodo) != 0)
        t = t->next;
    if(t == NULL)
        return false;
    return true;
}

/*
 * Funzione per aggiungere un elemento nella priority queue
 *
 * @param p Linked list
 * @param nodo nome nodo
 * @param valore costo da start fino al nodo, per determinare la priorità
 * @return Testa della linked list
 */
struct priority *inserisciPriority(struct priority *p, char nodo[15], int valore) {
    struct priority *pTemp;
    struct priority *testa = p;

    p = p->next;
    /* Controllo se l'elemento è già presente nella priority queue e se presente
       lo elimina ed inserisce quello nuovo */
    if(containsPriority(p, nodo))
        /* L'elemento viene eliminato di principio in quanto questa funzione viene
           richiamata solo se si trova un percorso più corto per il nodo e quindi
           di sicuro i suoi dati nella priority cambiano */
        p = rimuoviPriority(p, nodo);

    pTemp = (struct priority *)malloc(sizeof(struct priority));
    strcpy(pTemp->nodo, nodo);
    pTemp->valore = valore;
    /* Se è presente un solo elemento nella tabella allora quello nuovo viene
       inserito alla fine */
    if(p == NULL) {
        p = pTemp;
        p->next = NULL;
        testa->next = p;
    }
    /* Se esisitono più elementi nella priority l'elemento viene inserito prima
       del primo nodo che ha il campo valore maggiore del suo */
    else {
        p = testa;
        while(p->next != NULL && p->next->valore < valore)
            p = p->next;
        pTemp->next = p->next;
        p->next = pTemp;
    }
    return testa;
}

/*
 * Funzione che rimuove un elemento dalla priority queue.
 *
 * @param p Linked list
 * @param nodo nome nodo da rimuovere
 * @return Testa della linked list
 */
struct priority *rimuoviPriority(struct priority *p, char nodo[15]) {
    struct priority *testa = p;

    /* Controllo se l'elemento da rimuovere dalla priority si trova in testa */
    if(strcmp(p->nodo, nodo) == 0)
        return p->next;
    else {
        while(strcmp(p->next->nodo, nodo) != 0)
            p = p->next;
        p->next = p->next->next;
        return testa;
    }
}

/*
 * Funzione che controlla se un nodo è già stato inserito nella priority queue
 *
 * @param p Linked list
 * @param nodo nome del nodo da ricercare
 * @return true se è presente, false al contrario
 */
bool containsPriority(struct priority *p, char nodo[15]) {
    while(p != NULL && strcmp(p->nodo, nodo) != 0)
        p = p->next;
    if(p != NULL)
        return true;
    return false;
}

/*
 * Funzione che restituisce il costo per arrivare ad un nodo, in questo caso
 * il router è quello che precede un nodo di destinazione analizzato.
 *
 * @param tabella linked list che rappresenta la tabella
 * @param nodo destinazione
 * @return
 */
int costoRouterPrecedente(struct llTabella *tabella, char nodo[15]) {
    while(strcmp(tabella->riga.destinazione, nodo) != 0) {
        tabella = tabella->next;
    }
    return tabella->riga.costo;
}

/**
 * Funzione per rimuovere un router dalla rete
 */
bool rimuoviConnessione() {
    char rPartenza[15], rArrivo[15];
    struct Links *testa = links;

    /* Inserimento dati della connessione da rimuovere */
    printf("Rimozione connessione:\n");
    printf("Inserisci router di partenza: ");
    scanf("%s", rPartenza);
    printf("Inserisci router di arrivo: ");
    scanf("%s", rArrivo);

    /* Controllo se la connessione da rimuovere si trova in testa o in mezzo alla
       linked list */
    if(strcmp(rPartenza, links->connessione.routerPartenza) == 0
        && strcmp(rArrivo, links->connessione.routerArrivo) == 0) {
        testa = links->next;
        printf("\nConnessione rimossa con successo!");
        links = testa;
        return true;
    }
    while(links->next != NULL) {
        if(strcmp(rPartenza, links->next->connessione.routerPartenza) == 0
            && strcmp(rArrivo, links->next->connessione.routerArrivo) == 0) {
            links->next = links->next->next;
            printf("\nConnessione rimossa con successo!");
            links = testa;
            return true;
        }
        links = links->next;
    }
    links = testa;
    printf("\nConnessione non esistente!\n");
}
