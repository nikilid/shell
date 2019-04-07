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
    int c = 1;
    int count = 0;
    char c_prev = '|';
    cmd_list *cmd_first; 
    char *tmp;
    int k = 2;
    int i = 0;
    int j = 0;
    int is_name_cmd = 0;
    int is_first_cmd = 1;
    int screened_space = 0;
    int double_quotes = 0;
    int single_quotes = 0;
    int screened_quotes = 0;
    int is_prev_newline = 0;
    cmd *new_cmd;
    cmd_list *new_elem;
    while ((c = getchar()) != EOF){
        if (is_first_cmd && c == '\n')
            continue;
        screened_quotes = 0;
        is_prev_newline = 0;
        if (c == '|' && !single_quotes && !double_quotes){
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
                is_prev_newline = 1;
            }
        }
        if (c == '\\' ){
            int c_h = getchar();
            if (c_prev != '|')
                c_prev = c;
            if ((!single_quotes && !double_quotes) || (single_quotes && c_h == '\'') || (double_quotes && c_h == '"' )){
                if (c_h == ' '){
                    screened_space = 1;
                } else if (c_h == '\n'){
                    if (c_prev != '|')
                        c_prev = c_h;
                    continue;
                }
                c = c_h;
                screened_quotes = 1; 
            } else if ((!single_quotes && c_h == '\'') || (!double_quotes && c_h == '"')){
                tmp[i++] = c_prev;
                c = c_h;
                screened_quotes = 1;
            }
        }
        if ((!double_quotes && !single_quotes) && (c == ' ' && c_prev == ' ')){
            continue;
        }
        if ((c == '"' && !single_quotes) || (c == '\'' && !double_quotes)) {
            if ((!double_quotes && c == '"') || (!single_quotes && c == '\'')){
                if (c_prev == ' '  || screened_quotes) {
                    c_prev = c;
                    if (c == '"'){
                        double_quotes = 1;
                    } else {
                        single_quotes = 1;
                    }
                    k = 2;
                    i = 0;
                    tmp = (char*)calloc(k,sizeof(char)); 
                }
                if (!screened_quotes)
                    continue;
            } else {
                c_prev = c;
                int c_h = getchar();
                if ((c_h == ' ' || (c_h == '"'  )  || c_h == '\n' || c_h == '\\') &&  c == '"'){
                    
                        double_quotes = 0;
        
                } 
                  if ((c_h == ' ' || (c_h == '\''  )  || c_h == '\n' || c_h == '\\') &&  c == '\'') {
                    
                        single_quotes = 0;
        
                } 
                if (screened_quotes){
                    tmp[i++] = c;
                }
                if ((c_h == '"' && c == '"') || (c_h == '\'' && c== '\'')){
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
        if (c_prev == '|' && !double_quotes && !single_quotes){
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
            is_name_cmd = 1;
            if (is_first_cmd){
                cmd_first = new_elem;
                is_first_cmd = 0;
            }
        } else if (( (c == ' ' && !screened_space ) ||  c == '\n' || c =='#' || is_prev_newline)&& !double_quotes && !single_quotes ){
            if (c_prev != ' '){
                c_prev = (char)(c);
                tmp[i] = '\0';
                tmp = realloc(tmp, i+1);
                if (is_name_cmd){
                    new_cmd->name =(char*)calloc(i+1, sizeof(char));
                    strncpy(new_cmd->name,tmp, i+1);
                    new_cmd->name[i] = '\0';
                    is_name_cmd = 0;
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
            if (c_prev == ' ' && !double_quotes && !single_quotes && !screened_space){
                tmp = (char*)calloc(k,sizeof(char)); 
            } else if (k == i){
                k *= 2;
                tmp = (char*)realloc(tmp, k*sizeof(char));
            }
            tmp[i++] = (char)(c);
            c_prev = (char)(c);
        }   
    }
    *cmd_l = cmd_first;
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
        } else {
            analyse(cm, fd);
            execvp(cm->name, cm->argv);
            exit(1);
            return 0;
        }
    }           
    int pid = 0;
    pid = fork();
    if (pid > 0) {
        waitpid(pid,&st, 0);
    } else {
        analyse(cm, fd);
        execvp(cm->name, cm->argv);
        exit(1);
    }
    return (WIFEXITED(st) && !WEXITSTATUS(st));
        
}

int main (int argc, char** argv)
{
    int c;
    int st, count = 0;
    char c_prev = '|';
    cmd_list **cmd_l;
    cmd_list *cmd_first;
    cmd_l = (cmd_list**)malloc(sizeof(cmd_list*)); ;
    *cmd_l = NULL;  
    uint32_t count_pipe = read_cmd(cmd_l);
      int stdin_copy = dup(0);
    int stdout_copy = dup(1);
    while (count_pipe != 0){
        cmd_list *next = *cmd_l;
        count_pipe -= 1;
        int fd[2];
        int fd_read_prev = -1;
        for (int i = 0; i <= count_pipe; i++) {
             if (count_pipe == 0 && !(strcmp(next->current_cmd->name, "cd"))){
                if (chdir(next->current_cmd->argv[1])==-1){
                    perror(next->current_cmd->argv[1]);
                }
             } else {
                if (i != count_pipe) {
                    pipe(fd);
                }
                if (!fork()) {
                    if (i > 0) {
                        dup2(fd_read_prev, 0);
                        close(fd_read_prev);
                    }
                    if (i < count_pipe) {
                        dup2(fd[1], 1);
                        close(fd[0]);
                        close(fd[1]);
                    }
                    logical_an(next->current_cmd, fd);
                    exit(1);
                }
                if (i > 0) {
                    close(fd_read_prev);
                }
                if (i < count_pipe) {
                    close(fd[1]);
                }
                next = next->prev;
                fd_read_prev = fd[0];
            }
            for (int i = 0; i <= count_pipe; i++) {
                wait(NULL);
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
