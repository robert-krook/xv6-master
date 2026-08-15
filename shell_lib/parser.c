#include "types.h"
#include "user.h"
#include "fcntl.h"

#include "stat.h"
#include "fs.h"

#include "ctype.h"

#include "bash/shell.h"
#include "bash/command.h"

#include "hashtable.h"

char *
strdup2 (char *src)
{
    char *str;
    char *p;
    int len = 0;

    while (src[len])
        len++;
    str = malloc(len + 1);
    p = str;
    while (*src)
        *p++ = *src++;
    *p = '\0';
    return str;
}

// Tokenizes the input into an array of tokens.
char **
tokenize(const char *input, int *count) 
{
    char **tokens = malloc(sizeof(char*) * 256);

    *count = 0;

    const char *p = input;

    while (*p) {

        while (isspace(*p)) 
            p++;
        
        if (!*p) 
            break;
        
        if (*p == ';' || *p == '&' || *p == '|' || *p == '<' || *p == '>' || *p == '(' || *p == ')') {

            if (*p == ';' && *(p+1) == ';') {
                tokens[(*count)++] = strdup2(";;");
                p += 2;
            }
            else if ((*p == '&' && *(p+1) == '&') || (*p == '|' && *(p+1) == '|')) {

                char token[3] = {*p, *p, '\0'};
                tokens[(*count)++] = strdup2(token);
                p += 2;
            }
            else if (*p == '>' && *(p+1) == '>') {

                tokens[(*count)++] = strdup2(">>");
                p += 2;
            } else {

                char token[2] = {*p, '\0'};
                tokens[(*count)++] = strdup2(token);
                p++;
            }
            continue;
        }

        char token[1024] = {0};
        int t = 0;

        if (*p == '"' || *p == '\'') {

            char quote = *p++;
            while (*p && *p != quote)
                token[t++] = *p++;
            if (*p == quote) 
                p++;
        } else {
            while (*p && !isspace(*p) &&
                   *p != ';' && *p != '&' && *p != '|' &&
                   *p != '<' && *p != '>' && *p != '(' && *p != ')') {
                if (*p == '*' && *(p+1) == ')') {
                    token[t++] = *p++;
                    token[t++] = *p++;
                    break;
                }
                token[t++] = *p++;
            }
        }
        token[t] = '\0';
        tokens[(*count)++] = strdup2(token);
    }

    tokens[*count] = NULL;

    return tokens;
}


command_t *
parse_input (char *input) 
{
    int count = 0;
    char **tokens = tokenize (input, &count);

    // printf (0, "count = %d\n", count);
    // for (int i=0; i<count; i++)
    //     printf (0, "token = %s\n", tokens [i]);

    return nullptr;

}