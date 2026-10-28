#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "include/colors.h"
#include "include/errors.h"
#include "include/debug.h"

typedef double listElem_t;
typedef ssize_t listIndex;
typedef int    err_t;

#define IS_POISON(val) isnan(val)
#define LIST_ELEM_FORMAT "%lg"

struct listMember {
    listElem_t value;

    listIndex next;
    listIndex prev;
};

struct listResources {
    size_t capacity;

    listMember * List;

    listIndex head;
    listIndex tail;

};

const size_t DEFAULT_CAPACITY = 10;
const listElem_t LIST_POISON  = NAN;
const char FREE_FLAG = -1;    

err_t ListCtor(listResources * linkedList);

err_t GetFreeCell(listResources * linkedList, listIndex * newIndex);

err_t PushFront(listResources * linkedList, listElem_t val);

void ListDump(FILE * stream, listResources * linkedList);
/*
err_t GetFreeCell(next);

err_t PushBack(val);
err_t PushFront(val);

err_t Erase(pos);

err_t Insert(pos, val);

*/


int main() {

    listResources linkedList = {};

    err_t prepareStat = ListCtor(&linkedList);
    if (prepareStat != ERR_OK) {
        printf("Error of constructor. Error code: %d\n", prepareStat);
        return 1;
    }

    for (int i = 100; i < 105; i++) {

        PushFront(&linkedList, i);
        ListDump(stdout, &linkedList);

    }

    return 0;
}

err_t ListCtor(listResources * linkedList) {

    ASSERT(linkedList);

    linkedList->capacity = DEFAULT_CAPACITY;

    listMember * newList = (listMember *)calloc(linkedList->capacity, sizeof(listMember));
    if (!newList) return ERR_CALLOC;
    linkedList->List = newList;

    for (size_t i = 0; i < linkedList->capacity; i++) {

        linkedList->List[i].value = LIST_POISON;
        linkedList->List[i].next  = -1;
        linkedList->List[i].prev  = -1;

    }

    linkedList->tail = 0;
    linkedList->head = 1;

    listMember initTail = {.value = 0, .next = linkedList->head, .prev = linkedList->tail};
    listMember initHead = {.value = 0, .next = linkedList->head, .prev = linkedList->tail};

    linkedList->List[linkedList->tail] = initTail;
    linkedList->List[linkedList->head] = initHead;

    // linkedList->List[linkedList->tail].next = linkedList->head;
    // linkedList->List[linkedList->tail].prev = linkedList->tail;

    // linkedList->List[linkedList->head].prev = linkedList->tail;
    // linkedList->List[linkedList->head].next = linkedList->head;

    // TODO: ListVerity

    return ERR_OK;
}

err_t PushFront(listResources * linkedList, listElem_t val) {

    ASSERT(linkedList);

    listIndex newIndex = -1;
    if (GetFreeCell(linkedList, &newIndex) != ERR_OK) {
        printf("Error of searching a free cell\n");
        return ERR_PUSHFRONT;
    }

    listIndex oldHead = linkedList->head;

    linkedList->List[oldHead].value = val;
    linkedList->List[oldHead].next  = newIndex;

    listMember newHead = {.value = 0, .next = newIndex, .prev = oldHead};

    linkedList->List[newIndex] = newHead;

    linkedList->head = newIndex;  // <-- rewrite head index

    return ERR_OK;
}

err_t GetFreeCell(listResources * linkedList, listIndex * newIndex) {

    ASSERT(linkedList);
    ASSERT(newIndex);

    // FIXME: make more effective algorithm
    for (size_t i = 0; i < linkedList->capacity; i++) {

        if (linkedList->List[i].next == FREE_FLAG &&
            linkedList->List[i].prev == FREE_FLAG)  {
            if (!IS_POISON(linkedList->List[i].value)) return ERR_GET_CELL;

            *newIndex = i;

            return ERR_OK;
        }
    }

    return ERR_GET_CELL;
}

/*

err_t PushBack(val) {

    listIndex newIndex = GetFreeCell(next);

    List[tail].value = val;
    List[tail].next  = newIndex;

    List[newIndex].next  = listResource.tail; <-- previous tail index
    List[newIndex].prev  = newIndex;          <-- pointer on its own
    List[newIndex].value = 0;                 <-- tail's default value
    listResource.tail    = newIndex;          <-- rewrite tail index

    return ERR_OK;

}

err_t Erase(pos) {

    List[pos].value = poison;

    List[List[pos].prev].next = List[pos].next;
    List[List[pos].next].prev = List[pos].prev;

    List[pos].next = -1;
    List[pos].prev = -1;
}
*/

/*
err_t PushAfter(val, pos) {

    listIndex newIndex = GetFreeCell;

    listIndex oldNext = List[pos].next;
    listIndex oldPrev = List[pos].prev;

    listMember newMember = {.value = val, .prev = pos};

    List[oldNext].prev   = newIndex;
    List[oldPrev].next = newIndex;

    List[newIndex] = newMember;

    ListVerify()

    return ERR_OK;
}
*/

void ListDump(FILE * stream, listResources * linkedList) {

    ASSERT(linkedList);
    ASSERT(stream);

    fprintf(stream, "_________________________________\n");
    fprintf(stream, "|    VALUE    | PREVIOUS | NEXT |\n");
    fprintf(stream, "---------------------------------\n");
    
    for (size_t i = 0; i < linkedList->capacity; i++) {

        char valueStr[13] = "";
        char prevStr [10] = "";
        char nextStr [6]  = "";

        snprintf(valueStr, 13, LIST_ELEM_FORMAT, linkedList->List[i].value);
        snprintf(prevStr, 10, "%zd", linkedList->List[i].prev);
        snprintf(nextStr, 6, "%zd", linkedList->List[i].next);

        fprintf(stream, "|%13s|%10s|%6s|\n", valueStr, prevStr, nextStr);

    }

    fflush(stream);
}