#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <math.h>
#include "hashmap.h"

//remove stop words
void stop_word(struct hashmap* hm, glob_t glob){
    // printf("before removal\n");//for debugging
    // printMap(hm);//for debugging
    //loop through hashmap, if(df of word == glob.gl_pathc){remove word}
    struct llnode * cur;
    int i;
    int n = glob.gl_pathc;//gets num of elements in scores
    for(i = 0; i < hm->num_buckets; i++){
        cur = hm->map[i];
        while(cur != NULL){
            if(cur->df == n){
                // printf("%s\n", cur->word);//for debugging
                hm_remove(hm, cur->word);
                // printMap(hm);//for debugging
            }
            cur = cur->next;
        }
    }
}

struct hashmap* training(glob_t glob, int buckets){
    //create hashmap
    struct hashmap* hm = hm_create(buckets);
    if(hm == NULL){
        return NULL;
    }
    //loop through documents from glob and add every word to hashmap
    int i;
    int n = glob.gl_pathc;//gets num of elements in scores
    for(i = 0; i < n; i++){
        FILE * D = fopen(glob.gl_pathv[i],"r");
        char buf[100];
        while (fscanf(D, "%s", buf) == 1){//read text word by word
            int occur = hm_get_tf(hm, buf, glob.gl_pathv[i]);
            if(occur == -1){
                    occur = 0;
            }
            // printf("doc: %s word: %s\n", glob.gl_pathv[i], buf);//for debugging
            hash_table_insert(hm, buf, glob.gl_pathv[i], occur + 1);
        }
        fclose(D);
    }
    stop_word(hm, glob); //remove stopwords from hashmap
    // printMap(hm);//for debugging
    return hm;
}

//read search, parse char in by " " to get words
void read_query(char * search, struct hashmap* hm, glob_t glob, double score[]){
    // int score[glob.gl_pathc];//make an array top hold each ranking for each file
    //parse search
    char * piece = strtok(search, " ");
    int i;
    int n = glob.gl_pathc;//gets num of elements in scores
    //get idf score for every doc 
    while(piece != NULL){
        for(i = 0; i < n; i++){
        
            //read a word and call rank
            score[i] += rank(piece, hm, glob.gl_pathv[i], glob);
            //printf("%s idf: %d\n", piece, score[i]);//for debugging
            
        }
        piece = strtok(NULL, " ");
    }

    sort(score, glob);
    
    //print results
    //doc[i]
    for(i = 0; i < n; i++){
        char* substr = malloc(7);
        substr[6] = '\0';
        strncpy(substr, glob.gl_pathv[i]+7, 6);
        printf("%s\n", substr);
        free(substr);
    }
}

void swap_int(double* xp, double* yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void swap1(char **str1_ptr, char **str2_ptr)
{
char *temp = *str1_ptr;
*str1_ptr = *str2_ptr;
*str2_ptr = temp;
}
 
// A function to implement bubble sort
void sort(double score[], glob_t glob)
{
    int i, j;
    int n = glob.gl_pathc;//gets num of elements in scores
    
    for (i = 0; i < n - 1; i++){
        // Last i elements are already in place
        for (j = 0; j < n - i - 1; j++){
            if (score[j] < score[j + 1]){
                swap_int(&score[j], &score[j + 1]);
                swap1(&glob.gl_pathv[j], &glob.gl_pathv[j + 1]);//swap pathnames tp match swapped rankings
            }
        }
    }
}

double rank(char * word, struct hashmap* hm, char* document_id, glob_t glob){
    //get tf_idf score for word in doc i
    double df;
    double idf;
    int tf;
    int n = glob.gl_pathc;//gets num of elements in scores
    df = hm_get_df(hm, word);
    //calculate idf
    
    if (df==-1){
        df=0;
    }
    if(df == 0){
        idf = log(n/(1 + df));
    }
    if(df != 0){
        idf = log(n/df);
    }
    // printf("word, df: %s, %f\n", word, df);//for debugging
    // printf("word, idf: %s, %f\n", word, idf);//for debugging
    tf = hm_get_tf(hm, word, document_id);
    if(tf==-1){
        tf =0;
    }
    // printf("word, tf: %s, %d\n", word, tf);//for debugging
    // printf("word, tf_idf: %s, %f\n", word, tf_idf);//for debugging
    //printf("tf*idf ===%f\n", tf*idf);//for debugging

    return tf*idf;
}

//function to append to scores file
void append(double scores[], glob_t glob, char * destination){
    FILE *fp = fopen(destination, "a+");
    // If file is not found then return.
    if (!fp) {
        printf("Unable to open/"
                "detect file(s)\n");
        return;
    }
    
    int i;
    int n = glob.gl_pathc;//gets num of elements in scores
    for(i = 0; i < n; i++){
        char* substr = malloc(7);
        substr[6] = '\0';
        strncpy(substr, glob.gl_pathv[i]+7, 6);
        fprintf(fp, "%s     ", substr);
        fprintf(fp, "%0.3f\n", scores[i]);
        free(substr);
    }
    fclose(fp);
}

int main(void){
    //get info from user, num_buckets, search query, directory to search
    int numBuckets;
    printf("How many buckets?\n");
    scanf("%d", &numBuckets);
    //get file directory and loop through all files and hash_table_insert all words
    glob_t result;
    glob("p5docs/*.txt", 0, NULL, &result);//fill with the right parameters
    //training phase
    struct hashmap * train = training(result, numBuckets);// train == hashmap with all the word from all the docs
    if(train == NULL){
        printf("Buckets cannot be less than 1\n");
        exit(0);
    }
    //printf("The value of pathc : %zu\n", result.gl_pathc);//for debugging
    //printMap(train);//for debugging

    // printf("\n");//for debugging
    while(1){
        char in[1024] = "\0";
        printf("Enter search string or X to exit:\n");
        scanf(" %[^\n]s",in);//works when I change \n to any other char, I think its taking the \n from when entering the num_buckets
        if(!strcmp(in, "X")){
            hm_destroy(train);
            globfree(&result);
            exit(0);//exit program with no error
        }

        //printf("After input from user is received\n");//for debugging
        //read in search and sort scores
        int n = result.gl_pathc;//gets num of elements in scores
        double score[n];//make an array top hold each ranking for each file
        read_query(in, train, result, score);

        //add to scores file
        append(score, result, "search_scores.txt");

        int i;
        for(i = 0; i < n; i++){
            score[i] = 0;
        }
        // printMap(train);//for debugging
    }


    //free all
    hm_destroy(train);
    globfree(&result);
    
    return 0;
}