# Doubly linked list

Fast doubly linked list, implemented as an array of structures. The complexity of element insertion and deletion algorithms is *O(1)*.

This applies when the physical index is known and no resizing or verification is required. Resizing takes *O(capacity)*, and checking the occupied ring takes *O(size)*.

**Element of list:**

| Index to the previous | Value | Index to the next |
| :--: | :--: | :--: |
| `prev` (`listIndex`) | `value` (`listElem_t`, currently `double`) | `next` (`listIndex`) |

Cell `0` is the system cell. Its `next` stores the tail index, and its `prev` stores the head index. In an empty list both links point to `0`. Each free cell has `prev == FREE_FLAG` and a poison value (`NAN`). Its `next` points to the next free cell, with `-1` marking the end of the free chain.

# Usage

The current source uses Windows functions for creating folders and naming reports. Build it with MinGW GCC and add Graphviz's `dot` executable to `PATH`.

From the project directory, compile and run the demonstration in PowerShell:

```powershell
g++ -std=gnu++20 -DVERIFY -DLISTDUMP -DMYDEBUG main.cpp -o list.exe
.\list.exe
```

The current `main.cpp` builds with `g++` and GNU extensions. The example in `main` creates a list, inserts values at both ends and deliberately sets `List[1].next = 0` to demonstrate a broken connection. With `VERIFY` enabled, this produces an error report and an early return.

Open the new report in `html/`. Its PNG images and DOT script are saved under `graphs/<report-name>/`. Keep these folders together so that image links work. Each run creates a separate report.

Use `ListCtor` before working with a list and `ListDtor` when finished. Compare returned status codes with `ERR_OK`, whose value is `100`. The `position` argument is a physical array index.

| Function | Purpose |
| :-- | :-- |
| `PushFront` / `PushBack` | Add an element at the head / tail |
| `PushAfter` / `PushBefore` | Insert relative to a specified cell |
| `Insert` | Insert after the specified cell |
| `Erase` | Remove an element and return its cell to the free chain |
| `GetFront` / `GetBack` | Write the head / tail index to the output argument |
| `GetSize` | Write the number of elements to the output argument |
| `ListResize` | Change array capacity while preserving occupied indices |

# Features

1. Dump with object graph of the list. Right connections between elements drawn as double-headed arrows. Incorrect connections drawn as red one-way arrows. Available memory cells are drawn separately. The list is looped back to a system cell.

For a list with two values, the occupied ring can be represented as follows. The two `SYS` boxes represent the same cell.

```mermaid
flowchart LR
    S["[0] SYS"] <--> A["[1] 100"]
    A <--> B["[2] 200"]
    B <--> R["[0] SYS"]
```

2. Each call to `LIST_DUMP` saves a snapshot to an HTML file, including the graph, a table of cells and list metadata.
3. The complexity of finding an available memory cell is *O(1)*. This is implemented using a singly linked list of free elements.
4. The verification function is named `ListVerity` in the source. It checks the occupied ring, backward links, element count, system cell and the first free cell. The current verifier checks only the beginning of the free chain.
5. Many functions return an error code. `GetErrorDescription` provides a message for each of the 18 codes in `include/errors.h`.

| Error code | Error description |
| :--: | :--: |
| ERR_OK | There's no errors, everything is correct |
| ERR_CALLOC | Failed to allocate the list array |
| ERR_GET_CELL | Failed to obtain a free cell |
| ERR_PUSHFRONT | Failed to insert at the front of the list |
| ERR_PUSHBEFORE | Failed to insert before the specified cell |
| ERR_ERASE | Cannot erase a cell with a poison value |
| ERR_LIST | Invalid list |
| ERR_SIZE | List size must be less than capacity |
| ERR_DISRUPT | List links are inconsistent with the ring or its size |

Capacity starts at `10`, including the system cell. When the free chain is empty, insertion doubles capacity. After deletion, capacity may halve if fewer than a quarter of the cells are occupied and every cell in the removed half is free. Automatic shrinking stops at the initial capacity. Both resizing operations preserve occupied indices.

# Debug

1. Macro **VERIFY** enables `LIST_VERIFY`. It calls `CheckListErrors`, which runs `ListVerity` and prints the calling function, line number and error description. The macro returns the error code from its calling function.

2. Macro **LISTDUMP** enables `LIST_DUMP`. Reports use the format `YYYYMMDD_HHMM_MMSS-PID.html`, for example `20261106_1430_3015-1234.html`. Here `PID` is the process ID. Pictures have a similar timestamp followed by the dump counter. The counter starts at zero for each run. `PrepareHTML` is called separately, so an HTML file is created even when `LISTDUMP` is disabled.

3. Macro **MYDEBUG** enables `ASSERT` and `PRINT_DEBUG`. A failed assertion prints its location and aborts the program.

For example, breaking the ring with `linkedList.List[1].next = 0` in the demonstration produces this error description:

```text
  <<<<< LIST VERIFY >>>>>
     ! ERROR DETECTED !
...
Description: List links are inconsistent with the ring or its size
```

The omitted line contains the function name and source line number, which depend on where the check is called. Use the red one-way arrows in the corresponding graph to locate the inconsistent links.
