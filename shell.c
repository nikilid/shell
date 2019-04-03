#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct cmd
{
    char *name;
    char **argv;
    int argc;
} cmd;

struct cmd_list;
typedef struct cmd_list
{
    cmd *current_cmd;
    struct cmd_list *next;
    struct cmd_list *prev;
} cmd_list;

uint32_t read_cmd(cmd_list** cmd_l)
{
    int c;
    int count = 0;
    char c_prev = '|';
    cmd_list *cmd_last; 
    char *tmp;
    int k = 2;
    int i = 0;
    int j = 0;
    int flag = 0;
    int flag1 = 1;
    int flag_im = 0;
    int flag_quotes = 0;
    int flag_quotes_one = 0;
    int flag_escape = 0;
    int flag_enter = 0;
    int count_open = 0;
    int count_open_one = 0;
    cmd *new_cmd;
    cmd_list *new_elem;
    while ((c = getchar()) != EOF){
        if (flag1 && c == '\n')
            continue;
        flag_escape = 0;
        flag_enter = 0;
        if (c == '|' && !flag_quotes_one && !flag_quotes){
            if (c_prev != '\n'){
                c_prev = c;
                c = getchar();
                if (c == '|'){
                    tmp = (char*)calloc(k,sizeof(char)); 
                    tmp[i++] = '|';
                    tmp[i++] = '|';
                    c_prev = 'a';
                    continue;
                }
            } else {
                flag_enter = 1;
            }
        }
        if (c == '\\' ){
            int c_h = getchar();
            if (c_prev != '|')
                c_prev = c;
            if (!flag_quotes_one && !flag_quotes || flag_quotes_one && c_h == '\'' || flag_quotes && c_h == '"' ){
                if (c_h == ' '){
                    flag_im = 1;
                } else if (c_h == '\n'){
                    if (c_prev != '|')
                        c_prev = c_h;
                    continue;
                }
                c = c_h;
                flag_escape = 1; 
            } else if (!flag_quotes_one && c_h == '\'' || !flag_quotes && c_h == '"'){
                tmp[i++] = c_prev;
                c = c_h;
                flag_escape = 1;
            }
        }
        if (!flag_quotes && !flag_quotes_one && c == ' ' && c_prev == ' '){
            continue;
        }
        if (c == '"' && !flag_quotes_one) {
            if (!flag_quotes){
                if (c_prev == ' '  || flag_escape) {
                    c_prev = c;
                    count_open++;
                    flag_quotes = 1;
                    k = 2;
                    i = 0;
                    tmp = (char*)calloc(k,sizeof(char)); 
                }
                if (!flag_escape)
                    continue;
            } else {
                c_prev = c;
                int c_h = getchar();
                if (c_h == ' ' || c_h == '"' || '\n') {
                    if (c_h == '"'){
                        count_open--;
                    }
                    count_open--;
                    if(count_open == 0){
                        flag_quotes = 0;
                    }
                } else {
                    count_open++;
                }
                if (flag_escape){
                    tmp[i++] = c;
                }
                if (c_h == '"' ){
                        continue;
                }
                if (c_h == '\\'){
                    c_h = getchar();
                    if (c_h == '\n'){
                        c_prev = c_h;
                        continue;
                    }
                }
                c = c_h;         
            }
        } 
        if (c == '\'' && !flag_quotes) {
            if (!flag_quotes_one){
                if (c_prev == ' '  || flag_escape) {
                    c_prev = c;
                    count_open_one++;
                    flag_quotes_one = 1;
                    k = 2;
                    i = 0;
                    tmp = (char*)calloc(k,sizeof(char)); 
                }
                if (!flag_escape)
                    continue;
            } else {
                int c_h = getchar();
                if (c_h == ' ' || c_h == '\'' || '\n' || c_h == '\\') {
                    if (c_h == '\''){
                        count_open_one--;
                    }
                    count_open_one--;
                    if(count_open_one == 0){
                        flag_quotes_one = 0;
                    }
                } else {
                    count_open_one++;
                }
                if (flag_escape){
                    tmp[i++] = c;
                }
                if (c_h == '\'' ){
                    continue;
                }
                if (c_h == '\\'){
                    c_h = getchar();
                    if (c_h == '\n'){
                        c_prev = c_h;
                        continue;
                    }
                }
                c = c_h;     
            }
        }
        if (c_prev == '|' && !flag_quotes && !flag_quotes_one){
            new_cmd = (cmd *)malloc(sizeof(cmd));
            new_elem = (cmd_list *)malloc(sizeof(cmd_list));
            memset(new_cmd, 0, sizeof(cmd));
            memset(new_elem, 0, sizeof(cmd_list));
            (new_cmd->argv) = (char **)malloc(sizeof(char*)); 
            *(new_cmd->argv) = NULL;
            new_cmd->argc = 0;
            new_elem->current_cmd = new_cmd;
            count++;
            new_elem->prev = NULL;
            new_elem->next = *cmd_l;
            j = 0;
            if (new_elem->next != NULL){
                new_elem->next->prev = new_elem;
            }
            *cmd_l = new_elem;
            if (c != ' ' ){
                tmp = (char*)calloc(k,sizeof(char)); 
                tmp[i++] = (char)(c);
            }
            c_prev = (char)(c);
            flag = 1;
            if (flag1){
                cmd_last = new_elem;
                flag1 = 0;
            }
        } else if ((c == ' ' && !flag_im  ||  c == '\n' || c =='#' || flag_enter)&& !flag_quotes && !flag_quotes_one ){
            if (c_prev != ' '){
                c_prev = (char)(c);
                tmp[i] = '\0';
                tmp = realloc(tmp, i+1);
                if (flag){
                    new_cmd->name =(char*)calloc(i+1, sizeof(char));
                    strncpy(new_cmd->name,tmp, i+1);
                    new_cmd->name[i] = '\0';
                    flag = 0;
                } 
                if (j != 0){
                    (new_cmd->argv) = (char**)realloc(new_cmd->argv,(j+1)*sizeof(char*));// );
                }
                (new_cmd->argv)[j] = (char*)calloc(i+1, sizeof(char));         
                strncpy((new_cmd->argv)[j], tmp, i+1);
                free(tmp);
                tmp = NULL;
                j++;
                ++(new_cmd->argc);
                i = 0;
                k = 2;
            }
            if (c== '\n' || c == '#'){
                (new_cmd->argv) = (char**)realloc(new_cmd->argv,(j+1)*sizeof(char*));
                (new_cmd->argv)[j] = NULL;
                break;
            }
        } else {
            if (c_prev == ' ' && !flag_quotes && !flag_quotes_one && !flag_im){
                tmp = (char*)calloc(k,sizeof(char)); 
            } else if (k == i){
                k *= 2;
                tmp = (char*)realloc(tmp, k*sizeof(char));
            }
            tmp[i++] = (char)(c);
            c_prev = (char)(c);
        }   
    }
    *cmd_l = cmd_last;
    return count; 
}

void analyse (cmd *cm, int fd[])
{
    int end = cm->argc+1;
    int flag = 0;
    for (int i = 0; i < cm->argc; i++){
        if (!(strcmp(cm->argv[i],">"))){
            if (i + 1 < cm->argc){
                int d = creat(cm->argv[i+1], 0777);
                dup2(d, 1);
                close(fd[0]);
                close(fd[1]);
                close(d);
                flag = 1;
            }
        } else if (!(strcmp(cm->argv[i],">>"))){
            if (i + 1 < cm->argc){
                int d = open(cm->argv[i+1], O_WRONLY | O_APPEND);
                dup2(d, 1);
                close(fd[0]);
                close(fd[1]);
                close(d);
                flag = 1;   
            }
        } else if (!(strcmp(cm->argv[i],"<"))){
            if (i + 1 < cm->argc){
                int d = open(cm->argv[i+1], O_RDONLY);
                dup2(d, 0);
                close(fd[0]);
                close(fd[1]);
                close(d);
                flag = 1;               
            }
        }
        if (flag){
            cm->argc = end - 3;
            for (int j = i; j < cm->argc; j++){
                cm->argv[j] = cm->argv[j+2];
            }
            cm->argv[end-1] = NULL;
            cm->argv[end-2] = NULL;
            cm->argv[end-3] = NULL;
            flag = 0;
        }
    }
    
}

int logical_an (cmd *cm, int fd[])
{
    int st;
    int stat_prev_or = 0;
    int stat_prev_and = 1;
    int flag = 0;
    int res_or, res_and;
    for (int i = cm->argc - 1; i >= 0; i--){
        flag = !(strcmp(cm->argv[i],"||"));
        if (flag){
            flag = 1;
        } else {
            flag = !(strcmp(cm->argv[i],"&&"));
            if (flag){
                flag = 2;
            } else {
                flag = !(strcmp(cm->argv[i],"&"));
                if (flag){
                    flag = 3;
                }
            }
        }
        if (flag != 0) {
            char **mas = NULL;
            mas = malloc((i + 1)*sizeof(*mas));
            for (int j = 0; j < i; j++){
                mas[j] = cm->argv[j];
            }
            mas[i] = NULL;
            cmd *cm_new = (cmd *)malloc(sizeof(cm_new));
            cm_new->name = cm->name;
            cm_new->argv = mas;
            cm_new->argc = i;
            switch (flag) {
                case 1:
                    res_or = logical_an(cm_new, fd);
                    if (res_or){
                        return 1;
                    }
                    break;
                case 2:
                    res_and = logical_an(cm_new, fd);
                    if (!res_and){
                        return 0;
                    }
                    break;
                case 3:
                    if (!fork()) {
                        logical_an(cm_new, fd);
                        exit(1);
                    }   
                    break;
            }
            cm->name = cm->argv[i+1];
            cm->argv = &(cm->argv[i+1]);
            cm->argc -= (i+1);
            i = cm->argc;
        } 
    }           
    int pid = 0;
    if (pid = fork()) {
        waitpid(pid,&st, 0);
    } else {
        analyse(cm, fd);
        if (!strcmp(cm->name, "cd")){
            if (chdir(cm->argv[1])==-1){
                perror(cm->argv[1]);
            }
            exit(1);
        } 
        execvp(cm->name, cm->argv);
        exit(1);
    }
    return (WIFEXITED(st) && !WEXITSTATUS(st));
        
}

int main (int argc, char** argv)
{
    int c;
    int st, count=0;
    char c_prev = '|';
    cmd_list **cmd_l;
    cmd_list *cmd_last;
    cmd_l = (cmd_list**)malloc(sizeof(cmd_list*)); ;
    *cmd_l = NULL;  
    uint32_t count_pipe = read_cmd(cmd_l);
    while (count_pipe != 0){
        cmd_list *next = *cmd_l;
        count_pipe -= 1;
        int fd[2];
        for (int i = 0; i < count_pipe; i++){
            pipe(fd);
            if (!fork()) {
                dup2(fd[1], 1);
                close(fd[1]);
                close(fd[0]);
                logical_an(next->current_cmd, fd);
                exit(1);
            } 
            dup2(fd[0], 0);
            close(fd[1]);
            close(fd[0]);
            next = next->prev;
        }
        if (!fork()) {
            logical_an(next->current_cmd, fd);
            exit(1);
        } 
        while(wait(NULL) != -1);
        if (count_pipe == 0 && !(strcmp(next->current_cmd->name, "cd"))  ){
            if (chdir(next->current_cmd->argv[1])==-1){
                perror(next->current_cmd->argv[1]);
            }
        }  
        cmd_list *del = *cmd_l;
        for (int i = 0; i <= count_pipe; i++){
            free(del);
            del = del->prev;
        }
        *cmd_l = NULL;
        count_pipe = read_cmd(cmd_l);
    }  
    free (cmd_l);
    return 0;
}
