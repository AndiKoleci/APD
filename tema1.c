#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#include "tema1.h"

pthread_barrier_t barrier;
pthread_mutex_t mutex;

int isPow(int x, int y){
    //checks if x is something to the power of y
    //ex: 243 is something to the power of 5: 3^5
    //isPow(243, 5) = 1;
    int i;
    if(x==0) return 0;
    if(x==1) return 1;
    for(i=2; pow(i, y)<=x; i++){
        if(pow(i, y)==x) {
            return 1;
        }
    }
    return 0;
}

void *f(void *arg)
{	
    ThreadSctruct9 *tData = arg;
    int thread_id = tData->thread_id;
    Structura *d = tData->data;
    
    if(thread_id<d->nrMap){
        while(d->contor<d->contorMax){
            int x, times;
            char filename1[100];
            char num[31];
            int opened = 0;

            //-1 - nedeschis
            // 0 - deschis
            pthread_mutex_lock(&mutex);
            // if(v[contor]==-1){
            //     v[contor]=0;
            //     sprintf(filename1, "%s/in%d.txt", fileStart, contor+1);
            //     opened=1;
            // }
            if (d->contor < d->contorMax) {
                d->contor++;
                opened = 1;
                sprintf(filename1, "%s/in%d.txt", d->fileStart, d->contor);
            }
            pthread_mutex_unlock(&mutex);
            
            if(opened==1){
                FILE *fi1;
                fi1 = fopen(filename1, "r");
                fgets(num, 11, fi1);
                times = atoi(num); //atoi transforma din string in int
                //printf("%d", times);
                for(int i=0; i<times; i++){
                    fgets(num, 11, fi1);
                    x = atoi(num);
                    for(int j=0; j<5; j++){
                        if(isPow(x, j+2)){
                            d->MapperLists[thread_id][j] = insert(d->MapperLists[thread_id][j], x);
                        } 
                    }
                }
                fclose(fi1);
            }
        }

        //printf("Hi from Mapper %d\n", thread_id);
        //printf("\n");
        
    }
    //mapper
    pthread_barrier_wait(&barrier);

    //reducer
    if(thread_id>=d->nrMap){
        //reducer
        //printf("Hi from reducer %d\n", thread_id);
        for(int i=0; i<d->nrMap; i++){
            SortedList inter = d->MapperLists[i][thread_id-d->nrMap];
            while(inter!=NULL){
                d->ReducerLists[thread_id-d->nrMap] = insert(d->ReducerLists[thread_id-d->nrMap], inter->value);
                inter=inter->next;
            }
        }

        SortedList inter2 = d->ReducerLists[thread_id-d->nrMap];
        int count=1;
        while(inter2->next!=NULL){
            if(inter2->value!=inter2->next->value){
                count++;
            }
            inter2=inter2->next;
        }
        FILE *final;
        char filenamefinal[100];
        snprintf(filenamefinal, 30, "%s/out%d.txt", d->fileStart, thread_id-1);
        
        final=fopen(filenamefinal, "w");
        fprintf(final, "%d", count);
        fclose(final);
        
    }	
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    pthread_barrier_init(&barrier, NULL, atoi(argv[1])+atoi(argv[2]));
    pthread_mutex_init(&mutex, NULL);

    int r;
    pthread_t threads[atoi(argv[1])+atoi(argv[2])];
	ThreadSctruct9 arg[atoi(argv[1])+atoi(argv[2])]; //nr mapperi;
    char Ni[10]; //nr documente
    int N;
    char initFile[100];
    snprintf(initFile, 30, "%s", argv[3]);

    FILE *fi;
    fi = fopen(initFile, "r");

    fgets(Ni, 10, fi);
    N = atoi(Ni);

    //vectoru v[] va avea marimea N;

    // for(int i=0; i<N; i++){
    //     fgets(docname, 100, fi);
    //     //TODO: ADD THESE FILES SOMEWEHERE
    //     //printf("%s", docname);
    //     testfile = fopen(docname, "r");
    //     if(testfile!=NULL){
    //         printf("%s\n", docname);
    //         fseek(testfile, 0L, SEEK_END);
    //         fclose(testfile);
    //     }
    // }
    // fclose(fi);

    char *fileStart = strtok(initFile, "/"); //testX

    Structura data = {
        .nrMap = atoi(argv[1]),
        .nrRed = atoi(argv[2]),
        .fileStart = fileStart, 
        .contorMax = N
    };

    for(int i=0; i<atoi(argv[1])+atoi(argv[2]); i++){
        arg[i].thread_id=i;
        arg[i].data = &data;
        r = pthread_create(&threads[i], NULL, f, &arg[i]);

    }

    for(int i=0; i<atoi(argv[1])+atoi(argv[2]); i++){
        r = pthread_join(threads[i], NULL);

    }

    printf("\nMapperLists should look like this:\n");
    for(int i=0; i<atoi(argv[1]); i++){
        for(int j=0; j<atoi(argv[2]); j++){
            printf("{");
            SortedList inter = data.MapperLists[i][j];
            while(inter!=NULL){
                printf("%d ", inter->value);
                inter = inter->next;
            }
            printf("}");
        }
        printf("\n");
    }
    printf("\n");
    printf("\n");
    printf("\nReducerLists should look like this:\n");
    for(int i=0; i<atoi(argv[2]); i++){
        printf("{");
        SortedList inter = data.ReducerLists[i];
        while(inter!=NULL){
            printf("%d ", inter->value);
            inter = inter->next;
        }
        printf("}");
        printf("\n");
    }
    printf("\n");
    printf("\n");

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);
    pthread_exit(NULL);

	return 0;
}
