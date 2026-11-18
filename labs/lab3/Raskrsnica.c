#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include "getch.c"

#define RING_SIZE 16

typedef struct
{

    unsigned int head;
    unsigned int tail;
    unsigned char niz[RING_SIZE];

} RingBuffer;

static RingBuffer ring;

static sem_t semFull;  // semafor za zauzeta mjesta
static sem_t semEmpty; // semafor za slobodna mjesta

static pthread_mutex_t bufferAccess; // mutex za pristup baferu
  
char getCharFromBuf(RingBuffer *buffer)
{
    int index;
    index = buffer->head;                          // zapamti trenutnu poziciju
    buffer->head = (buffer->head + 1) % RING_SIZE; // pomjeri buffer kruzno
    return buffer->niz[index];                     // vrati procitani znak
}

void putCharInBuf(RingBuffer *buffer, const char c)
{
    buffer->niz[buffer->tail] = c;
    buffer->tail = (buffer->tail + 1) % RING_SIZE;
}

 

int smjerIndeks(char c) {
    if (c == 'S')
        return 0;
    else if (c == 'Z')
        return 1;
    else if (c == 'J')
        return 2;
    else if (c == 'I')
        return 3;
    else return -1;
}

const char oznake [4] ={'S','Z','J','I'};

void *producer(void *param)
{

    char c;
    while (1)
    {
        c = toupper(getche());               // prebacuje q u veliko slovo
       

        if (c != 'Q' && c != 'S' && c != 'J' && c != 'I' && c != 'Z')
            continue; // ako nismo unijeli ni S J I Z ni Q ide na sljedecu iteraciju

        sem_wait(&semEmpty); // ceka na znak da ima slobodnih mjesta za upis u buffer

        pthread_mutex_lock(&bufferAccess);

            putCharInBuf(&ring, c); // mutexom zakljucavamo buffer kao dijeljeni resurs

        pthread_mutex_unlock(&bufferAccess);

        sem_post(&semFull);     //povecava broj zauzetih mjesta 
        
        if (c == 'Q')         // ako je korisnik unio q ili Q prekida program
            break;

    }
return NULL;

}

void *consumer(void *param)
{

char c;
int ukupno=0;               //ukupan broj auta na raskrsnici
int brojac [4] = {0,0,0,0};    //broji koliko auta ima iz odgovarajuceg smjera


while(1){

sem_wait(&semFull);         //ceka na signal da ima znakova u bufferu

pthread_mutex_lock(&bufferAccess); 

    c=getCharFromBuf(&ring);

pthread_mutex_unlock(&bufferAccess);

if(c == 'Q')
    break;

sem_post(&semEmpty);        //povecava broj slobodnih mjesta

brojac [smjerIndeks(c)]++;      //povecava broj auta iz odgovarajuceg smjera
ukupno++;

}

printf("\nRedoslijed prolaska: ");  

while(ukupno) // dok ima auta na raskrsnici
{ 

int prolazi=-1;

for(int i=0; i < 4; i++) {   //prolazi kroz redove auta

    if (brojac[i] > 0 && brojac[(i+1) % 4] == 0)     //prvenstvo ima onaj red ako ima auta u njemu 
        {                                           // i ako nema nikog sa desne strane
            prolazi=i;
            break;              //izlazi iz for petlje jer je nasao auto koje je doslo na red
        }
}

if(prolazi==-1)  // ako se desio deadlock, prvenstvo ima auto sa sjevera
{  
    prolazi = 0;
}

printf("%c", oznake[prolazi]);  // ispis redoslijeda prolazaka
brojac[prolazi]--;
ukupno--;
if(ukupno > 0)
    printf(" -> ");

}
printf("\n");
return NULL;

}

int main(void)
{

    pthread_t hProducer;
    pthread_t hConsumer;

    sem_init(&semFull, 0, 0);          // nula zauzetih mjesta na pocetku
    sem_init(&semEmpty, 0, RING_SIZE); // na pocetku ima 16 slobodnih mjesta

    pthread_mutex_init(&bufferAccess, NULL);

    printf("Unesite smjerove (S, Z, J, I), 'q' za kraj: "); 
    fflush(stdout);  

    pthread_create(&hProducer, NULL, producer, 0);
    pthread_create(&hConsumer, NULL, consumer, 0);

    pthread_join(hProducer, NULL);
    pthread_join(hConsumer, NULL);

    sem_destroy(&semEmpty);
    sem_destroy(&semFull);
    pthread_mutex_destroy(&bufferAccess);

    return 0;
}
