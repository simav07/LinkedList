# Doubly linked list

Fast doubly linked list, implemented as an array of structures. The complexity of element insertion and deletion algorithms is *O(1)*.

# Usage

...

# Features

1. Dump with object graph of the list. Right connections between elements drawn as double-headed arrows. Incorrect connections drawn as red one-way arrows. Available memory cells are drawn separately. The list is looped back to a system cell.

! Picture with object graph

2. All information regarding actions performed on the list is saved to HTML file.
3. The complexity of finding an available memory cell is *O(1)*. This is implemented using a singly linked list of free elements.
4. Verifying list connectivity using a powerful function **ListVerify**. Checks are being performed for broken links in the list, among other things.
5. In the project, many functions return an error code, and a dedicated function handles around twenty different errors.

| Error code | Error description |
| :--: | :--: |
| ERR_OK | There's no errors, everything is correct |
| ... | ... |
| ... | ... |
| ... | ... |


**Element of list:**

| Index to the previous | Value | Index to the next |
| :--: | :--: | :--: |

