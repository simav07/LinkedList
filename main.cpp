#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "colors.h"

typedef double listElem_t;
typedef size_t listIndex;
typedef int    err_t;

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

/*
err_t PrepareResource;

err_t GetFreeCell(next);

err_t PushBack(val);
err_t PushFront(val);

err_t Erase(pos);

err_t Insert(pos, val);


*/

int main() {

}

/*
err_t PushFront(val) {

    listIndex newIndex = GetFreeCell(next);

    List[head].value = val;
    List[head].next  = newIndex;

    List[newIndex].prev  = listResource.head; <-- previous head index
    List[newIndex].next  = newIndex;          <-- pointer on its own
    List[newIndex].value = 0;                 <-- head's default value
    listResource.head    = newIndex;          <-- rewrite head index

    return ERR_OK;

}

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