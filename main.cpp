// TODO: resize(up/down, List)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "include/colors.h"
#include "include/errors.h"
#include "include/debug.h"

typedef double listElem_t;
typedef ssize_t listIndex;
typedef int    err_t;

#define LIST_ELEM_FORMAT "%lg"

#ifdef VERIFY
#define LIST_VERIFY(linkedList, stream) CheckListErrors(linkedList, stream, __func__, __LINE__)
#else
#define LIST_VERIFY(linkedList, stream) (void)0
#endif

struct listMember {
    listElem_t value;

    listIndex next;
    listIndex prev;
};

struct listResources {
    size_t capacity;
    size_t size;

    listMember * List;

    listIndex head;
    listIndex tail;
};

const size_t DEFAULT_CAPACITY = 10;
const listElem_t LIST_POISON  = NAN;
const ssize_t FREE_FLAG = -1;

bool IsPoison(double number);

err_t ListCtor(listResources * linkedList);
err_t ListDtor(listResources * linkedList);

err_t GetFreeCell(listResources * linkedList, listIndex * newIndex);

err_t PushFront(listResources * linkedList, listElem_t val);

err_t PushAfter (listResources * linkedList, listElem_t value, listIndex position);
err_t PushBefore(listResources * linkedList, listElem_t value, listIndex position);
err_t Insert    (listResources * linkedList, listElem_t value, listIndex position);

err_t Erase(listResources * linkedList, listIndex position);

err_t CheckListErrors(const listResources * linkedList,
                      FILE * stream,
                      const char * funcname,
                      const int line);
    
void ListDump(FILE * stream, const listResources * linkedList);

err_t ListVerity(const listResources * linkedList);

const char * GetErrorDescription(err_t errorCode);

/*

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
        ListDump(stderr, &linkedList);

    }

    PushAfter(&linkedList, 69, 1);
    ListDump(stderr, &linkedList);
    PushAfter(&linkedList, 67, 1);
    ListDump(stderr, &linkedList);

    Erase(&linkedList, 4);
    ListDump(stderr, &linkedList);

    PushBefore(&linkedList, 666, 5);
    ListDump(stderr, &linkedList);

    Insert(&linkedList, 999, 7);
    ListDump(stderr, &linkedList);

    LIST_VERIFY(&linkedList, stderr);

    return 0;
}

err_t ListCtor(listResources * linkedList) {

    ASSERT(linkedList);

    linkedList->capacity = DEFAULT_CAPACITY;
    linkedList->size     = 0;

    listMember * newList = (listMember *)calloc(linkedList->capacity, sizeof(listMember));
    if (!newList) return ERR_CALLOC;
    linkedList->List = newList;

    for (size_t i = 0; i < linkedList->capacity; i++) {

        linkedList->List[i].value = LIST_POISON;
        linkedList->List[i].next  = FREE_FLAG;
        linkedList->List[i].prev  = FREE_FLAG;

    }

    linkedList->tail = 0;
    linkedList->head = 0;

    listMember initTail = {.value = 0, .next = linkedList->head, .prev = linkedList->tail};

    linkedList->List[linkedList->tail] = initTail;

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
    linkedList->size++;

    return ERR_OK;
}

err_t GetFreeCell(listResources * linkedList, listIndex * newIndex) {

    ASSERT(linkedList);
    ASSERT(newIndex);

    // FIXME: make more effective algorithm
    for (size_t i = 0; i < linkedList->capacity; i++) {

        if (linkedList->List[i].next == FREE_FLAG &&
            linkedList->List[i].prev == FREE_FLAG)  {
            if (!IsPoison(linkedList->List[i].value)) return ERR_GET_CELL;

            *newIndex = i;

            return ERR_OK;
        }
    }

    return ERR_GET_CELL;
}

/*
TODO:
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
*/

err_t Erase(listResources * linkedList, listIndex position) {

    ASSERT(linkedList);
    if (IsPoison(linkedList->List[position].value)) return ERR_ERASE;

    linkedList->List[position].value = LIST_POISON;

    linkedList->List[linkedList->List[position].prev].next = linkedList->List[position].next;
    linkedList->List[linkedList->List[position].next].prev = linkedList->List[position].prev;

    linkedList->List[position].next = FREE_FLAG;
    linkedList->List[position].prev = FREE_FLAG;

    linkedList->size--;

    return ERR_OK;

}

err_t PushAfter(listResources * linkedList, listElem_t value, listIndex position) {

    ASSERT(linkedList);
    
    listIndex newIndex = -1;
    if (GetFreeCell(linkedList, &newIndex) != ERR_OK) {
        printf("Error of searching a free cell\n");
        return ERR_PUSHFRONT;
    }

    listIndex oldNext = linkedList->List[position].next;

    listMember newMember = {.value = value, .next = oldNext, .prev = position};

    linkedList->List[position].next = newIndex;
    linkedList->List[oldNext].prev  = newIndex;

    linkedList->List[newIndex] = newMember;
    linkedList->size++;

    return ERR_OK;

}

err_t PushBefore(listResources * linkedList, listElem_t value, listIndex position) {

    ASSERT(linkedList);

    listIndex newIndex = -1;
    if (GetFreeCell(linkedList, &newIndex) != ERR_OK) {
        printf("Error of searching a free cell\n");
        return ERR_PUSHBEFORE;
    }

    listIndex oldPrev = linkedList->List[position].prev;

    listMember newMember = {.value = value, .next = position, .prev = oldPrev};

    linkedList->List[position].prev = newIndex;
    linkedList->List[oldPrev].next  = newIndex;

    linkedList->List[newIndex] = newMember;
    linkedList->size++;

    // TODO: ListVerify

    return ERR_OK;
}

err_t Insert(listResources * linkedList, listElem_t value, listIndex position) {

    ASSERT(linkedList);

    return PushBefore(linkedList, value, linkedList->List[position].next);
}

// TODO: GetBegin
// TODO: GetBack

err_t ListDtor(listResources * linkedList) {

    free(linkedList->List);
    linkedList->tail     = 0;
    linkedList->head     = 0;
    linkedList->capacity = 0;
    linkedList->size     = 0;

    return ERR_OK;

}

err_t CheckListErrors(  const listResources * linkedList,
                        FILE * stream,
                        const char * funcname,
                        const int line) {

    ASSERT(linkedList);
    ASSERT(stream);
    ASSERT(funcname);
    ASSERT(line);

    err_t listStatus = ListVerity(linkedList);

    if (listStatus == ERR_OK) return ERR_OK;

    fprintf(stream, "  <<<<< LIST VERIFY >>>>>   \n");
    fprintf(stream, "     ! ERROR DETECTED !     \n");
    fprintf(stream, "Function: %s:%d\n", funcname, line);
    fprintf(stream, "Description: %s\n", GetErrorDescription(listStatus));

    fflush(stream);
    return listStatus;
}

err_t ListVerity(const listResources * linkedList) {

    // TODO: more if

    ASSERT(linkedList);

    if (linkedList->size > linkedList->capacity) return ERR_SIZE;

    listIndex start = linkedList->tail;
    listIndex end   = linkedList->head;
    size_t maxCount = linkedList->size;

    listIndex currIndex = start;

    size_t count = 0;

    while (linkedList->List[currIndex].next != end) {
        if (count > maxCount) return ERR_DISRUPT;

        currIndex = linkedList->List[currIndex].next;
        count++;
    }

    return ERR_OK;

}

const char * GetErrorDescription(err_t errorCode) {

    switch(errorCode) {

        case (ERR_OK):      return "All good";
        case (ERR_SIZE):    return "Incorrect size"; // FIXME: write good description
        case (ERR_DISRUPT): return "Disruption of indexes in the list";

        // TODO: description
        default: break;

    }

    return "There is no description of this error";

}

void ListDump(FILE * stream, const listResources * linkedList) {

    ASSERT(linkedList);
    ASSERT(stream);

    fprintf(stream, "\n              <<<<< LIST DUMP >>>>>             \n");
    fprintf(stream, "__________________________________________________\n");
    fprintf(stream, "|   INDEX   |    VALUE    | PREVIOUS |    NEXT   |\n");
    fprintf(stream, "--------------------------------------------------\n");
    
    for (size_t i = 0; i < linkedList->capacity; i++) {
        
        char indexStr[11]  = "";
        char valueStr[13] = "";
        char prevStr [10] = "";
        char nextStr [11]  = "";

        if (i == linkedList->head)      snprintf(indexStr, 11, "[H] %zu", i);
        else if (i == linkedList->tail) snprintf(indexStr, 11, "[T] %zu", i);
        else                            snprintf(indexStr, 11, "%zu", i);

        snprintf(valueStr, 13, LIST_ELEM_FORMAT, linkedList->List[i].value);
        snprintf(prevStr, 10, "%zd", linkedList->List[i].prev);
        snprintf(nextStr, 11, "%zd", linkedList->List[i].next);

        fprintf(stream, "|%11s|%13s|%10s|%11s|\n", indexStr, valueStr, prevStr, nextStr);

    }
    fprintf(stream, "__________________________________________________\n");
    fprintf(stream, ">>> CAPACITY: %zu\n", linkedList->capacity);
    fprintf(stream, ">>> SIZE:     %zu\n", linkedList->size);
    fprintf(stream, "__________________________________________________\n");

    fflush(stream);
}

bool IsPoison(double number) {

    if (isnan(number)) return true;

    return false;
}