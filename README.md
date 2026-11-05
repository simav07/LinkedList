# Doubly linked list

Fast doubly linked list, implemented as an array of structures. The complexity of element insertion and deletion algorithms is *O(1)*.

# Usage

...

# Features

1. Dump with object graph of the list. Right connections between elements drawn as double-headed arrows. Incorrect connections drawn as red one-way arrows. Available memory cells are drawn separately. the list is looped back to a system cell.

! Picture with object graph

2. The complexity of finding an available memory cell is *O(1)*. This is implemented using a singly linked list of free elements.
3. Verifying list connectivity using a powerful function ListVerify. Checks are being performed for broken links in the list, among other things.
4. 
**Element of list:**

| Index to the previous | Value | Index to the next |
| :--: | :--: | :--: |

## First commit
First commit