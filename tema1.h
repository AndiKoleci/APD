#ifndef SORTED_LIST_H
#define SORTED_LIST_H

#include <stdlib.h>
#include <assert.h>

typedef struct node
{
    unsigned int value;
    struct node *next;
} TNode, *SortedList;

typedef struct
{
    int nrMap;
    int nrRed;
    SortedList **MapperLists;
    SortedList *ReducerLists;
    char **fileStrings;
    int fileCounter;
    int fileCounterMax;
    pthread_barrier_t barrier;
    pthread_mutex_t mutex;
}GlobalStruct;

typedef struct{
    int thread_id;
    GlobalStruct *data;
}ThreadStruct;

SortedList create(int value)
{

    SortedList list = malloc(sizeof(TNode));
    list->value = value;
    list->next = NULL;
    return list;
}

SortedList insert(SortedList list, int element)
{
    SortedList copy = list;
    SortedList inter = create(element);
    if (list == NULL)
    {
        return inter;
    }
    if (element < list->value)
    {
        inter->next = list;
        return inter;
    }
    while (copy->next != NULL && copy->next->value < element)
    {
        copy = copy->next;
    }
    inter->next = copy->next;
    copy->next = inter;

    return list;
}

SortedList freeSortedList(SortedList list)
{
    SortedList copy = list;
    SortedList node;
    if(list==NULL) return NULL;
    while (copy->next != NULL)
    {
        node = copy->next;
        copy->next = copy->next->next;
        free(node);
    }
    free(list);
    return NULL;
}

#endif