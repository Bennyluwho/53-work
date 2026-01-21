#include "linkedlist.h"
#include "hw1.h"

// The linked list used the data field of the node_t struct as a value (not a pointer to a value). 
// The void* data can hold any value. In this case, we use the memory space allocated for the pointer to store the value itself. 

int longComparator(const void *p, const void *q)
{
    // casting the pointers to integer values
    return ((long)p - (long)q);
}

void longPrinter(void *data, void* fp)
{
        fprintf((FILE*) fp, "%ld ", (long) data);
}

void longDeleter(void *data)
{
    return;   //Nothing to delete! data field is not a pointer to dynamically allocated data
}

int main(int argc, char* argv[]) {
    // long start = 1;
    // long end = 20;
    // long i;

    // // Create and initialize integer linked list
    // list_t* list = CreateList(&longComparator, &longPrinter, &longDeleter);

    // // Output: List is empty
    // printf("**********Empty List**********\n");    
    // PrintLinkedList(list, stdout);
    // printf("\n");


    // // Output: 1
    // printf("**********1 List Value**********\n");   
    // // Insert first node. Why does start need to be casted?
    // InsertAtHead(list, (void*)start);
    // PrintLinkedList(list, stdout);
    // printf("\n");


    // printf("**********Full List except 10**********\n");   
    // // Inserting the remaining values, skip value 10
    // for (i = start + 1; i <= end; i++) {
    //     if(i != 10)
    //         InsertAtTail(list, (void*)i);
    // }
    // // Output (each value on a line): 1 2 3 4 5 6 7 8 9 11 12 13 14 15 16 17 18 19 20
    // PrintLinkedList(list, stdout);
    // printf("\n");

    // printf("**********Full List**********\n");   
    // InsertInOrder(list, (void*)10);
    // // Output (each value on a line): 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
    // PrintLinkedList(list, stdout);
    // printf("\n");


    // printf("**********Duplicate Odd Numbers In List**********\n");   
    // // Inserting every other value again, inOrder based on comparator
    // for (i = start; i <= end; i=i+2) {
    //     InsertInOrder(list, (void*)i);
    // }
    // // Output (each value on a line): 1 1 2 3 3 4 5 5 6 7 7 8 9 9 10 11 11 12 13 13 14 15 15 16 17 17 18 19 19 20
    // PrintLinkedList(list, stdout);
    // printf("\n");


    // // Delete the list  - Need to implement this function for HW 
    // DestroyList(&list);

    //TODO: use the lines below to help test and create my own MIPSinstrList
    // list_t* list = CreateList(&longComparator, &longPrinter, &longDeleter);
    // PrintLinkedList(list, stdout);
    // printf("\n");
    // DestroyList(&list);

    // list_t *list = CreateList(&MIPSinstr_uidComparator, &MIPSinstr_Printer, &MIPSinstr_Deleter);

    // MIPSinstr* myInstr1 = loadInstrFormat("r 00000009 jalr 2\n");
    // MIPSinstr* myInstr2 = loadInstrFormat("i 84000000 lh 4\n");
    // MIPSinstr* myInstr3 = loadInstrFormat("j 0c000009 jal 6\n");
    // InsertAtHead(list, myInstr1);
    // InsertInOrder(list, myInstr2);
    // InsertInOrder(list, myInstr3);

    list_t *list = createMIPSinstrList(stdin);
    char* humanRegisters = "$zero,$at,$v0,$v1,$a0,$a1,$a2,$a3,$t0,$t1,$t2,$t3,$t4,$t5,$t6,$t7,$s0,$s1,$s2,$s3,$s4,$s5,$s6,$s7,$t8,$t9,$k0,$k1,$gp,$sp,$fp,$ra";


    //look inside instr_map.txt and the 2nd value (uid) to test
    MIPSfields* test1 = malloc(sizeof(MIPSfields));
    MIPSfields* test2 = malloc(sizeof(MIPSfields));
    MIPSfields* test3 = malloc(sizeof(MIPSfields));
    MIPSfields* test4 = malloc(sizeof(MIPSfields));
    
    test1->uid = 0x20000000;
    test2->uid = 0x00000004;
    test3->uid = 0x0000002b;
    test4->uid = 0xffffffff;

    printf("%d\n",printInstr(test4, list, &humanRegisters, stdout));

    free(test1);
    free(test2);
    free(test3);
    free(test4);
    DestroyList(&list);
    return 0;
}
