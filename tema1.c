#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#include "tema1.h"

int binarySearch(int l, int r, int x, int power)
{
    int mid;
    unsigned long long nr;

    mid = (l + r) / 2;
    nr = pow(mid, power);
    if(nr == x) return 1;
    else if(l >= r) return 0;
    else if(nr > x) return binarySearch(l, mid, x, power);
    else if(nr < x) return binarySearch(mid + 1, r, x, power);
    
}

void *f(void *arg)
{	
    ThreadStruct *threadData = arg;
    int thread_id = threadData->thread_id;
    GlobalStruct *g = threadData->data;

    if(thread_id < g->nrMap){ 
        while(g->fileCounter < g->fileCounterMax){
            int x;
            int times;
            char filename1[100];
            char num[31];
            int opened = 0;

            pthread_mutex_lock(&g->mutex);
            if (g->fileCounter < g->fileCounterMax) {
                g->fileCounter++;
                opened = 1;
                sprintf(filename1, "%s", g->fileStrings[g->fileCounter - 1]);
            }
            pthread_mutex_unlock(&g->mutex);

            if(opened==1){

                FILE *fi1;
                fi1 = fopen(filename1, "r");
                fgets(num, 31, fi1);
                times = atoi(num);
                for(int i = 0; i < times; i++){
                    fgets(num, 31, fi1);
                    x = atoi(num);

                    for(int j=2; j <= g->nrRed + 1; j++)
                    {
                        if(binarySearch(2, sqrt(x), x, j) == 1){
                            g->MapperLists[thread_id][j - 2] = 
                                insert(g->MapperLists[thread_id][j - 2], x);
                        }
                    }
                }

                fclose(fi1);
            }
        }
    }

    pthread_barrier_wait(&g->barrier);

    if(thread_id >= g->nrMap){

        for(int i = 0; i < g->nrMap; i++){
            SortedList inter = g->MapperLists[i][thread_id - g->nrMap];
            while(inter != NULL){
                g->ReducerLists[thread_id - g->nrMap] = 
                    insert(g->ReducerLists[thread_id - g->nrMap], inter->value);
                inter=inter->next;
            }
        }

        SortedList inter2 = g->ReducerLists[thread_id - g->nrMap];
        int count = 1;
        if(inter2 != NULL) {
            while (inter2->next != NULL) {
                if (inter2->value != inter2->next->value) {
                    count++;
                }
                inter2 = inter2->next;
            }
        }
        else {
            count = 0;
        }
        FILE *final;
        char filenamefinal[100];

        sprintf(filenamefinal, "out%d.txt", thread_id - g->nrMap+2);
        
        final=fopen(filenamefinal, "w");
        fprintf(final, "%d", count);
        printf("%d\n", count);
        fclose(final);
        
    }	
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    int M = atoi(argv[1]);
    int R = atoi(argv[2]);

    SortedList **mapperLists = calloc(M, sizeof(SortedList *));
    for(int i = 0; i < M; i++){
        mapperLists[i] = calloc (R, sizeof(SortedList));
    }

    SortedList *reducerLists = calloc(R, sizeof(SortedList));

    int r;
    char Nread[10];
    int N;
    pthread_t threads[M + R];
	ThreadStruct arg[M + R];
    char initFile[100];
    snprintf(initFile, 30, "%s", argv[3]);

    FILE *fi;
    fi = fopen(initFile, "r");
    fgets(Nread, 10, fi);
    N = atoi(Nread);

    char** fileStrings = calloc(N, sizeof(char *));
    for(int i = 0; i < N; i++){
        fileStrings[i] = calloc(100, sizeof(char));
    }

    GlobalStruct data = {
        .nrMap = M,
        .nrRed = R,
        .fileCounterMax = N,
        .fileStrings = fileStrings,
        .MapperLists = mapperLists,
        .ReducerLists = reducerLists
    };

    pthread_barrier_init(&data.barrier, NULL, M + R);
    pthread_mutex_init(&data.mutex, NULL);

    
    for(int i = 0; i < N; i++){
        fscanf(fi, "%s\n", data.fileStrings[i]);
    }

    for(int i = 0; i < M + R; i++){
        arg[i].thread_id = i;
        arg[i].data = &data;
        r = pthread_create(&threads[i], NULL, f, &arg[i]);

        if (r) {
            printf("Eroare la crearea thread-ului %d\n", i);
            exit(-1);
        }
    }

    for(int i = 0; i < M + R; i++) {
        r = pthread_join(threads[i], NULL);

        if (r) {
            printf("Eroare la asteptarea thread-ului %d\n", i);
            exit(-1);
        }
    }

    for(int i = 0; i < N; i++){
        free(fileStrings[i]);
    }
    free(fileStrings);

    for(int i = 0; i < M; i++){
        for(int j = 0; j < R; j++){
            freeSortedList(mapperLists[i][j]);
        }
        free(mapperLists[i]);
    }
    free(mapperLists);

    for(int i = 0; i < R; i++){
        freeSortedList(reducerLists[i]);
    }
    free(reducerLists);

    pthread_barrier_destroy(&data.barrier);
    pthread_mutex_destroy(&data.mutex);
    pthread_exit(NULL);

}
