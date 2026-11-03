/*  TODO: err_t resize(up/down, List)
    TODO: system cell - another color
    TODO: head, tail, free elem in object graph

    TODO: arv flags for list dump
    
    TODO: GetFront()
    TODO: GetBack()
    TODO: bit code for the list verify

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "include/colors.h"
#include "include/errors.h"
#include "include/debug.h"

typedef double listElem_t;
typedef ssize_t listIndex;
typedef int    err_t;

#define IS_STAT_OK(status)  do {                                        \
                                if (status != ERR_OK) {                 \
                                    PRINT_DEBUG("ERROR: %d\n", status); \
                                    return status;                      \
                                }                                       \
                            } while (0)

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

const size_t     DEFAULT_CAPACITY = 10;
const listElem_t LIST_POISON      = NAN;
const ssize_t    FREE_FLAG        = -1;
const ssize_t    SYS_CELL_INDEX   = 0;

// TODO: get if from console
const char FOLDER_WITH_PICTURES[]      = "graphs";
const char FILE_SCRIPT_FOR_GRAPHVIZ[]  = "graph.dot";
const char DUMP_FILENAME[]             = "dump.html";

bool IsPoison(double number);

err_t ListCtor(listResources * linkedList);
err_t ListDtor(listResources * linkedList);

err_t GetFreeCell(listResources * linkedList, listIndex * newIndex);

err_t GetFront(const listResources * linkedList, listIndex * head);
err_t GetBack (const listResources * linkedList, listIndex * tail);

err_t PushFront(listResources * linkedList, listElem_t val);
err_t PushBack (listResources * linkedList, listElem_t val);

err_t PushAfter (listResources * linkedList, listElem_t value, listIndex position);
err_t PushBefore(listResources * linkedList, listElem_t value, listIndex position);
err_t Insert    (listResources * linkedList, listElem_t value, listIndex position);

err_t Erase(listResources * linkedList, listIndex position);

err_t CheckListErrors(const listResources * linkedList,
                      FILE * stream,
                      const char * funcname,
                      const int line);
    
err_t ListDump(const listResources * linkedList, size_t count, FILE * stream);
err_t PrepareHTML(const char * filename, FILE ** stream);

err_t GeneratePictureName(char ** pictureName, size_t count);
err_t GetPicturePath(const char * sourceFolder, const char * pictureName, char ** picturePath);
err_t GetObjectGraph(const listResources * linkedList, char * pictureName);
void  MakeGraphvizScript(FILE * stream, const listResources * linkedList);
void  InsertObjectGraph(FILE * stream, const char * picturePath);

void WriteTextDump(FILE * stream, const listResources * linkedList);

err_t ListVerity(const listResources * linkedList);

const char * GetErrorDescription(err_t errorCode);

int main() {

    listResources linkedList = {};

    err_t prepareStat = ListCtor(&linkedList);
    IS_STAT_OK(prepareStat);

    system("del /f /q \"graphs\\*.*\"");

    FILE * htmlStream = NULL;
    err_t htmlStat = PrepareHTML(DUMP_FILENAME, &htmlStream);
    IS_STAT_OK(htmlStat);

    ListDump(&linkedList, 0, htmlStream);

    for (int i = 100; i < 105; i++) {

        PushFront(&linkedList, i);

        ListDump(&linkedList, i, htmlStream);
    }

    PushBack(&linkedList, 111);
    ListDump(&linkedList, 222, htmlStream);
    
    PushBack(&linkedList, 112);
    ListDump(&linkedList, 223, htmlStream);

    PushAfter(&linkedList, 69, 1);
    ListDump(&linkedList, 1, htmlStream);
    PushAfter(&linkedList, 67, 1);
    ListDump(&linkedList, 2, htmlStream);
    
    Erase(&linkedList, 4);
    ListDump(&linkedList, 3, htmlStream);
    
    PushBefore(&linkedList, 666, 5);
    ListDump(&linkedList, 4, htmlStream);
    
    Insert(&linkedList, 999, 7);
    ListDump(&linkedList, 5, htmlStream);
    
    LIST_VERIFY(&linkedList, htmlStream);
    
    fclose(htmlStream);
    ListDtor(&linkedList);
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

    listMember initSysCell = {.value = LIST_POISON, .next = SYS_CELL_INDEX, .prev = SYS_CELL_INDEX};

    linkedList->List[linkedList->tail] = initSysCell;

    // TODO: ListVerity
    LIST_VERIFY(linkedList, stderr);

    return ERR_OK;
}

err_t PrepareHTML(const char * filename, FILE ** stream) {

    ASSERT(filename);

    FILE * htmlStream = fopen(filename, "w");
    if (htmlStream == NULL) {
        PRINT_DEBUG("html error\n");
        return ERR_HTML;
    }
    fprintf(htmlStream, "<pre>\n");
    
    *stream = htmlStream;

    return ERR_OK;
}

err_t PushFront(listResources * linkedList, listElem_t val) {

    ASSERT(linkedList);
    
    listIndex oldHead = linkedList->head;

    // TODO: check error
    PushAfter(linkedList, val, oldHead);

    linkedList->head = linkedList->List[oldHead].next;  // <-- rewrite head index

    LIST_VERIFY(linkedList, stderr);

    return ERR_OK;
}

err_t PushBack(listResources * linkedList, listElem_t val) {

    ASSERT(linkedList);

    listIndex oldTail = linkedList->tail;

    // TODO: check error
    PushBefore(linkedList, val, oldTail);

    linkedList->tail = linkedList->List[oldTail].prev;

    LIST_VERIFY(linkedList, stderr);

    return ERR_OK;
}

err_t GetFront(const listResources * linkedList, listIndex * head) {

    ASSERT(linkedList);
    ASSERT(head);

    listIndex headIndex = linkedList->head;

    if (headIndex < 0) return ERR_HEAD;

    *head = headIndex;

    return ERR_OK;
}

err_t GetBack(const listResources * linkedList, listIndex * tail) {

    ASSERT(linkedList);
    ASSERT(tail);

    listIndex tailIndex = linkedList->tail;

    if (tailIndex < 0) return ERR_TAIL;

    *tail = tailIndex;

    return ERR_OK;
}

err_t GetFreeCell(listResources * linkedList, listIndex * newIndex) {

    ASSERT(linkedList);
    ASSERT(newIndex);

    LIST_VERIFY(linkedList, stderr);

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

    LIST_VERIFY(linkedList, stderr);

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

    LIST_VERIFY(linkedList, stderr);

    return ERR_OK;

}

err_t PushBefore(listResources * linkedList, listElem_t value, listIndex position) {

    ASSERT(linkedList);

    // TODO: ListVerify
    return PushAfter(linkedList, value, linkedList->List[position].prev);
}

err_t Insert(listResources * linkedList, listElem_t value, listIndex position) {

    ASSERT(linkedList);

    return PushBefore(linkedList, value, linkedList->List[position].next);
}

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

    if (linkedList->size > linkedList->capacity)           return ERR_SIZE;
    if (!IsPoison(linkedList->List[SYS_CELL_INDEX].value)) return ERR_SYS_CELL;

    listIndex start = linkedList->tail;
    listIndex end   = linkedList->head;
    size_t maxCount = linkedList->size;

    listIndex currIndex = start;

    size_t count = 0;

    while (linkedList->List[currIndex].next != end) {
        if (count > maxCount) return ERR_DISRUPT;

        listIndex newNext = linkedList->List[currIndex].next;
        if (newNext != -1 && newNext < 0) return ERR_DISRUPT;

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

err_t ListDump(const listResources * linkedList, size_t count, FILE * stream) {

    ASSERT(linkedList);
    ASSERT(stream);

    fprintf(stream, "<h2><<<<<<<<<<<<< LIST DUMP %zu >>>>>>>>>>>>>>>></h2>\n", count);

    char * pictureName = NULL;
    char * picturePath = NULL;

    err_t nameStat = GeneratePictureName(&pictureName, count);
    IS_STAT_OK(nameStat);

    err_t pathStat = GetPicturePath(FOLDER_WITH_PICTURES, pictureName, &picturePath);
    IS_STAT_OK(pathStat);

    err_t getGraphStat = GetObjectGraph(linkedList, picturePath); // pictureName
    IS_STAT_OK(getGraphStat);

    InsertObjectGraph(stream, picturePath);

    fprintf(stream, "<p>");
    WriteTextDump(stream, linkedList);
    LIST_VERIFY(linkedList, stream);

    return ERR_OK;
}

err_t GeneratePictureName(char ** pictureName, size_t count) {

    ASSERT(pictureName);

    //! example of picture's name: 20261102_2020-1 - the 2nd of November, 20:20, 1st picture
    size_t nameLength = 20 + (int)(count / 10) + 1;
    char newName[nameLength] = "";

    time_t mytime = time(NULL);
    struct tm * now = localtime(&mytime);

    int printStat = snprintf(newName, nameLength, "%d_%dh%dm-%zu.png",
                             now->tm_yday, now->tm_hour,
                             now->tm_min, count);
    
    if (printStat > nameLength || printStat <= 0){
        PRINT_DEBUG("ERROR: %d", printStat);
        return ERR_SNPRINTF;
    }
    *pictureName = strdup(newName);
    if (*pictureName == NULL) return ERR_MEMORY;

    return ERR_OK;
}

err_t GetPicturePath(const char * sourceFolder, const char * pictureName, char ** picturePath) {

    ASSERT(pictureName);
    ASSERT(picturePath);
    ASSERT(sourceFolder);

    const size_t pathLength = strlen(sourceFolder) + strlen(pictureName) + 2;
    char bufferPath[pathLength] = "";

    err_t printVal = snprintf(bufferPath, pathLength, "%s/%s", sourceFolder, pictureName);
    if (printVal <= 0 || printVal > pathLength) return ERR_SNPRINTF;

    *picturePath = strdup(bufferPath);
    if (*picturePath == NULL) return ERR_MEMORY;

    return ERR_OK;
}

err_t GetObjectGraph(const listResources * linkedList, char * pictureName) {

    ASSERT(linkedList);
    ASSERT(pictureName);

    FILE * stream = fopen(FILE_SCRIPT_FOR_GRAPHVIZ, "w");
    if (stream == NULL) return ERR_FILEOPEN;

    MakeGraphvizScript(stream, linkedList);
    fclose(stream);

    size_t commandLen = 20 + strlen(FILE_SCRIPT_FOR_GRAPHVIZ) + strlen(pictureName);
    char command[commandLen] = "";
    int printStat = snprintf(command, commandLen, "dot %s -Tpng -o %s", FILE_SCRIPT_FOR_GRAPHVIZ, pictureName);
    if (printStat > commandLen || printStat <= 0) return ERR_SNPRINTF;

    system("cd graphs");
    int sysStat = system(command);
    if (sysStat == -1) return ERR_SYSTEM;

    return ERR_OK;      
}

void MakeGraphvizScript(FILE * stream, const listResources * linkedList) {

    ASSERT(stream);
    ASSERT(linkedList);

    fprintf(stream, "digraph {\n"
                              "rankdir=LR;\n"
                              "ranksep=0.8;\n");
    fprintf(stream, "node [shape=record, fontname = \"Arial\", style=filled, fillcolor=\"#EEF4FF\"];\n"
                    "edge [fontname = \"Arial\", arrowsize=0.8;];\n\n");
    
    for (size_t i = 0; i < linkedList->capacity; i++) {

        fprintf(stream, "n%zu [label=\"{<i> index = %zu} | {<prev> prev = %zd | <data> %lg | <next> next = %zd}\"];\n",
                            i, i, linkedList->List[i].prev, linkedList->List[i].value, linkedList->List[i].next);

    }

    fprintf(stream, "nHead [shape=Mrecord; style=\"filled\"; fillcolor=\"#952a2a\"; label = \"{<head> HEAD}\"];\n");
    fprintf(stream, "nTail [shape=Mrecord; style=\"filled\"; fillcolor=\"#952a2a\"; label = \"{<tail> TAIL}\"];\n");

    for (size_t i = 0; i < linkedList->capacity; i++) {
        listIndex next = linkedList->List[i].next;
        listIndex prev = linkedList->List[i].prev;

        if (next != -1) fprintf(stream, "n%zu:next:e -> n%zu:prev:w [color=\"#2563EB\"];\n", i, next);
        if (prev != -1) fprintf(stream, "n%zu:prev:s -> n%zu:data:s [color=\"#DC2626\"];\n", i, prev);

        if (i == linkedList->head) fprintf(stream, "nHead -> n%zu:i:n [color=\"#DC2626\"];\n", i);
        if (i == linkedList->tail) fprintf(stream, "nTail -> n%zu:i:n [color=\"#DC2626\"];\n", i);

    }

    fprintf(stream, "}\n");

}

void InsertObjectGraph(FILE * stream, const char * picturePath) {

    ASSERT(stream);
    ASSERT(picturePath);

    fprintf(stream, "<img src=\"%s\" style=\"width: 60%;\">\n", picturePath);

}

void WriteTextDump(FILE * stream, const listResources * linkedList) {

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

        if (i == SYS_CELL_INDEX)        snprintf(indexStr, 11, "[SYS] %zu", i);
        else if (i == linkedList->head) snprintf(indexStr, 11, "[H] %zu", i);
        else if (i == linkedList->tail) snprintf(indexStr, 11, "[T] %zu", i);
        else                            snprintf(indexStr, 11, "%zu", i);

        snprintf(valueStr, 13, LIST_ELEM_FORMAT, linkedList->List[i].value);
        snprintf(prevStr, 10, "%zd", linkedList->List[i].prev);
        snprintf(nextStr, 11, "%zd", linkedList->List[i].next);

        fprintf(stream, "|%11s|%13s|%10s|%11s|\n", indexStr, valueStr, prevStr, nextStr);

    }
    fprintf(stream, "__________________________________________________\n");
    fprintf(stream, ">>> CAPACITY:   %zd\n", linkedList->capacity);
    fprintf(stream, ">>> SIZE:       %zd\n", linkedList->size);
    fprintf(stream, ">>> FREE CELLS: %zd\n", linkedList->capacity - linkedList->size - 1);
    fprintf(stream, "__________________________________________________\n");

    fflush(stream);
}

bool IsPoison(double number) {

    if (isnan(number)) return true;

    return false;
}