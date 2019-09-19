#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define ERROR -1

//estimated
#define COMMAND_LENGTH 7
#define IO_LENGTH 1024
#define NAME_LENGTH 128
#define RELATIONS_NUMBER 31
#define ENTITIES_NUMBER 109

//encode
#define TYPE_ACTION 0
#define TARGET 3

#define ADDREL 1
#define DELREL 2
#define ADDENT 3
#define DELENT 4
#define REPORT 5
#define END 6


typedef struct entity{
    char name[NAME_LENGTH];
    int occurrences;
} entity;


typedef struct max_receivers{
    entity tie;
    struct max_receivers* next;
} max_receivers;

typedef struct relation_element{
    entity* sender;
    entity* receiver;
    struct relation_element* next;
} relation_element;

typedef struct relation{
    max_receivers* maxReceiver;
    int max;
    relation_element* relations;
} relation;

/**
 * A node of the hashMap of the relations
 */
typedef struct node_Rel{
    char key[NAME_LENGTH];
    relation* val;
    struct node_Rel *next;
} node_Rel;

/**
 * A node of the hashMap of the entities
 */
typedef struct node_Ent{
    char key[NAME_LENGTH];
    entity* val;
    struct node_Ent *next;
} node_Ent;

/**
 * The effective hashMap of the relations
 */
typedef struct table_Rel{
    int size;
    node_Rel **list;
} table_Rel;

/**
 * The effective hashMap of the relations
 */
typedef struct table_Ent{
    int size;
    node_Ent** list;
} table_Ent;


int hashCode(int size, char *key_String);

table_Rel *createTable_Rel(int size);

void insert_Rel(char key[],relation *val);

relation* lookup_Rel(table_Rel *t, char *key);

table_Ent *createTable_Ent(int size);

void insert_Ent(char key[],entity *val);

entity* lookup_Ent(table_Ent *t, char *key);

int encode(const char command[]);

void addrel(char input[]);

void delrel(char input[]);

void addent(char input[]);

void delent(char input[]);

void report();

void divide_input(char input[],char name[], char sender[], char receiver[]);

void calcMax( relation *rel);

void addElement(entity element, relation *rel);

void MergeSort_list(max_receivers **headRef);

max_receivers* SortedMerge_list(max_receivers *a, max_receivers *b);

void FrontBackSplit(max_receivers* source, max_receivers** frontRef, max_receivers** backRef);

void clean_list(max_receivers** list);

void heapify(char a[][IO_LENGTH],int n);

void adjust(char a[][IO_LENGTH],int n);

void heapsort(char a[][IO_LENGTH],int n);

void itoam(int n, char s[]);

void reverse(char s[]);

//Global variables
table_Ent *entities;
table_Rel *relations;
int debug=0;

int main() {
    int operation;
    entities= createTable_Ent(ENTITIES_NUMBER);
    relations= createTable_Rel(RELATIONS_NUMBER);

    char input [IO_LENGTH];
    while (TRUE) {
        fgets(input, IO_LENGTH, stdin);
        operation = encode(input);
        debug++;
        switch (operation) {
            case ADDREL:
                addrel(&input[COMMAND_LENGTH]);
                break;
            case DELREL:
                delrel(&input[COMMAND_LENGTH]);
                break;
            case ADDENT:
                addent(&input[COMMAND_LENGTH]);
                break;
            case DELENT:
                delent(&input[COMMAND_LENGTH]);
                break;
            case REPORT:
                report();
                break;
            case END:
                return 0;
            default:
                printf("Command not found\n");
        }
    }
}

/**
 * For each relationship print on the std output the name and the maximum receiver
 */
void report() {
    node_Rel *temp;
    int none=1;
    char output[RELATIONS_NUMBER][IO_LENGTH];
    int j=0;
    for (int i=0; i<RELATIONS_NUMBER;i++) {
        temp = relations->list[i];
        while (temp) {
            calcMax(temp->val);
            none = 0;
            strcpy(output[j], temp->key);
            strcat(output[j], " ");
            max_receivers *tie = temp->val->maxReceiver;
            while (tie) {
                strcat(output[j], tie->tie.name);
                strcat(output[j], " ");
                tie = tie->next;
            }
            char max_string[NAME_LENGTH];
            itoam(temp->val->max, max_string);
            strcat(output[j], max_string);
            strcat(output[j], ";");
            temp = temp->next;
            j++;
        }
    }
    heapsort( output, j);
    for (int k=0; k<j; k++) {
        if(k!=j-1){
            strcat(output[k], " ");
        }
        printf("%s",output[k]);
    }
    if (none){
        printf("none");
    }
    printf("\n");
}


/**
 * Add a new entity
 * @param input the name of the entity that will be created
 */
void addent(char input[]) {
    int found=0;
    for(int i=strlen(input)-1;i>0 && !found;i--) {
        if (input[i] == '\n') {
            input[i] = '\0';
            found=1;
        }
    }
    entity* check = lookup_Ent(entities,input);
    if (!check) {
        entity *val;
        val = (entity *) malloc(sizeof(entity));
        strcpy(val->name, input);
        val->occurrences=0;
        insert_Ent(input, val);
    }
}


/**
 * Delete an entity and all the relationships involved
 * @param input The entity to delete
 */
void delent(char input[]) {
     int found=0;
     for(int i=strlen(input)-1;i>0 && !found;i--) {
         if (input[i] == '\n') {
             input[i] = '\0';
             found=1;
         }
     }
     if(!lookup_Ent(entities,input)){
         return;
     }
     node_Rel* temp_rel;
     char fake_input [IO_LENGTH];
     relation_element* temp_rel_el=NULL;
     relation_element* back_up_el=NULL;
     node_Rel* back_up=NULL;
     for (int i=0; i<RELATIONS_NUMBER;i++) {
         temp_rel = relations->list[i];
         while (temp_rel) {
             temp_rel_el = temp_rel->val->relations;
             back_up=temp_rel->next;
             while (temp_rel_el) {
                 back_up_el=temp_rel_el->next;
                 if (strcmp(temp_rel_el->sender->name, input) == 0 || strcmp(temp_rel_el->receiver->name, input) == 0) {
                     strcpy(fake_input, temp_rel_el->sender->name);
                     strcat(fake_input, " ");
                     strcat(fake_input, temp_rel_el->receiver->name);
                     strcat(fake_input, " ");
                     strcat(fake_input, temp_rel->key);
                     strcat(fake_input, "\n");
                     delrel(fake_input);
                     strcpy(fake_input, "\0");
                 }
                 temp_rel_el = back_up_el;

             }
             temp_rel=back_up;
         }
     }

     node_Ent *to_free_ent=NULL;
     node_Ent *prev=NULL;
     int pos = hashCode(entities->size, input);
     node_Ent *list = entities->list[pos];
     node_Ent *temp_ent = list;
     if (temp_ent) {
         if (strcmp(temp_ent->key, input) == 0) {
             to_free_ent = temp_ent;
         }
         while (temp_ent->next) {
             if (strcmp(temp_ent->next->key, input) == 0) {
                 to_free_ent = temp_ent->next;
                 prev = temp_ent;
             }
             temp_ent = temp_ent->next;
         }


     }
     if (to_free_ent) {
         if (prev) {
             prev->next = to_free_ent->next;
         } else{
             entities->list[pos]=to_free_ent->next;
         }
         free(to_free_ent);
     }

}

/**
 * Remove a relationship between 2 entities
 * @param input the relationship that will be deleted and the entities involved
 */
void delrel(char input[]) {
    char rel[NAME_LENGTH];
    char sender[NAME_LENGTH];
    char receiver[NAME_LENGTH];
    int stop=0;
    divide_input(input,rel,sender,receiver);
    relation* check = lookup_Rel(relations,rel);
    entity* check_sender= lookup_Ent(entities,sender);
    entity* check_receiver= lookup_Ent(entities,receiver);
    if(check_sender && check_receiver && check){
        relation_element* temp=check->relations;
        relation_element* temp_prev=NULL;
        relation_element* to_free=NULL;
        while (temp && !stop){
            if(check_receiver==temp->receiver){
                if(check_sender==temp->sender){
                    if(temp_prev){
                        temp_prev->next=temp->next;
                    }
                    else{
                        check->relations = temp->next;

                        //no more rel_el so delete node_rel
                        if(!temp->next){
                            node_Rel *to_free_rel=NULL;
                            node_Rel *prev=NULL;
                            int pos = hashCode(relations->size, rel);
                            node_Rel *list = relations->list[pos];
                            node_Rel *temp_rel = list;
                            if (strcmp(temp_rel->key, rel) == 0){
                                to_free_rel=temp_rel;
                            }
                            while (temp_rel->next) {
                                if (strcmp(temp_rel->next->key, rel) == 0) {
                                    to_free_rel = temp_rel->next;
                                    prev = temp_rel;
                                }
                                temp_rel = temp_rel->next;
                            }

                            if (to_free_rel) {
                                if (prev) {
                                    prev->next = to_free_rel->next;
                                } else{
                                    relations->list[pos]=to_free_rel->next;
                                }
                                free(to_free_rel->val);
                                free(to_free_rel);
                                check=NULL;
                            }

                        }
                    }
                    to_free=temp;
                    stop=1;
                }
            }
            temp_prev=temp;
            temp=temp->next;
        }
        if(to_free) {
            free(to_free);
        }
    }
}

/**
 * Add a new relationship between 2 entities
 * @param input the relationship that will be added and the entities involved
 */
void addrel(char input[]) {
    char rel[NAME_LENGTH];
    char sender[NAME_LENGTH];
    char receiver[NAME_LENGTH];
    divide_input(input,rel,sender,receiver);
    relation* check = lookup_Rel(relations,rel);
    entity* check_sender= lookup_Ent(entities,sender);
    entity* check_receiver= lookup_Ent(entities,receiver);
    if (check_sender && check_receiver) {
        if (!check) {
            relation *val;
            val = (relation *) malloc(sizeof(relation));
            //Max receiver initialization
            max_receivers* max=(max_receivers*)malloc(sizeof(max_receivers));
            max->tie = *check_receiver;
            max->next=NULL;
            //Relations initialization
            relation_element* rel_element=(relation_element*)malloc(sizeof(relation_element));
            rel_element->next=NULL;
            rel_element->receiver=check_receiver;
            rel_element->sender=check_sender;
            //Val initialization
            val->max=1;
            val->maxReceiver = max;
            val->relations=rel_element;
            insert_Rel(rel, val);
        } else {

            relation_element *list = check->relations;
            relation_element *temp = list;
            while (temp) {
                if(strcmp(receiver,temp->receiver->name)==0 && strcmp(sender,temp->sender->name)==0){
                    return;
                }
                temp=temp->next;
            }
            check->relations = (relation_element *) malloc(sizeof(relation_element));
            check->relations->receiver = check_receiver;
            check->relations->sender = check_sender;
            check->relations->next=list;
        }
    }
}

/**
 * Convert the command in an integer
 * @param command The command that needs to be converted
 * @return ADDREL if the command is addrel, DELREL if the command is delrel, ADDENT if the command is addent,
 * DELENT if the command is delent, REPORT if the command is report, END if the command is end
 */
int encode(const char command []) {
    char difference = command[TYPE_ACTION];
    if (difference == 'e') {
        return END;
    }
    if (difference == 'a') {
        difference = command[TARGET];
        if (difference == 'r') {
            return ADDREL;
        }
        if (difference == 'e') {
            return ADDENT;
        }
    }
    if (difference == 'd') {
        difference = command[TARGET];
        if (difference == 'r') {
            return DELREL;
        }
        if (difference == 'e') {
            return DELENT;
        }
    }
    if (difference == 'r') {
        return REPORT;
    }
    return ERROR;
}


//HashMap handle functions

/**
 * Create a new hash map of relations
 * @param size the size of the hash map
 * @return a new hash map
 */
table_Rel *createTable_Rel(int size){
    table_Rel *t = (table_Rel*)malloc(sizeof(table_Rel));
    t->size = size;
    t->list = (node_Rel**)malloc(sizeof(node_Rel*)*size);
    int i;
    for(i=0;i<size;i++) {
        t->list[i] = NULL;
    }
    return t;
    }

/**
* Create a new hash map of entities
* @param size the size of the hash map
* @return a new hash map
*/
table_Ent *createTable_Ent(int size){
    table_Ent *t = (table_Ent*)malloc(sizeof(table_Ent));
    t->size = size;
    t->list = (node_Ent**)malloc(sizeof(node_Ent*)*size);
    int i;
    for(i=0;i<size;i++) {
        t->list[i] = NULL;
    }
    return t;
}

/**
 * Return the position in an hash table based on the key
 * @param size the size of the hash table
 * @param key_String the key
 * @return the position in the hash table
 */
int hashCode(int size, char *key_String){
    int key=0;
    for(int i=0;i<strlen(key_String);i++){
        key=key+key_String[i];
    }
    return key%size;
}

/**
 * Insert a new relation in the hash table
 * @param key the key
 * @param val the relation to add
 */
void insert_Rel(char key[], relation *val) {
    int pos = hashCode(relations->size,key);
    node_Rel *list = relations->list[pos];
    node_Rel *newNode = (node_Rel*)malloc(sizeof(node_Rel));

    strcpy(newNode->key , key);
    newNode->val = val;
    newNode->next = list;

    relations->list[pos] = newNode;
}
/**
 * Insert a new entity in the hash table
 * @param key the key
 * @param val the entity to add
 */
void insert_Ent(char key[], entity *val){
    int pos = hashCode(entities->size,key);
    node_Ent *list = entities->list[pos];
    node_Ent *newNode = (node_Ent*)malloc(sizeof(node_Ent));

    strcpy(newNode->key , key);
    newNode->val = val;
    newNode->next = list;

    entities->list[pos] = newNode;
}

/**
** find an element in an hash map from its key
* @param t the hash map
* @param key the key of the value
* @return the value if founded, an empty relation otherwise
*/
relation* lookup_Rel(table_Rel *t, char *key){
    int pos = hashCode(t->size, key);
    node_Rel *list = t->list[pos];
    node_Rel *temp = list;
    while(temp){
        if(strcmp(temp->key,key)==0){
            return (temp->val);
        }
        temp = temp->next;
    }
    return NULL;
}

/**
** find an element in an hash map from its key
* @param t the hash map
* @param key the key of the value
* @return the value if founded, an NULL otherwise
*/
entity* lookup_Ent(table_Ent *t, char *key){
    int pos = hashCode(t->size, key);
    node_Ent *list = t->list[pos];
    node_Ent *temp = list;
    while(temp){
        if(strcmp(temp->key,key)==0){
            return (temp->val);
        }
        temp = temp->next;
    }
    return NULL;
}

/**
 * divide the string input in its components (regex " ")
 * @param input the string to divide
 * @param name the third component
 * @param sender the first component
 * @param receiver the second component
 */
void divide_input(char *input,char *name, char *sender, char *receiver) {
    int component=1;
    int j=0;
    int k=0;
    for(int i =0;i<strlen(input);i++){
        switch (component) {
            default:
                printf("error dividing input");
                break;
            case 1 : {
                if (input[i] != ' ') {
                    sender[i] = input[i];
                }else{
                    component = 2;
                    sender[i] = '\0';
                }
                break;
            }
            case 2:{
                if (input[i] != ' ') {
                    receiver[j] = input[i];
                    j++;
                } else {
                    component = 3;
                    receiver[j] = '\0';
                }
                break;
            }
            case 3:{
                if (input[i] != '\n') {
                    name[k] = input[i];
                    k++;
                } else {
                    name[k] = '\0';
                    return;
                }
                break;
            }

        }

    }

}

/**
 * calculate the maximum receiver of a relation and save it in relation.max, and the maximum receiver
 */
void calcMax(relation* rel) {
    relation_element *temp=rel->relations;
    while (temp){
        temp->receiver->occurrences++;
        temp=temp->next;
    }
    temp=rel->relations;
    int max=0;
    while (temp){
        if(temp->receiver->occurrences>max){
            max=temp->receiver->occurrences;
        }
        temp=temp->next;
    }
    rel->max=max;
    clean_list(&rel->maxReceiver);
    temp=rel->relations;
    while (temp){
        if(temp->receiver->occurrences==max){
            addElement(*temp->receiver,rel);
        }
        temp->receiver->occurrences=0;
        temp=temp->next;
    }
    MergeSort_list(&rel->maxReceiver);
}

//max receiver list functions

void addElement(entity element, relation *rel){
    max_receivers* new_element=(max_receivers*)malloc(sizeof(max_receivers));
    new_element->tie=element;
    if (rel->maxReceiver) {
        new_element->next = rel->maxReceiver->next;
        rel->maxReceiver->next = new_element;
    }
    else{
        rel->maxReceiver=new_element;
        new_element->next=NULL;
    }
}

/* sorts the linked list by changing next pointers (not data) */
void MergeSort_list(max_receivers **headRef){
    max_receivers* head = *headRef;
    max_receivers* a;
    max_receivers* b;

    /* Base case -- length 0 or 1 */
    if ((head == NULL) || (head->next == NULL)) {
        return;
    }

    /* Split head into 'a' and 'b' sublists */
    FrontBackSplit(head, &a, &b);

    /* Recursively sort the sublists */
    MergeSort_list(&a);
    MergeSort_list(&b);

    /* answer = merge the two sorted lists together */
    *headRef = SortedMerge_list(a, b);
}

max_receivers* SortedMerge_list(max_receivers *a, max_receivers *b)
{
    max_receivers* result = NULL;

    /* Base cases */
    if (a == NULL)
        return (b);
    else if (b == NULL)
        return (a);

    /* Pick either a or b, and recur */
    if (strcmp(a->tie.name,b->tie.name)<0) {
        result = a;
        result->next = SortedMerge_list(a->next, b);
    }
    else {
        result = b;
        result->next = SortedMerge_list(a, b->next);
    }
    return (result);
}

/* UTILITY FUNCTIONS */
/* Split the nodes of the given list into front and back halves,
    and return the two lists using the reference parameters.
    If the length is odd, the extra node should go in the front list.
    Uses the fast/slow pointer strategy. */
void FrontBackSplit(max_receivers* source, max_receivers** frontRef, max_receivers** backRef)
{
    max_receivers* fast;
    max_receivers* slow;
    slow = source;
    fast = source->next;

    /* Advance 'fast' two nodes, and advance 'slow' one node */
    while (fast != NULL) {
        fast = fast->next;
        if (fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    /* 'slow' is before the midpoint in the list, so split it in two
    at that point. */
    *frontRef = source;
    *backRef = slow->next;
    slow->next = NULL;
}

void clean_list(max_receivers** list){
    max_receivers *temp, *temp_next;
    temp=*list;
    while (temp){
        temp_next=temp->next;
        free(temp);
        temp=temp_next;
    }
    *list=NULL;
}

void heapify(char a[][IO_LENGTH],int n) {
    int k,i,j;
    char  item[IO_LENGTH];
    for (k=1;k<n;k++) {
        strcpy(item , a[k]);
        i = k;
        j = (i-1)/2;
        while((i>0)&&(strcmp(item,a[j])>0)) {
            strcpy(a[i] , a[j]);
            i = j;
            j = (i-1)/2;
        }
        strcpy(a[i] , item);
    }
}
void adjust(char a[][IO_LENGTH],int n) {
    int i,j;
    char item[IO_LENGTH];
    j = 0;
    strcpy(item , a[j]);
    i = 2*j+1;
    while(i<=n-1) {
        if(i+1 <= n-1)
            if(strcmp(a[i] ,a[i+1])<0)
                i++;
        if(strcmp(item,a[i])<0) {
            strcpy(a[j] , a[i]);
            j = i;
            i = 2*j+1;
        } else
            break;
    }
    strcpy(a[j] , item);
}

void heapsort(char a[][IO_LENGTH],int n) {
    int i;
    char t[IO_LENGTH];
    heapify(a,n);
    for (i=n-1;i>0;i--) {
        strcpy(t , a[0]);
        strcpy(a[0] , a[i]);
        strcpy(a[i] , t);
        adjust(a,i);
    }
}

void itoam(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}