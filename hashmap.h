#ifndef HASHMAP_H
#define HASHMAP_H

// struct llnode {
//         char* word;
//         char* document_id;
//         int num_occurrences;
//         struct llnode* next;
//         //struct llnode* docs;
// };

struct llnode {
        char* word;
        int df;
        struct llnode* next;
        struct lldoc* docs;
};

struct lldoc{
    char* document_id;
    int tf;
    struct lldoc* next;
};

struct hashmap {
        struct llnode** map;
        int num_buckets;
        int num_elements;
};


struct hashmap* hm_create(int num_buckets);
int hm_get_tf(struct hashmap* hm, char* word, char* document_id);
int hm_get_df(struct hashmap* hm, char* word);
void hash_table_insert(struct hashmap* hm, char* word, char* document_id, int num_occurrences);
void hm_remove(struct hashmap* hm, char* word);
void hm_destroy(struct hashmap* hm);
int hash(struct hashmap* hm, char* word);
void printMap(struct hashmap* hm);//for debugging
struct hashmap* training(glob_t glob, int buckets);
void read_query(char * search, struct hashmap* hm, glob_t glob, double score[]);
void sort(double score[], glob_t glob);
void stop_word(struct hashmap* hm, glob_t glob);
double rank(char * word, struct hashmap* hm, char* document_id, glob_t glob);
void append(double scores[], glob_t glob, char * destination);
void swap_int(double* xp, double* yp);
void swap1(char **str1_ptr, char **str2_ptr);

#endif