/*

TODO: error descriptions

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <direct.h>
#include <process.h>

#include "include/colors.h"
#include "include/errors.h"
#include "include/debug.h"

typedef double  listElem_t;
typedef ssize_t listIndex;
typedef int     err_t;

#define IS_STAT_OK(status)  do {                                        \
                                if (status != ERR_OK) {                 \
                                    PRINT_DEBUG("ERROR: %d\n", status); \
                                    return status;                      \
                                }                                       \
                            } while (0)

#define LIST_ELEM_FORMAT "%lg"

#ifdef VERIFY
#define LIST_VERIFY(linkedList, stream) do { \
                                            err_t errStat = CheckListErrors(linkedList, stream, __func__, __LINE__); \
                                            if (errStat != ERR_OK) return errStat;                                   \
                                        } while (0)
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

    listIndex freeCell;
};

const size_t     DEFAULT_CAPACITY = 10;
const listElem_t LIST_POISON      = NAN;
const ssize_t    FREE_FLAG        = -1;
const ssize_t    LAST_FREE_INDEX  = -1;
const ssize_t    SYS_CELL_INDEX   = 0;
const size_t     MAX_FILENAME_LEN = 80;

char pictureFolder[MAX_FILENAME_LEN] = "graphs";

bool IsPoison(double number);

err_t ListCtor(listResources * linkedList);
err_t ListDtor(listResources * linkedList);
err_t ListResize(listResources * linkedList, size_t newCapacity);

err_t GetFreeCell(listResources * linkedList, listIndex * newIndex);

err_t GetFront(const listResources * linkedList, listIndex * head);
err_t GetBack (const listResources * linkedList, listIndex * tail);
err_t GetSize (const listResources * linkedList, size_t * size);

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
    
err_t ListDump   (const listResources * linkedList, /*size_t count,*/ FILE * stream);
err_t PrepareHTML(FILE ** stream);

err_t GeneratePictureName(char ** pictureName, size_t count);
err_t GetPicturePath     (const char * sourceFolder, const char * pictureName, char ** picturePath);
err_t GetObjectGraph     (const listResources * linkedList, char * pictureName);
void  WriteGraphLinks    (FILE * stream, const listResources * linkedList, listIndex i);
void  MakeGraphvizScript (FILE * stream, const listResources * linkedList);
void  InsertObjectGraph  (FILE * stream, const char * picturePath);

void WriteTextDump(FILE * stream, const listResources * linkedList);

err_t ListVerity(const listResources * linkedList);

const char * GetErrorDescription(err_t errorCode);

int main() {

    listResources linkedList = {};

    err_t prepareStat = ListCtor(&linkedList);
    IS_STAT_OK(prepareStat);

    FILE * htmlStream = NULL;
    err_t htmlStat = PrepareHTML(&htmlStream);
    IS_STAT_OK(htmlStat);

    ListDump(&linkedList, htmlStream);

    PushFront(&linkedList, 100);
    ListDump(&linkedList, htmlStream);
    PushFront(&linkedList, 200);
    ListDump(&linkedList, htmlStream);
    PushFront(&linkedList, 300);
    ListDump(&linkedList, htmlStream);

    PushBack(&linkedList, 400);
    ListDump(&linkedList, htmlStream);

    Erase(&linkedList, 3);
    ListDump(&linkedList, htmlStream);
    
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
        linkedList->List[i].prev = FREE_FLAG;

        if (i == linkedList->capacity - 1) {
            linkedList->List[i].next = FREE_FLAG;
            break;
        }
        linkedList->List[i].next = i + 1;
    }

    linkedList->tail = SYS_CELL_INDEX;
    linkedList->head = SYS_CELL_INDEX;

    linkedList->freeCell = SYS_CELL_INDEX + 1;

    listMember initSysCell = {.value = LIST_POISON, .next = SYS_CELL_INDEX, .prev = SYS_CELL_INDEX};

    linkedList->List[SYS_CELL_INDEX] = initSysCell;

    LIST_VERIFY(linkedList, stderr);

    return ERR_OK;
}

err_t PrepareHTML(FILE ** stream) {

    ASSERT(stream);
    _mkdir("html");
    _mkdir("graphs");

    time_t now = time(NULL);
    struct tm * date = localtime(&now);
    if (date == NULL) return ERR_HTML;

    char reportName[64] = "";
    snprintf(reportName, sizeof(reportName), "%04d%02d%02d_%02d%02d_%02d%02d-%d",
             date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
             date->tm_hour, date->tm_min, date->tm_min, date->tm_sec, _getpid());
    snprintf(pictureFolder, sizeof(pictureFolder), "graphs/%s", reportName);
    if (_mkdir(pictureFolder) != 0) return ERR_HTML;

    char filename[80] = "";
    snprintf(filename, sizeof(filename), "html/%s.html", reportName);
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
    
    err_t pushStat = PushBefore(linkedList, val, SYS_CELL_INDEX);
    IS_STAT_OK(pushStat);

    err_t getFStat = GetFront(linkedList, &linkedList->head);
    IS_STAT_OK(getFStat);

    LIST_VERIFY(linkedList, stderr);

    return ERR_OK;
}

err_t PushBack(listResources * linkedList, listElem_t val) {

    ASSERT(linkedList);

    err_t pushStat = PushAfter(linkedList, val, SYS_CELL_INDEX);
    IS_STAT_OK(pushStat);

    err_t getBStat = GetBack(linkedList, &linkedList->tail);
    IS_STAT_OK(getBStat);

    LIST_VERIFY(linkedList, stderr);

    return ERR_OK;
}

err_t GetFront(const listResources * linkedList, listIndex * head) {

    ASSERT(linkedList);
    ASSERT(head);

    listIndex headIndex = linkedList->List[SYS_CELL_INDEX].prev;

    if (headIndex < 0) return ERR_HEAD;

    *head = headIndex;

    return ERR_OK;
}

err_t GetBack(const listResources * linkedList, listIndex * tail) {

    ASSERT(linkedList);
    ASSERT(tail);

    listIndex tailIndex = linkedList->List[SYS_CELL_INDEX].next;

    if (tailIndex < 0) return ERR_TAIL;

    *tail = tailIndex;

    return ERR_OK;
}

err_t GetSize(const listResources * linkedList, size_t * size) {

    ASSERT(linkedList);
    ASSERT(size);

    LIST_VERIFY(linkedList, stderr);

    *size = linkedList->size;

    return ERR_OK;
}

err_t ListResize(listResources * linkedList, size_t newCapacity) {

    size_t oldCapacity = linkedList->capacity;

    for (size_t i = newCapacity; i < oldCapacity; i++) {
        if (linkedList->List[i].prev != FREE_FLAG) return ERR_OK;
    }

    listMember * newList = (listMember *)realloc(linkedList->List, newCapacity * sizeof(listMember));
    if (!newList) return ERR_MEMORY;

    linkedList->List = newList;
    linkedList->capacity = newCapacity;

    if (newCapacity > oldCapacity) {
        
        for (size_t i = oldCapacity; i < newCapacity; i++) {
            newList[i].value = LIST_POISON;
            newList[i].prev  = FREE_FLAG;
            newList[i].next  = i + 1;
        }

        newList[newCapacity - 1].next = linkedList->freeCell;
        linkedList->freeCell = oldCapacity;
    }

    else {

        linkedList->freeCell = LAST_FREE_INDEX;
        for (size_t i = newCapacity - 1; i > 0; i--) {
            if (newList[i].prev != FREE_FLAG) continue;
            newList[i].next = linkedList->freeCell;
            linkedList->freeCell = i;
        }
    }
    return ERR_OK;
}

err_t GetFreeCell(listResources * linkedList, listIndex * newIndex) {

    ASSERT(linkedList);
    ASSERT(newIndex);

    LIST_VERIFY(linkedList, stderr);

    if (linkedList->freeCell == LAST_FREE_INDEX) {
        if (linkedList->size != linkedList->capacity - 1) return ERR_FREE_INDEX;
        if (linkedList->capacity > SIZE_MAX / sizeof(listMember) / 2) return ERR_MEMORY;
        err_t resizeStat = ListResize(linkedList, linkedList->capacity * 2);
        IS_STAT_OK(resizeStat);
    }
    listIndex freeIndex = linkedList->freeCell;

    *newIndex = freeIndex;

    listIndex newFreeIndex = linkedList->List[freeIndex].next;
    if (newFreeIndex <= 0 && newFreeIndex != LAST_FREE_INDEX) return ERR_FREE_INDEX;
    linkedList->freeCell = newFreeIndex;

    return ERR_OK;
}

err_t Erase(listResources * linkedList, listIndex position) {

    ASSERT(linkedList);
    if (IsPoison(linkedList->List[position].value)) return ERR_ERASE;

    linkedList->List[position].value = LIST_POISON;

    linkedList->List[linkedList->List[position].prev].next = linkedList->List[position].next;
    linkedList->List[linkedList->List[position].next].prev = linkedList->List[position].prev;

    linkedList->List[position].prev = FREE_FLAG;
    linkedList->List[position].next = linkedList->freeCell;
    linkedList->freeCell = position;

    linkedList->size--;

    if (linkedList->capacity > DEFAULT_CAPACITY && linkedList->size < linkedList->capacity / 4) {
        ListResize(linkedList, linkedList->capacity / 2);
    }

    LIST_VERIFY(linkedList, stderr);

    return ERR_OK;
}

err_t PushAfter(listResources * linkedList, listElem_t value, listIndex position) {

    ASSERT(linkedList);

    LIST_VERIFY(linkedList, stderr);
    
    listIndex newIndex = -1;
    err_t cellStat = GetFreeCell(linkedList, &newIndex);
    IS_STAT_OK(cellStat);

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
    LIST_VERIFY(linkedList, stderr);
    
    return PushAfter(linkedList, value, linkedList->List[position].prev);
}

err_t Insert(listResources * linkedList, listElem_t value, listIndex position) {

    ASSERT(linkedList);
    LIST_VERIFY(linkedList, stderr);

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
    
    listIndex start     = SYS_CELL_INDEX;
    size_t    size      = linkedList->size;
    size_t    capacity  = linkedList->capacity;
    listIndex end       = 0;
    listIndex freeIndex = linkedList->freeCell;
    err_t     getFStat  = GetFront(linkedList, &end);
    IS_STAT_OK(getFStat);

    if (size >= capacity) return ERR_SIZE;
    if (freeIndex != -1 && freeIndex <= 0) return ERR_FREE_INDEX;
    if (!IsPoison(linkedList->List[SYS_CELL_INDEX].value)) return ERR_SYS_CELL;


    listIndex currIndex = start;

    size_t count = 0;

    while (linkedList->List[currIndex].next != start) {
        if (count > size) return ERR_DISRUPT;

        listIndex newNext = linkedList->List[currIndex].next;

        if (!(newNext >= 0 && newNext < capacity)) return ERR_DISRUPT;

        currIndex = linkedList->List[currIndex].next;
        count++;
    }

    if (count != size) return ERR_DISRUPT;

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

err_t ListDump(const listResources * linkedList, /* size_t count,*/ FILE * stream) {

    ASSERT(linkedList);
    ASSERT(stream);

    static size_t nextDump = 0;
    size_t count = nextDump++;

    fprintf(stream, "<h2><<<<<<<<<<<<< LIST DUMP %zu >>>>>>>>>>>>>>>></h2>\n", count);

    char * pictureName = NULL;
    char * picturePath = NULL;

    err_t nameStat = GeneratePictureName(&pictureName, count);
    IS_STAT_OK(nameStat);

    err_t pathStat = GetPicturePath(pictureFolder, pictureName, &picturePath);
    IS_STAT_OK(pathStat);

    err_t getGraphStat = GetObjectGraph(linkedList, picturePath);
    IS_STAT_OK(getGraphStat);

    InsertObjectGraph(stream, picturePath);

    fprintf(stream, "<p>");
    WriteTextDump(stream, linkedList);
    LIST_VERIFY(linkedList, stream);

    return ERR_OK;
}

err_t GeneratePictureName(char ** pictureName, size_t count) {

    ASSERT(pictureName);

    char newName[MAX_FILENAME_LEN] = "";
    time_t mytime = time(NULL);
    struct tm * now = localtime(&mytime);

    int printStat = snprintf(newName, sizeof(newName), "%04d%02d%02d_%02d%02d_%02d%02d-%zu.png",
                             now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                             now->tm_hour, now->tm_min, now->tm_min, now->tm_sec, count);
    
    if (printStat <= 0 || (size_t)printStat >= sizeof(newName)){
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

    char scriptPath[96];
    snprintf(scriptPath, sizeof(scriptPath), "%s/graph.dot", pictureFolder);
    FILE * stream = fopen(scriptPath, "w");
    if (stream == NULL) return ERR_FILEOPEN;

    MakeGraphvizScript(stream, linkedList);
    fclose(stream);

    size_t commandLen = 20 + strlen(scriptPath) + strlen(pictureName);
    char command[commandLen] = "";
    int printStat = snprintf(command, commandLen, "dot %s -Tpng -o %s", scriptPath, pictureName);
    if (printStat > commandLen || printStat <= 0) return ERR_SNPRINTF;

    int sysStat = system(command);
    if (sysStat != 0) return ERR_SYSTEM;

    return ERR_OK;      
}

void WriteGraphLinks(FILE * stream, const listResources * linkedList, listIndex i) {

    ASSERT(stream);
    ASSERT(linkedList);

    listIndex next = linkedList->List[i].next;
    listIndex prev = linkedList->List[i].prev;
    listIndex back = linkedList->capacity;

    if (next != FREE_FLAG) {
        if (next < 0 || next >= back) {
            fprintf(stream, "n%zd -> \"invalid %zd\" [color=red];\n", i, next);
        } else {
            listIndex to = (next == 0 && i != 0) ? back : next;
            bool paired = linkedList->List[next].prev == i;
            if (i != to) fprintf(stream, "n%zd -> n%zd [style=invis, weight=100];\n", i, to);
            fprintf(stream, "n%zd:e -> n%zd:w [constraint=false, %s];\n",
                    i, to, paired ? "dir=both" : "color=red");
        }
    }

    if (prev == FREE_FLAG) return;
    listIndex from = (i == 0 && prev > 0) ? back : i;
    if (prev < 0 || prev >= back)
        fprintf(stream, "n%zd -> \"invalid %zd\" [color=red];\n", from, prev);
    else if (linkedList->List[prev].next != i)
        fprintf(stream, "n%zd:w -> n%zd:e [constraint=false, color=red];\n", from, prev);
}

void MakeGraphvizScript(FILE * stream, const listResources * linkedList) {

    ASSERT(stream);
    ASSERT(linkedList);

    fprintf(stream, "digraph {\nrankdir=LR;\nranksep=0.5;\nsplines=ortho;\n"
                    "node [shape=record, fontname=Arial, style=filled, fillcolor=\"#EEF4FF\"];\n");

    const listMember sys = linkedList->List[SYS_CELL_INDEX];

    if (linkedList->size || sys.prev > 0)
        fprintf(stream, "n%zu [label=\"[0] SYS\", fillcolor=\"#7ea46d\"];\n", linkedList->capacity);

    listIndex lastFree = FREE_FLAG;

    for (listIndex i = 0; i < (listIndex)linkedList->capacity; i++) {

        const listMember cell = linkedList->List[i];
        fprintf(stream, "n%zd [label=\"{index = %zd", i, i);

        if (i == 0) fprintf(stream, " SYS");
        fprintf(stream, "} | {prev = %zd | %lg | next = %zd}\"];\n", cell.prev, cell.value, cell.next);
        if (i == 0) fprintf(stream, "n0 [fillcolor=\"#7ea46d\"];\n");

        if (i != 0 /* && cell.next == FREE_FLAG */ && cell.prev == FREE_FLAG) {
            fprintf(stream, "n%zd [fillcolor=lightgrey];\n", i);
            if (lastFree == FREE_FLAG)
                fprintf(stream, "{ rank=same; n0; n%zd; }\n"
                                "freeLabel [shape=plain, style=\"\", fontsize=18, fontcolor=black, label=\"Free cells\"];\n"
                                "freeLabel -> n%zd [style=invis, weight=100];\n", i, i);
            else fprintf(stream, "n%zd -> n%zd [style=invis, weight=100];\n", lastFree, i);
            lastFree = i;
        }
        else WriteGraphLinks(stream, linkedList, i);
        
        if (i == sys.prev)
            fprintf(stream, "{ rank=same; HEAD [shape=box]; n%zd; } HEAD -> n%zd:n;\n", i, i);
        if (i == sys.next)
            fprintf(stream, "{ rank=same; TAIL [shape=box]; n%zd; } TAIL -> n%zd:n;\n", i, i);
    }
    fprintf(stream, "}\n");
}

void InsertObjectGraph(FILE * stream, const char * picturePath) {

    ASSERT(stream);
    ASSERT(picturePath);

    fprintf(stream, "<img src=\"../%s\" style=\"width: 60%%;\">\n", picturePath);

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
