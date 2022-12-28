#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glob.h>
#include "hashmap.h"

struct hashmap* hm_create(int num_buckets){
    //error checking
    if(num_buckets <= 0){
        printf("%s\n", "invalid bucket number");
        return NULL; 
    }
    struct hashmap * hm = calloc(1, sizeof(struct hashmap)); 
    hm->num_buckets = num_buckets;
    hm->num_elements = 0;
    hm->map = calloc(hm->num_buckets, sizeof(struct llnode *));//allocate mem for buckets as pointers to nodes
    int i;
    for(i = 0; i < num_buckets; i++){
        hm->map[i] = NULL;
    }
    return hm;
}

//get num_occurences of a word in a specific doc
int hm_get_tf(struct hashmap* hm, char* word, char* document_id){
    if(hm == NULL){//if hashmap isnt initialized properly
        printf("%s\n", "hashmap is NULL");
        return -1;
    }
    if ((word == NULL) || document_id == NULL) {
        return -1;
    }

    int h = hash(hm,word); //get hash, h = bucket number
    if(h == -1){
        return -1;
    }
    //printf("%d\n", h); //for debugging
    if (hm->map[h] == NULL) {
        return -1;
    }
    struct llnode * cur = hm->map[h];
    while(cur != NULL){
        if(!strcmp(word, cur->word)){
            struct lldoc * temp = cur->docs;
            if(temp == NULL){
                return -1;
            }
            while(temp != NULL){
                if(!strcmp(temp->document_id, document_id)){
                    return temp->tf;
                }
                temp = temp->next;
            }
        }
        cur = cur->next;
    }
    return -1;
}

int hm_get_df(struct hashmap* hm, char* word){
    if(hm == NULL){//if hashmap isnt initialized properly
        printf("%s\n", "hashmap is NULL");
        return -1;
    }
    if (word == NULL) {
        return -1;
    }

    int h = hash(hm,word); //get hash, h = bucket number
    if(h == -1){
        return -1;
    }
    //printf("%d\n", h); //for debugging
    if (hm->map[h] == NULL) {
        return -1;
    }
    struct llnode * cur = hm->map[h];
    while(cur != NULL){
        if(!strcmp(word, cur->word)){
            return cur->df;
        }
        cur = cur->next;
    }
    return -1;
}

//adds word, doc, and freq of word in doc and adds it to hm
void hash_table_insert(struct hashmap* hm, char* word, char* document_id, int num_occurrences){
    //error checking
    //printf("%s\n", "before error check");//for debugging
    if(!strcmp(word, "") || hm == NULL || !strcmp(document_id, "") || document_id == NULL || word == NULL){
        return;
    }
    //printf("%s\n", "passed error check");//for debugging
    //if word and document exist -> llnode.num_occurences = num_occurences
    //if word and document dont exist -> add to hashmap and set llnode.num_occurences = num_occurences
    int h = hash(hm,word); //get hash, h = bucket number
    // printf("%d\n %s\n %s\n", h, document_id, word); //for debugging
    //check if already exists in bucket llist
    // printf("%s\n", "checking if word exists");//for debugging
    struct llnode * cur = hm->map[h];
    while(cur != NULL){
        if(!strcmp(word, cur->word)){
            struct lldoc * temp = cur->docs;
            //case if doc node doesnt exit in words list
            if(temp == NULL){//add doc node
                struct lldoc * newDoc = malloc(sizeof(struct lldoc)); 
                newDoc->next = NULL;
                newDoc->document_id = strdup(document_id);
                newDoc->tf = num_occurrences;
                cur->docs = newDoc;
                return;
            }
            //if word node already has a list of docs and doc is in that list
            while(temp != NULL){
                if(!strcmp(document_id, temp->document_id)){
                    temp->tf = num_occurrences;
                    return;
                }
                temp = temp->next;
            }
            //if doc doesnt exist in the word doc list
            temp = cur->docs;
            while(temp->next != NULL){
                temp = temp->next;
            }
            //printf("%s", "searched for existing doc node\n");//for debugging
            struct lldoc * newDoc = malloc(sizeof(struct lldoc)); 
            newDoc->document_id = strdup(document_id);
            newDoc->tf = num_occurrences;
            newDoc->next = NULL;
            temp->next = newDoc;
            cur->df++;
            return;
            // printf("%s %d\n", "num_pccurences: ", cur->num_occurrences);//for debugging
            // printf("%s %s\n", "word: ", cur->word);//for debugging
            // printf("%s %s\n", "doc_id: ", cur->document_id);//for debugging
        }
        cur = cur->next;
    }
    //if doesnt exist, add to hm, bucket is empty
    cur = hm->map[h];
    if(cur == NULL){
        // printf("doc: %s word: %s\n", document_id, word);
        struct llnode * new = malloc(sizeof(struct llnode));
        new->word = strdup(word);
        new->df = 1;
        new->next = NULL;
        struct lldoc * newDoc = malloc(sizeof(struct lldoc));
        newDoc->next = NULL;
        newDoc->document_id = strdup(document_id);
        // printf("newDoc ID: %s\n", newDoc->document_id);//for debugging
        newDoc->tf = num_occurrences;
        new->docs = newDoc;
        // printf("newDocs ID: %s\n", new->docs->document_id);//for debugging
        hm->map[h] = new;
        hm->num_elements++;
        return;
    }
    while(cur->next != NULL){//get last node in list
        cur = cur->next;
    }
    // printf("cur doc ID and word: %s, %s\n", cur->docs->document_id, cur->word);//for debugging
    //atp cur == last node in list
    //case if word doesn exist, so no doc list exists either
    struct llnode * new = malloc(sizeof(struct llnode));
    new->word = strdup(word);
    new->df = 1;
    new->next = NULL;
    struct lldoc * newDoc = malloc(sizeof(struct lldoc));
    newDoc->next = NULL;
    newDoc->document_id = strdup(document_id);
    newDoc->tf = num_occurrences;
    new->docs = newDoc;
    cur->next = new;
    hm->num_elements++;
}

void hm_remove(struct hashmap* hm, char* word){
    //error checking
    if(!strcmp(word, "") || hm == NULL || word == NULL){
        return;
    }
    int h = hash(hm,word); //get hash, h = bucket number
    struct llnode * cur = hm->map[h];
    struct llnode * toRemove;
    if(cur == NULL){//empty bucket
        return;
    }
    //if we need to remove first node
    if(!strcmp(word, cur->word)){
            //loop through doc list and free
            struct lldoc * remove = cur->docs;
            struct lldoc * nextToRemove;
            while(remove != NULL){
                nextToRemove = remove->next;
                // printf("cur word: %s\n", cur->word);//for debugging
                free(remove->document_id);
                free(remove);
                remove = nextToRemove;
            }
            //remove word node
            hm->map[h] = cur->next;
            free(cur->word);
            free(cur);
            return; 
        }
    //remove anywhere else
    while(cur != NULL){
        //printf("%s\n", "in while loop to check and remove word");//for debugging
        if(!strcmp(word, cur->next->word)){
            toRemove = cur->next;
            cur->next = toRemove->next;
            //loop to free doc list
            struct lldoc * remove = toRemove->docs;
            struct lldoc * nextToRemove;
            while(remove != NULL){
                nextToRemove = remove->next;
                free(remove->document_id);
                free(remove);
                remove = nextToRemove;
            }
            free(toRemove->word);
            free(toRemove);
            return; 
        }
        cur = cur->next;
    }
}

void hm_destroy(struct hashmap* hm){
    int i;
    for(i = hm->num_buckets - 1; i >= 0; i--){//start from bottom of list since map points to first bucket
        struct llnode * cur = hm->map[i];
        struct llnode * next;
        while(cur != NULL){
            next = cur->next;
            hm_remove(hm, cur->word);
            cur = next;
        }
        
    }
    
    free(hm->map);
    hm->num_elements = 0;
    //delete the hashmap
    free(hm);
}

int hash(struct hashmap* hm, char* word){
    int sum = 0, i, len, bucket;
    len = strlen(word);
    for(i = 0; i < len; i++){
        sum += (int)word[i];
    }
    bucket = sum % hm->num_buckets;
    return bucket; //returns bucket to be placed in, doesnt actually place in hashmap
}

void printMap(struct hashmap* hm){//for debugging
    struct llnode * cur;
    struct lldoc * temp;
    int i;
    for(i = 0; i < hm->num_buckets; i++){
        // printf("In Bucket: %d\n", i);//for debugging
        cur = hm->map[i];
        while(cur != NULL){
            // printf("word: %s\n", cur->word);//for debugging
            // printf("Hash: %d\n", hash(hm, cur->word));//for debugging
            // printf("docFreq: %d\n", cur->df);//for debugging
            temp = cur->docs;
            while(temp != NULL){
                // printf("doc: %s\nTermFreq: %d\n", temp->document_id, temp->tf);//for debugging
                temp = temp->next;
            }
            cur = cur->next;
        }
    }
    printf("end hashmap\n");
}