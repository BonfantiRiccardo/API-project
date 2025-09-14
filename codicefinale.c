/*      COMMENTO SULLA MIA SOLUZIONE AL PROGETTO:

Il primo approccio è stato quello di produrre un programma che funzionasse e che fosse abbastanza semplice da implementare, per questo motivo ho scelto di immagazzinare le stazioni
in una lista doppiamente concatenata ordinata che rappresentasse l'autostrada, ciò comportava che ricerca, inserimento e cancellazione fossero O(n) n=stazioni. Per pianificare il percorso
utilizzavo invece inzialmente una struttura grafo che contenesse i nodi con le loro liste di adiacenza, che portava il programma ad occupare troppa memoria.
Dopo aver ottenuto un programma funzionante, ma decisamente troppo lento e occupante troppa memoria, ho deciso di passare all'ottimizzazione: innanzitutto ho inserito un nuovo puntatore
che tenesse traccia della stazione usata più di recente in modo tale da rendere O(1) l'inserimento e la cancellazione ripetuta di auto in una stazione. Dopodichè ho deciso di scartare
l'idea della struttura grafo e di calcolare il percorso direttamente dalla lista ordinata. Infine ho deciso di ampliare la struttura delle stazioni rendendo possibile anche la creazione di
un albero binario di ricerca, questo mi ha permesso di rendere ricerca, inserimento e cancellazione O(h) h=altezza albero.

Alcune funzioni sono implementate seguendo la linea guida dello pseudocodice sulle slides da lei fornite durante il corso di Algoritmi e principi dell'informatica, tutto (anche il resto)
è comunque di mia produzione.

È possibile raffinare ulteriormente l'ottimizzazione aggiungendo funzioni che creino e mantengano un albero Rosso-Nero, ma per questioni di tempo non sono riuscito ad implementare
una soluzione che fosse stabile e robusta.

*/
//-------------------------------------------------------------------------------------------------------------------------------------------------------- LIBRERIE

#include <stdio.h>                                    //importo le liberire necessarie per l'utilizzo di funzioni basilari come printf, scanf, strcmp, ecc...
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//-------------------------------------------------------------------------------------------------------------------------------------------------------- COSTANTI

#define MAXAUTO 512                                   // massimo di auto che possono stare in un parcheggio
#define LEN 18                                        //lunghezza massima della stringa in ingresso considerando le azioni: pianifica-percorso,
#define AS "aggiungi-stazione"                        //  aggiungi-stazione, demolisci-stazione, ...
#define DS "demolisci-stazione"                       //definisco dei codici per il tipo di azione da compiere
#define AA "aggiungi-auto"
#define RA "rottama-auto"
#define PP "pianifica-percorso"
#define INF 999999999                                 //definisco la distanza infinita come un numero molto più alto della distanza tra le singole stazioni che andremo ad analizzare

//-------------------------------------------------------------------------------------------------------------------------------------------------------- STRUTTURE DATI

typedef struct list_s {                               // struttura dati che rappresenta le stazioni, può essere usata sia come un albero (utile al fine di ottimizzare inserimento e
  int32_t dist;                                       //cancellazione), sia come lista doppiamente concatenata (utile per ottimizzare il calcolo del percorso)
  int32_t numauto;                                    //dist-> rappresenta la distanza della stazione dall'inizio dell'autostrada,    numauto-> rappresenta il numero esatto di auto
  int32_t parcheggio[MAXAUTO];                        //presenti in questo momento nella stazione (utile per lavorare sul vettore parcheggio),    parcheggio[]-> è il vettore contenente le
  struct list_s *l;                                   //figlio sx                                       //auto, rappresentate dalla loro autonomia
  struct list_s *r;                                   //figlio dx
  struct list_s *p;                                   //padre
  struct list_s *next;                                //successore
  struct list_s *prev;                                //predecessore
} stazioni;

typedef struct tupla_s {                              //struttura di puntatori a elementi della struttura stazioni
  stazioni* aRoot;                                    //punta alla radice dell'albero delle stazioni
  stazioni* prec;                                     //punta alla stazione usata più di recente, utile per l'ottimizzazione di alcuni processi come l'aggiungi e rimuovi auto
} tupla;

stazioni* nil = NULL;                                 //riferimento a cui puntano tutti i nodi ai margini dell'albero (esemp: root->p = nil, foglia->l = nil, foglia->r = nil)

//-------------------------------------------------------------------------------------------------------------------------------------------------------- SOTTOPROGRAMMI

void motosega (stazioni* );                           //abbatte l'albero delle stazioni definitivamente (parametri: aRoot)
stazioni* cercaBST (stazioni* , int32_t );            //cerca la stazione a data distanza nella lista e la restituisce, altrimenti restituisce nil (parametri: aRoot, dist)
stazioni* successore (stazioni* , stazioni* );        //cerca il successore della stazione passata come parametro (parametri: aRoot, x)

tupla* aggstazione (tupla* );                         //verifica se la stazione alla dist letta esiste già, altrimenti chiama nuovastazione per crearla (parametri: autostrada)
tupla* nuovastazione (tupla* , int32_t );             //crea nuova stazione e chiama inserisciBST per inserirla nell'albero, mette le auto nel parcheggio ordinandole (parametri: a, dist)
stazioni* inserisciBST (stazioni* , stazioni* );      //trova la posizione nell'albero in cui mettere la nuova stazione (parametri: aRoot, nuova)
int32_t comparaauto (const void* , const void* );     //funzione utilizzata da qsort per ordinare il parcheggio

tupla* demstazione (tupla* );                         // verifica se la stazione alla dist letta non esiste, altrimenti chiama cancellaBST per eliminarla (parametri: autostrada)
stazioni* cancellaBST (stazioni* , stazioni* );       //elimina la stazione dall'albero e libera lo spazio allocato (parametri: aRoot, daelimin)

tupla* aggauto (tupla* );                             // aggiunge l'auto di autonomia letta alla stazione a dist letta, a meno che il parcheggio sia pieno (parametri: autostrada)
int32_t nuovaauto (int32_t [], int32_t , int32_t );   //inserisce la nuova auto (parametri: parcheggio, numauto, autonomia)

tupla* rottauto (tupla* );                            // rottama l'auto di autonomia letta alla stazione a dist letta, a meno che essa non esista (parametri: autostrada)
int32_t ricercabin (int32_t [], int32_t , int32_t , int32_t );            //cerca l'autonomia dell'auto tra quelle già presenti (parametri: parcheggio, autoleft, autoright, datrovare)

void pianifica_percorso (tupla* );                    //legge le stazioni di partenza e arrivo e in base ad esse crea la lista, poi chiama calcolaestampa (parametri: autostrada)
void calcolaestampa (stazioni* , stazioni* , int32_t );         //calcola il miglior percorso in base alla lista e alle autonomie, poi lo stampa (parametri: part, arr, numnodi)

//-------------------------------------------------------------------------------------------------------------------------------------------------------- MAIN

int main(int argc, char *argv[]) {
  char azione[LEN+1];                                 //lunghezza massima della stringa in ingresso + terminatore
  tupla *autostrada = NULL;

  if ((nil = malloc(sizeof(stazioni)))) {             //creiamo nil e inizializziamo tutto a NULL
    nil->p = NULL;                                    //facciamo puntare tutte le foglie qua (anche root->p)
    nil->l = NULL;
    nil->r = NULL;

    if((autostrada = malloc(sizeof(tupla)))) {        //creiamo il puntatore alla radice dell'albero delle stazioni e alla stazione più recente, inizializzando tutto a nil
      autostrada->aRoot = nil;
      autostrada->prec = nil;
    }
  }

  while (scanf("%s", azione) == 1) {                  // cicla su stdin finchè non legge EOF, dopodichè esce

    if (strcmp(azione, AS) == 0) {                    //compara la stringa in ingresso con le varie azioni possibili e seleziona quella corretta
      autostrada = aggstazione(autostrada);
    }

    else if (strcmp(azione, DS) == 0) {
      autostrada = demstazione(autostrada);
    }

    else if (strcmp(azione, AA) == 0) {
      autostrada = aggauto(autostrada);
    }

    else if (strcmp(azione, RA) == 0) {
      autostrada = rottauto(autostrada);
    }

    else if (strcmp(azione, PP) == 0) {
      pianifica_percorso(autostrada);
    }

    else
      printf("Azione non valida\n");

  }

  motosega(autostrada->aRoot);                        //libera tutti i nodi dell'albero una volta finita l'esecuzione

  free(autostrada);                                   //libera la tupla

  free(nil);                                          //libera nil

  return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- distrugge l'albero

void motosega (stazioni* root) {                      //il processo è O(n) dove n sono i nodi dell'albero (li visitiamo tutti una volta)
  if (root == nil)                                    //se root è nil ritorna al nodo precedente
    return;

  motosega (root->l);                                 //chiama motosega sul nodo più a sinistra, se esso è nil allora passa al nodo a destra
  motosega (root->r);                                 //se anche il nodo a destra è nil siamo nel punto più profondo dell'albero (foglia)

  free(root);                                         //eliminiamo quindi questa foglia e risaliamo l'albero
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- Ricerca nel BST

stazioni* cercaBST (stazioni* root, int32_t n) {      //il processo è O(h) dove h è l'altezza dell'albero (al megio log(n))
  if (root == nil || root->dist == n)
    return root;                                      //ho trovato la stazione quindi la ritorno

  if (root->dist < n)                                 //se la dist della staz a cui mi trovo è < di quella da trovare:
    return cercaBST (root->r, n);                     //scendo nel sottoalbero destro e la cerco lì
  else
    return cercaBST (root->l, n);                     //altrimenti scendo nel sottoalbero sinistro e la cerco lì
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- Trova il succes nel BST

stazioni* successore (stazioni* root, stazioni* x) {  //il processo è O(h) dove h è l'altezza dell'albero (al megio log(n))
  stazioni* tmp, *tmp2;                               //puntatori per visitare l'albero

  if (x->r != nil) {                                  //controlla se il figlio dx è != nil, in tal caso:
    tmp = x->r;                                       //assegna tmp ad esso
    while (tmp->l != nil)                             //cerca il minimo in tale sottoalbero, cioè il nodo più a sinistra
      tmp = tmp->l;
    return tmp;                                       //ritorna il minimo
  }

  tmp = x->p;                                         //assegna a tmp il valore del padre
  tmp2 = x;                                           //assegna a tmp2 il valore della staz di cui trovare il successivo
  while (tmp != nil && tmp->r == tmp2) {              //fintantochè tmp2 è il figlio dx di tmp e tmp != nil
    tmp2 = tmp;                                       //risalgo l'albero facendo puntare tmp2 a tmp e tmp a suo padre
    tmp = tmp->p;
  }                                                   //la prima volta che tmp2 sarà figlio sx allora ho trovato il successore

  return tmp;
}



//-------------------------------------------------------------------------------------------------------------------------------------------------------- AGGIUNGI STAZIONE

tupla* aggstazione(tupla *a) {                        //TUTTA QUESTA PROCEDURA è O(h) dove h è l'altezza dell'albero (al megio log(n))
  int32_t dist, ret, i=0, n, auton;                   //dist = immagazzina la distanza, ret = controlla il valore di return da scanf, i = contatore per auto, n = numauto, auton = autonomia
  stazioni* p = NULL;                                 //inizializza un puntatore per la ricerca

  ret = scanf("%d", &dist);                           //legge la distanza a cui posizionare la stazione

  if (ret != 1)                                       //il compilatore mi obbliga a prendere il risultato di scanf e a controllarlo
    return NULL;

  if (a->prec != nil && a->prec->dist == dist) {      //se la stazione precedente è alla stessa dist di quella che si vuole aggiungere:
    ret = scanf("%d", &n);                            //legge il num di auto per fare avanzare l'input fino alla nuova azione
    while (i<n) {                                     //legge finchè non finiscono le macchine
      ret = scanf("%d", &auton);
      i++;
    }
    printf ("non aggiunta\n");                        //non aggiunge la stazione perchè c'è già
    return a;
  } else
    p = cercaBST (a->aRoot, dist);                    //se la stazione a quella distanza esiste la ritorna, la ricerca impiega O(log(h))

  if(p != nil && p->dist == dist) {                   //se la stazione esiste già:
    ret = scanf("%d", &n);                            //legge il num di auto per fare avanzare l'input fino alla nuova azione
    while (i<n) {                                     //legge finchè non finiscono le macchine
      ret = scanf("%d", &auton);
      i++;
    }
    printf ("non aggiunta\n");                        //non aggiunge la stazione perchè c'è già
    return a;
  }

  a = nuovastazione (a, dist);                        //altrimenti la stazione va aggiunta, alloca spazio per la nuova stazione e la inserisce nell'albero

  printf("aggiunta\n");

  return a;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- Gestisce inserimento

tupla* nuovastazione (tupla* a, int32_t d) {          //tutta la procedura è O(h + mlog(m)) dove m è il num di auto
  stazioni* p;
  int32_t ret, n, auton, i = 0;                       // ret = controlla il valore di return da scanf, i = contatore per auto, n = numauto, auton = autonomia

  if ((p = malloc (sizeof(stazioni)))) {              //alloco lo spazio necessario per un nuovo elemento della struttura stazioni
    p->dist = d;                                      //assegno al puntatore il valore della distanza letto
    p->l = nil;                                       //inizializzo i puntatori dell'albero a nil
    p->r = nil;
    p->p = nil;
    p->next = NULL;                                   //inizializzo i puntatori della lista a NULL
    p->prev = NULL;

    a->aRoot = inserisciBST (a->aRoot, p);            //chiamo la funzionee che inserisce il nodo nell'albero

    a->prec = p;                                      //assegna un nuovo valore a prec che è quello della stazione corrente

    ret = scanf("%d", &n);                            //legge il num di auto e lo inserisce nella struttura
    if (ret != 1)
      return NULL;
    p->numauto = n;

    while (i<n) {                                     //legge finchè non finiscono le macchine e le inserisce nel parcheggio
      ret = scanf("%d", &auton);
      p->parcheggio[i] = auton;                       //complessità O(m) dove m è il num di auto
      i++;
    }

    qsort (p->parcheggio, p->numauto, sizeof(int32_t), comparaauto);      //ordina il parcheggio con qsort, complessità O(mlog(m)) dove m sono il num di auto

  } else
    printf ("Errore allocazione\n");

  return a;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- Inserisce nel BST

stazioni* inserisciBST (stazioni* root, stazioni* nuova) {                //la procedura è O(h) dove h è l'altezza dell'albero (al megio log(n))
  stazioni* preced, *curr;                            //preced = tiene traccia dell'elemento precedente, curr = punta all'elemento attualmente analizzato (corrente)

  preced = nil;                                       //inizializzo i due puntatori che mi servono per scorrere l'albero e per tenere traccia del penultimo elem visto
  curr = root;

  while (curr != nil) {                               //fintantochè il nodo analizzato non è nil:
    preced = curr;                                    //tengo traccia di curr prima di muovermi sull'albero

    if (curr->dist < nuova->dist)                     //se la dist della staz da inserire è maggiore di quella a cui mi trovo adesso allora:
      curr = curr->r;                                 //sposto curr sul figlio a destra
    else                                              //altrimenti
      curr = curr->l;                                 //sposto curr sul filgio a sinistra
  }

  nuova->p = preced;                                  //il padre della nuova stazione punterà alla penultima visitata ovvero la precedente a curr

  if (preced == nil)                                  //se la preced è rimasta nil vuol dire che l'albero è vuoto e ritorno quindi la nuova stazione come root
    return nuova;

  else if (nuova->dist < preced->dist)                //altrimenti se la dist della nuova staz è minore della preced
    preced->l = nuova;                                //la inserisco come figlio sinistro
  else
    preced->r = nuova;                                //altrimenti la inserisco come figlio destro

  return root;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- Compara Auto

int comparaauto (const void* p, const void* q) {      //tutta la procedura è O(1)
  const int32_t a1 = *(int32_t*) p, a2 = *(int32_t*) q;                   //dichiaro e inizializzo le due variabili da confrontare

  if (a1 < a2)                                        //restituisco i valori che vuole qsort in base al confronto
    return -1;
  else if (a1 == a2)
    return 0;
  else
    return 1;

}



//-------------------------------------------------------------------------------------------------------------------------------------------------------- DEMOLISCI STAZIONE

tupla* demstazione (tupla* a) {                       //TUTTA QUESTA PROCEDURA è O(h) dove h è l'altezza dell'albero (al megio log(n))
  int32_t dist, ret;                                  // ret = controlla il valore di return da scanf, dist = distanza staz da eliminare
  stazioni* p;                                        // p = punta alla staz da eliminare

  ret = scanf("%d", &dist);                           //legge la distanza della stazione da eliminare

  if (ret != 1)
    return NULL;

  p = cercaBST (a->aRoot, dist);                      //ricerca la staz

  if (p != nil) {                                     //se la staz esiste allora:
    if (a->prec != nil && a->prec->dist == dist) {                        //controlla se è salvata come staz precedente e nel caso riassegna prec
      if (p != a->aRoot)                              //se p non è radice assegna prec al padre
        a->prec = p->p;
      else if (p->l != nil)                           //se il figlio sinistro di p non è nil assegna prec
        a->prec = p->l;
      else if (p->r != nil)                           //se il figlio destro di p non è nil assegna prec
        a->prec = p->r;
      else                                            //altrimenti l'albero è vuoto qunidi prec è nil
        a->prec = nil;
    }

    a->aRoot = cancellaBST (a->aRoot, p);                         //chiama la cancellaBST

    if (nil->p != NULL)                               //controllo per sicurezza sul padre di nil che potrebbe essere stato modificato
      nil->p = NULL;

    printf("demolita\n");

  } else
    printf("non demolita\n");

  return a;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- Cancella dal BST

stazioni *cancellaBST (stazioni *root, stazioni* x) {                     //la procedura è O(h) dove h è l'altezza dell'albero (al megio log(n))
  stazioni *tmp = nil, *del;                          //tmp = puntatore al nodo che sostituirà la staz eliminata, del = puntatore alla staz da eliminare
  int32_t i;                                          //i = contatore per posizione auto

  if (x->l == nil || x->r == nil)                     //se la  staz da elimin ha figlio sx o dx uguali a nil allora:
    del = x;                                          //del punterà a tale nodo
  else
    del = successore(root, x);                        //altrimenti cerco il nodo successore a x che avrà sicuramente o figlio dx o sx uguali a nil e punto del

  if (del->l != nil)                                  //se del ha figlio sx != da nil allora:
    tmp = del->l;                                     //il nodo elimin del sarà sostituito da tale nodo quindi lo faccio puntare a tmp
  else if (del->r != nil)                             //altrimenti se del ha filgio dx != da nil (nel caso entrambi siano nil allora del sarà sostituito da nil che è già puntato da tmp)
    tmp = del->r;                                     //il nodo elimin del sarà sostituito da tale nodo quindi lo faccio puntare a tmp

  if (del->p == nil) {                                //se del è la radice allora:
    root = tmp;                                       // riassegna la radice come tmp correggendo anche tmp->p
    tmp->p = nil;
  }
  else if (del == del->p->l) {                        //altrimenti se del è figlio sx del padre:
    del->p->l = tmp;                                  //riassegno il figlio sx del padre a tmp e correggo il padre di tmp con del->p
    tmp->p = del->p;
  }
  else if (del == del->p->r) {                        //altrimenti se del è figlio dx del padre:
    del->p->r = tmp;                                  //riassegno il figlio dx del padre a tmp e correggo il padre di tmp con del->p
    tmp->p = del->p;
  }

  if (del != x) {                                     //se del è il successore di x, quindi se x ha entrambi i figli != nil
    x->dist = del->dist;
    x->numauto = del->numauto;                        //mette in x i valori di del

    for (i = 0; i < x->numauto; i++)
      x->parcheggio[i] = del->parcheggio[i];
  }

  free (del);                                         //libera lo spazio occupato da del

  return root;
}



//-------------------------------------------------------------------------------------------------------------------------------------------------------- AGGIUNGI AUTO

tupla* aggauto (tupla* a) {                           //TUTTA QUESTA PROCEDURA è O(h) dove h è l'altezza dell'albero (al megio log(n)) e nascosto c'è O(m) con m num di auto
  int32_t dist, auton, ret;                           //dist = immagazzina la distanza, ret = controlla il valore di return da scanf, auton = autonomia
  stazioni *p;                                        //puntatore per la staz a dist letta

  ret = scanf("%d", &dist);                           //legge la distanza della stazione a cui aggiungere l'auto

  if (ret != 1)
    return NULL;

  ret = scanf("%d", &auton);                          //legge l'autonomia dell'auto da aggiungere

  if (a->prec != nil && a->prec->dist == dist)
    p = a->prec;                                      //ottimizzazione tramite a->prec
  else
    p = cercaBST (a->aRoot, dist);                    //cerca nell'albero la stazione a distanza letta

  if (p != nil && p->dist == dist && p->numauto <= MAXAUTO) {             //se la stazione non è presente oppure se il parcheggio è pieno non aggiunge l'auto

    p->numauto = nuovaauto(p->parcheggio, p->numauto, auton);             //inserisce la nuova auto e incrementa il num di auto presenti nel parcheggio

    printf ("aggiunta\n");

    a->prec = p;                                      //aggiorna il valore di prec con l'ultima staz utilizzata
  }
  else
    printf("non aggiunta\n");

  return a;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- Nuova Auto

int32_t nuovaauto (int32_t parch[], int32_t n, int32_t auton) {           //tutto questo processo è O(m) con m num di auto
  int32_t i, magg = n;                                //i = contatore e indice per la posiz delle auto, n = numauto, magg = indice della prima auto con autonom > di quella nuova

  for (i = 0; i < n && magg == n; i++) {              //trovo la prima auto con auton più grande di quella da aggiungere e mi segno l'indice in c
    if (parch[i] >= auton)
      magg = i;
  }

  for (i = n-1; magg <= i; i--) {                     //partendo dal fondo, finchè non arriviamo all'indice salvato spostiamo in avanti di 1 tutti gli elementi del vettore
    parch[i+1] = parch[i];
  }
  parch[magg] = auton;                                   //infine inseriamo la nuova auto

  return n+1;                                         //ritorno il nuovo numero di auto
}



//-------------------------------------------------------------------------------------------------------------------------------------------------------- ROTTAMA AUTO

tupla* rottauto (tupla* a) {                          //TUTTA QUESTA PROCEDURA è O(h) dove h è l'altezza dell'albero (al megio log(n)) e nascosto c'è O(m) con m num di auto
  int32_t dist, auton, i, posiz, ret;                 //dist = immagazzina la distanza, ret = controlla il valore di return da scanf, auton = autonomia, i = contatore,
  stazioni* p;                                        //posiz = indice dell'auto da eliminare, p = puntatore alla staz dell'auto

  ret = scanf("%d", &dist);                           //legge la distanza della stazione a cui si trova l'auto da rottamare

  if (ret != 1)
    return NULL;

  ret = scanf("%d", &auton);                          //legge l'autonomia dell'auto da rottamare

  if (a->prec != nil && a->prec->dist == dist)
    p = a->prec;                                      //ottimizzazione tramite a->prec
  else
    p = cercaBST (a->aRoot, dist);                    //cerca nell'albero la stazione a distanza letta

  if (p != nil && p->dist == dist) {
    posiz = ricercabin (p->parcheggio, 0, p->numauto, auton);             //se la stazione esiste cerca al suo interno l'auto con data autonomia da rottamare
    if (posiz != -1) {                                                    //se la posiz è -1 l'auto non è presente

      for (i = posiz; i < p->numauto; i++)            //se l'auto viene trovata, sposto tutti gli elementi successivi a essa indietro di 1
        p->parcheggio[i] = p->parcheggio[i+1];

      p->numauto--;                                   //diminuisco infine il num di auto nel parcheggio

      printf ("rottamata\n");
    }
    else
      printf ("non rottamata\n");

    a->prec = p;                                      //aggiorno il valore di prec con la staz appena usata
  }
  else
    printf ("non rottamata\n");

  return a;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- Ricerca Binaria

int32_t ricercabin(int32_t parch[], int32_t l, int32_t r, int32_t x) {    //tutto questo processo è O(log(m)) con m num di auto
  int32_t med;

  if (r >= l) {                                       //se l'elemento destro è >= a quello sinistro allora esegue, altrimenti l'elemento non è presente nel vettore
    med = l + (r - l) / 2;                            //calcolo dell'elemento mediano

    if (parch[med] == x)                              //se l'auto a tale indice è di autonomia x allora ritorno l'indice
      return med;

    else if (parch[med] > x)                          //altriemtni se l'elemento mediano è > di quello da trovare continuo la ricerca nella sezione di sinistra del vettore
      return ricercabin(parch, l, med - 1, x);

    return ricercabin(parch, med + 1, r, x);          //altrimenti continuo la ricerca nella sezione destra

  }

    return -1;                                        //se arrivo qui vuol dire che l'auto non c'è
}



//-------------------------------------------------------------------------------------------------------------------------------------------------------- PIANIFICA PERCORSO

void pianifica_percorso (tupla* a) {
  stazioni* tmp1, *tmp2, *tmp3;                       //puntatori per creare la lista di staz, dist1 = dist staz partenza,
  int32_t dist1, dist2, ret, numnodi;                 //ret = controlla il valore di return da scanf, dist2 = dist staz arrivo, numnodi = numero staz tra dist1 e dist2

  ret = scanf("%d", &dist1);                          //legge la distanza della stazione di partenza

  if (ret != 1)
    return;

  ret = scanf("%d", &dist2);                          //legge la distanza della stazione di arrivo

  if (dist1 != dist2) {                               //se le stazioni sono diverse allora:

    if (dist1 < dist2) {                              //differenzia i casi in cui l'autostrada è percorsa nei sensi diversi
      if(a->prec != nil && a->prec->dist == dist1)
        tmp1 = a->prec;                               //ottimizzazione tramite a->prec
      else
        tmp1 = cercaBST(a->aRoot, dist1);             //cerca il riferimento alla prima stazione

      tmp2 = tmp1;
      tmp3 = tmp1;
      for(numnodi = 1; tmp2->dist != dist2; numnodi++) {                  //conta il numero di nodi del grafo (stazioni tra la partenza e l'arrivo)
        tmp2 = successore (a->aRoot, tmp2);                               //cerca inoltre il riferimento alla staz di arrivo
        tmp3->next = tmp2;                                                //nel frattempo creo la lista doppiamente concatenata
        tmp2->prev = tmp3;
        tmp3 = tmp2;
      }

    } else {                                          //stesso codice per il caso dist1 > dist2
      if(a->prec != nil && a->prec->dist == dist2)
        tmp2 = a->prec;                               //ottimizzazione tramite a->prec
      else
        tmp2 = cercaBST(a->aRoot, dist2);             //cerca il riferimento alla prima stazione

      tmp1 = tmp2;
      tmp3 = tmp2;
      for(numnodi = 1; tmp1->dist != dist1; numnodi++) {                  //conta il numero di nodi del grafo (stazioni tra la partenza e l'arrivo)
        tmp1 = successore (a->aRoot, tmp1);                               //cerca inoltre il riferimento alla staz di arrivo
        tmp1->prev = tmp3;                                                //nel frattempo creo la lista doppiamente concatenata
        tmp3->next = tmp1;
        tmp3 = tmp1;
      }

    }

    calcolaestampa (tmp1, tmp2, numnodi);             //chiama la calcola e stampa passando i riferimenti alle staz part e arrivo e il num di staz tra di esse

  } else if (dist1 == dist2)                          //altrimenti se le stazioni sono uguali stampa solo una delle due
    printf ("%d\n", dist1);

  return;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------- Calcola e Stampa

void calcolaestampa (stazioni* part, stazioni* arr, int32_t numnodi) {
  stazioni* tmp1, *tmp2;                              //riferimenti per visita lista, predecessore[]= contiene gli indici della staz che precede quella in tale posiz,
  int32_t predecessore[numnodi], percorso[numnodi];             //percorso= contiene la lungh del percorso più breve dalla part alla staz in tale posiz, i, j, cont= contatore, ind= indice,
  int32_t i=0, j, curr, y, cont, tappe, ind, differ=INF;        //curr= indice che stiamo analizz, y= indice nodo da verificare, tappe= num tappe percorso, differ= dist tra y e curr

  i = 0;
  predecessore[i] = -1;                               //il predecessore della stazione di partenza è un valore non valido

  for (j = 0; j < numnodi; j++)
    percorso[j] = INF;                                //inizializza il percorso che collega i nodi a valore INF

  percorso[i] = 0;                                    //setta la lunghezza del percorso dalla stazione di partenza a 0

  tmp1 = part;                                        //inizializza tmp1 come la partenza

  if (part->dist < arr->dist) {                       //differenzia i casi in cui la partenza sia prima dell'arrivo e viceversa

    for (j = 0; j < numnodi-1; j++) {                 //fintantochè non ho esplorato tutti i nodi tranne l'arrivo allora eseguo:
      curr = j;                                       //analizziamo i nodi in ordine perchè sicuramente una staz che viene prima di un'altra avrà un percorso che è <=

      if(percorso[curr] == INF)                       //se l'indice corrente ha percorso INF allora vuol dire che nessuna staz lo raggiunge e quindi nessun percorso è possibile
        break;                                        //esce dal ciclo for

      else {                                          //altrimenti controllo a cosa è connesso curr:
        y = curr+1;                                   //analizza le stazioni successive che sono collegate:
        tmp2 = tmp1->next;
        if (tmp2 != NULL)                             //controllo che tmp2 sia un valore possibile (anche se arr->next fosse un valore valido viene scartato nel while)
          differ = tmp2->dist - tmp1->dist;           //calcolo la differ di dist tra staz

        while(tmp1->parcheggio[tmp1->numauto-1] >= differ && tmp2 != arr->next) {        //fintantochè l'auto con autonomia maggiore è più grande della differ e non siamo arrivati all'arr:

          if (percorso[curr]+1 < percorso[y]) {                           //se il percorso per arrivare a y è più lungo di curr + 1 allora:
            predecessore[y] = curr;
            percorso[y] = percorso[curr] +1;                              //aggiorno il valore di predecessore[y] e quello di percorso[y] con i nuovi dati

          } else if (percorso[curr]+1 == percorso[y]) {                   //altrimenti se i due percorsi sono uguali:
              if (curr < predecessore[y])             //controllo se l'indice del nodo corrente è minore dell'indice del nodo del predecessore già segnato, se ciò è vero allora
                predecessore[y] = curr;               //prediligo la staz con ind minore (ovvero dist minore da inizio autostrada) e setto il suo ind come nuovo predecessore
          }
          tmp2 = tmp2->next;                          //passo a tmp2 successivo per controllare se il nodo successivo è anch'esso connesso alla stazione considerata
          if (tmp2 != NULL)                           //controllo che tmp2 sia un valore possibile (anche se arr->next è un valore valido viene scartato nel while)
            differ = tmp2->dist - tmp1->dist;         //aggiorna il valore della differenza

          y++;                                        //passo all'indice della staz successiva
        }
      }
      tmp1 = tmp1->next;                              //passo ad analizzare la staz successiva
    }

  } else {                                            //ripeti per il caso in cui l'arrivo sia minore della partenza

    for (j = 0; j < numnodi-1; j++) {                 //fintantochè non ho esplorato tutti i nodi tranne l'arrivo allora eseguo:
      curr = j;                                       //analizziamo i nodi in ordine perchè sicuramente una staz che viene prima di un'altra avrà un percorso che è <=

      if(percorso[curr] == INF)                       //se l'indice corrente ha percorso INF allora vuol dire che nessuna staz lo raggiunge e quindi nessun percorso è possibile
        break;                                        //esce dal ciclo for

      else {                                          //altrimenti controllo a cosa è connesso curr:
        y = curr+1;                                   //analizza le stazioni successive che sono collegate:
        tmp2 = tmp1->prev;
        if (tmp2 != NULL)                             //controllo che tmp2 sia un valore possibile (anche se arr->prev è un valore valido viene scartato nel while)
          differ = tmp1->dist - tmp2->dist;

        while(tmp1->parcheggio[tmp1->numauto-1] >= differ && tmp2 != arr->prev) {        //fintantochè l'auto con autonomia maggiore è più grande della differ e non siamo arrivati all'arr:

          if (percorso[curr]+1 < percorso[y]) {                           //se il percorso per arrivare a y è più lungo di quello di curr + 1 allora:
            predecessore[y] = curr;
            percorso[y] = percorso[curr] +1;                              //aggiorno il valore di predecessore[y] e quello di percorso[y] con i nuovi dati

          } else if (percorso[curr]+1 == percorso[y]) {                   //altrimenti se i due percorsi sono uguali:
            if (curr > predecessore[y])               //controllo se l'indice del nodo corrente è maggiore dell'indice del nodo del predecessore già segnato, se ciò è vero allora
              predecessore[y] = curr;                 //prediligo la staz con ind maggiore (ovvero dist minore da inizio autostrada) e setto il suo ind come nuovo predecessore
          }
          tmp2 = tmp2->prev;                          //passo a tmp2 preced per controllare se il nodo successivo è anch'esso connesso alla stazione considerata
          if (tmp2 != NULL)                           //controllo che tmp2 sia un valore possibile (anche se  arr->prev è un valore valido viene scartato nel while)
            differ = tmp1->dist - tmp2->dist;         //aggiorna il valore della differenza

          y++;                                        //passo all'indice della staz successiva
        }
      }
      tmp1 = tmp1->prev;                              //passo ad analizzare la staz preced
    }
  }

  if (percorso[numnodi-1] == INF)                     //se la stazione di arrivo è ancora a distanza INF allora non è connessa quindi non c'è un percorso che la raggiunge
    printf ("nessun percorso\n");
  else {                                              //altrimenti ricorstruisco il miglior percorso:
    ind = j;                                          //salvo l'indice j che corrisponde alla stazione di arrivo
    for (cont = 1; predecessore[ind] != -1; cont++)   //conto quante sono le tappe scorrendo all'indietro il percorso dato dai predecessori
      ind = predecessore[ind];

    int32_t ris[cont];                                //inizializzo un vettore risultato
    tappe = cont;                                     //le tappe includono anche la stazione iniziale che ha valore di predecessore -1 e quindi prima non veniva contata

    cont--;                                           //decremento il contatore in modo tale da spostarlo sull'ultima cella del vettore
    ris[cont] = arr->dist;                            //la stazione di arrivo è sicuramente l'ultima del vettore
    cont--;

    tmp1 = arr;                                       //inizializzo una variabile che scorra sulle stazioni

    if (part->dist < arr->dist) {                     //faccio un controllo per differenziare i casi di part e arr < e >

      i = numnodi-1;                                  //setto il contatore i a numnodi-1 (indice dell'arrivo) in modo tale che decrementandolo si avvicini al predecessore
      while (predecessore[j] != -1) {                 //ciclo fintantochè non ho percorso tutte le tappe
        while (i > predecessore[j]) {                 //finchè il contatore i è maggiore dell'indice del predecessore lo decremento e
          tmp1 = tmp1->prev;                          //scorro le stazioni dall'arrivo verso la partenza finchè l'indice corrisponde alla stazione
          i--;
        }

        ris[cont] = tmp1->dist;                       //metto il valore della dist della stazione nel vettore risultato a indice che parte da prima di quello dell'arrivo e
        cont--;                                       //viene decrementato avvicinandosi allo 0 (cioè la partenza)
        j = predecessore[j];                          //aggiorno il valore dell'indice del predecessore
      }

    } else {                                          //gestisco il caso alternativo
      i = numnodi-1;
      while (predecessore[j] != -1) {
        while ( i > predecessore[j]) {
          tmp1 = tmp1->next;                          //in questo caso l'arrivo è minore quindi scorriamo le stazioni verso l'alto fino alla partenza
          i--;
        }

        ris[cont] = tmp1->dist;
        cont--;
        j = predecessore[j];
      }
    }

    printf("%d", ris[0]);
    for (i = 1; i < tappe; i++) {                     //stampo il vettore risultato
      printf(" ");
      printf("%d", ris[i]);
    }
    printf("\n");
  }

  return;
}
