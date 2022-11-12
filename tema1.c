#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#include "tema1.h"

int a = 0;
pthread_mutex_t mutex;
pthread_barrier_t barrier;
int nrMap;
int nrRed;

SortedList MapperLists[50][50];
SortedList ReducerLists[50];

int isPow(int x, int y){
    //checks if x is something to the power of y
    //ex: 243 is something to the power of 5: 3^5
    //isPow(243, 5) = 1;
    int i;
    for(i=0; pow(i, y)<=x; i++){
        if(pow(i, y)==x) {
            return 1;
        }
    }
    return 0;
}

void *f(void *arg)
{	
    
    int thread_id = *(int *)arg;
    if(thread_id<nrMap){
        printf("Hi from thread %d\n", thread_id);
        //printf("\n");
        int x, times;
        char filename1[100];
        char filename2[100];
        char num[11];

        if(thread_id==0) {
            sprintf(filename1, "test0/in1.txt");
        }
        if(thread_id==1) {
            sprintf(filename1, "test0/in2.txt");
        }
        if(thread_id==2) {
            sprintf(filename1, "test0/in3.txt");
            sprintf(filename2, "test0/in4.txt");
        }
        
        FILE *fi1;
        FILE *fi2;
        fi1 = fopen(filename1, "r");
        fgets(num, 11, fi1);
        times = atoi(num); //atoi transforma din string in int
        for(int i=0; i<times; i++){
            fgets(num, 11, fi1);
            x = atoi(num);
            for(int j=0; j<5; j++){
                if(isPow(x, j+2)){
                    MapperLists[thread_id][j] = insert(MapperLists[thread_id][j], x);
                } 
            }
        }
        fclose(fi1);

        if(thread_id==2){
            fi2 = fopen(filename2, "r");
            fgets(num, 11, fi2);
            times = atoi(num);
            for(int i=0; i<times; i++){
                fgets(num, 11, fi2);
                x = atoi(num);
                for(int j=0; j<5; j++){
                    if(isPow(x, j+2)){
                        MapperLists[thread_id][j] = insert(MapperLists[thread_id][j], x);
                    } 
                }
            }
            fclose(fi2);
        }
    }

    pthread_barrier_wait(&barrier);

    if(thread_id>=nrMap){
        //reducer
        printf("Hi from reducer %d\n", thread_id);
        printf("\n");
        for(int i=0; i<nrMap; i++){
            SortedList inter = MapperLists[i][thread_id-nrMap];
            while(inter!=NULL){
                ReducerLists[thread_id-nrMap] = insert(ReducerLists[thread_id-nrMap], inter->value);
                inter=inter->next;
            }
        }

        SortedList inter2 = ReducerLists[thread_id-nrMap];
        int count=1;
        while(inter2->next!=NULL){
            if(inter2->value!=inter2->next->value){
                count++;
            }
            inter2=inter2->next;
        }
        FILE *final;
        char filenamefinal[100];
        snprintf(filenamefinal, 30, "test0/out%d.txt", thread_id-1);
        
        final=fopen(filenamefinal, "w");
        fprintf(final, "%d", count);
        fclose(final);
        
    }	
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    nrMap = atoi(argv[1]);
    nrRed = atoi(argv[2]);
    char* initFile = argv[3];
    pthread_barrier_init(&barrier, NULL, nrMap+nrRed);

    printf("%d %d %s\n\n", nrMap, nrRed, argv[3]);

    int r;
    pthread_t threads[nrMap+nrRed];
	int arg[nrMap+nrRed]; //nr mapperi;
    int N; //nr documente
    int E = nrRed+1; //nr exponenti;
    char docname[100];

    FILE *fi;
    fi = fopen("test0/test.txt", "r");
    fscanf(fi, "%d", &N);
    printf("%d\n", N);
    for(int i=0; i<=N; i++){
        fgets(docname, 100, fi);
        //printf("%s", docname);
    }
    fclose(fi);

    for(int i=0; i<nrMap+nrRed; i++){
        arg[i] = i;
        r = pthread_create(&threads[i], NULL, f, &arg[i]);

    }

    for(int i=0; i<nrMap+nrRed; i++){
        r = pthread_join(threads[i], NULL);

    }

    printf("\nMapperLists should look like this:\n");
    for(int i=0; i<nrMap; i++){
        for(int j=0; j<nrRed; j++){
            printf("{");
            SortedList inter = MapperLists[i][j];
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
    printf("\nReduced MapperLists should look like this:\n");
    for(int i=0; i<nrRed; i++){
        printf("{");
        SortedList inter = ReducerLists[i];
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
    pthread_exit(NULL);

	return 0;
}
