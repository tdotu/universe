#include <stdint.h>
#include <string.h>


/**
 *@param 0 the table token
 *@param 1  the driver list
 **/
char **get_tabel_section(char * token, char *table)
{
    int token_len = strlen(token);
    char read_token[token_len+1];

    int i = 0, j,k;

    int num_entries = 0;
    int entries_start;
    char **entries;

    // search for the token
    while(! strcmp(read_token, token) ) {
        strcpy(read_token, table + i);
        i++;
    }

    // search '{'
    while( table[i] != '{' ) {
        i++;
    }
printf(".");
    entries_start = i;

    // count entries until '}'
    while( 1 ) {
        while( table[i] != ' ' && table[i] != '\n') {i++;}
        if( table[i] == '}' ) {
            break;
        } else {
            num_entries ++;
        }
    }
printf(".");
    entries = malloc(num_entries * sizeof(uintptr_t));
    int start[num_entries];
    int len[num_entries];
    memset(start, 0, num_entries * sizeof(int));
    memset(len, 0, num_entries * sizeof(int));
printf(".");
    i = entries_start;
    for(j = 0; j < num_entries; j++) {
        while( table[i] == ' ' || table[i] == '\n') i++;
        while( table[i] != ' ' && table[i] != '\n') {
            if(start[j] == 0) start[j] = i;
            len[j]++;
            i++;
        }
    }
printf(".");
    // copy
    for(j = 0; j < num_entries; j++) {
        i = start[j];
        for(k = 0; k < len[j]; k++) {
            entries[j][k] = table[i];
            i++;
        }
    }
printf(".");
}