#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZE 1024
#define BUFSIZE_SPLIT 64
#define DELIMITATOR " \t\r\n\a"

char * read_line(){ // am folosit metoda asta pentru ca, in caz ca se trece de spatiul alocat sa putem realoca memorie oricand.
    int bufsize = BUFSIZE;
    int position = strlen("dbxcli ");
    char * buffer = malloc(sizeof(char) * bufsize);
    strcpy(buffer, "dbxcli ");
    int c;
    
    if (!buffer){
        fprintf(stderr, "Allocation error\n"); //stderr si fprintf pentru a afisa mesajul ca pe o eroare.
        exit(EXIT_FAILURE);                    
    }

    while(1){
        //citim cate un caracter pe rand si il stocam ca pe un INT pentru a putea verifica daca am ajuns la sf linie. (EOF e intreg!)
        c = getchar();

        //Daca ajungem la sfarsitul liniei sau se apasa enter, marcam sf. vect. de caractere cu \0 si returnam
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        }else{  // altfel adaugam caracterul in string.
            buffer[position] = c;
        }
        position++;

        //Verificam daca depasim spatiul alocat bufferului, realocam (ii crestem memoria)
        if (position >= bufsize){  
            bufsize += BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char ** split_line(char * linie){ // Aici parsam string-ul intr-o lista de comenzi.
    int bufsize = BUFSIZE_SPLIT, position = 0;  //Cu o mica specificatie, nu putem folosii "" sau /, folosim spatii pentru a delimita argumentele intre ele.
    char ** tokens = malloc(bufsize* sizeof(char*)); //De exemplu, comanda: mkdir test va fi vazuta ca 2 argumente -> mkdir si test.
    char * token;

    if (!tokens){
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(linie, DELIMITATOR);
    while(token != NULL){
        tokens[position] = token;
        position++;
        
        if(position >= bufsize){ //La fel, daca am depasit memoria alocata deja, realocam.
            bufsize += BUFSIZE_SPLIT;
            tokens = realloc(tokens, bufsize*sizeof(char*));
            if (!tokens){
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, DELIMITATOR);
    }
    tokens[position] = NULL;
    return tokens;
}

int launch(char ** args){
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid < 0){
        perror("Error");
    }
    else if (pid == 0){
        //Child code;
        if (execvp(args[0], args) == -1){ //asteapta numele programului si o lista de argumente
            perror("Error");              //'p' inseamna ca, in loc sa specificam calea absoluta (ca in cazul lui execve)
        }                                 //specificam doar numele comenzii, iar sistemul face toata treaba.
        exit(EXIT_FAILURE);
    }else{
        //Parent code;
        do {
            wpid = waitpid(pid, &status, WUNTRACED); //waitpid pt a astepta ca statusul procesului sa se schimbe, adica asteptam pana cand fie a iesit fie a fost "omorat"
        }while(!WIFEXITED(status) && !WIFSIGNALED(status)); //wuntraced returneaza daca un copil s-a oprit. Statusul copiilor este urmarit chiar daca aceasta optiune nu este specificata 
    }   //wifexited pentru a stii statusul de exit al unui copil (true sau false)
        //wifsignaled verifica daca un proces copil a iesit deoarece s-a ridicat un semnal care l-a determinat sa iasa.

    return 1;
}

//Built in commands:

int s_cd(char ** args);
int h(char ** args);
int s_help(char ** args);
int s_exit(char ** args);
int clear(char ** args);
int s_cp(char ** args);
int move(char ** args);

char * builtin_str[] = { "cd", "help", "-h", "exit", "clear", "cp", "move"}; //builtin command names.

int (*builtin_func[])(char **) = { &s_cd, &s_help, &h,&s_exit, &clear, &s_cp, &move}; //vector de functii pentru fiecare nume de comanda

int num_builtins(){
    return sizeof(builtin_str) / sizeof(char*); //numarul functiilor deja implem
}

int s_cd (char ** args){
    printf("\nCD command does not work here.\n");
    /*
    if (args[1] == NULL){
        fprintf(stderr, "Expected argument to \"cd\"\n");
    }else{
        if (chdir(args[1]) != 0){
            perror("Error");
        }
    }*/
    return 1;
}

int s_help(char ** args){
    int i;
    printf("Use dbxcli to quickly interact with your Dropbox, upload/download files, \n");
    printf("manage your team and more. It is easy, scriptable and works on all platforms!\n");
    printf("\nAvailable commands: \n");
    
    printf("  account         Display account information\n");
    printf("  cp              Copy a file or folder to a different location in the user's Dropbox. If the source path is a folder all its contents will be copied.\n");
    printf("  du              Display usage information\n");
    printf("  get             Download a file\n");
    printf("  help            Help about any command\n");
    printf("  logout          Log out of the current session\n");
    printf("  ls              List files and folders\n");
    printf("  mkdir           Create a new directory\n");
    printf("  mv              Move files\n");
    printf("  put             Upload files\n");
    printf("  restore         Restore files\n");
    printf("  revs            List file revisions\n");
    printf("  rm              Remove files\n");
    printf("  search          Search\n");
    printf("  share           Sharing commands\n");
    printf("  team            Team management commands\n");
    printf("  version         Print version information\n");
    printf("  clear           Clear the terminal screen\n");
    printf("  exit         Normal process termination\n");

    printf("\nFlags:\n");
    printf("    --as-member string  Member ID to perform action as\n");
    printf("-h, --help              help for dbxcli\n");
    printf("-v, --verbose           Enable verbose logging\n");

    printf("\nUse [command] --help for more information about a command\n");
    return 1;
}

int h(char ** args){
    s_help(args);
}

int s_exit(char ** args){
    return 0;
}

int l_execute(char ** args){
    int i;
    if (args[0] == NULL){
        //o comanda goala a fost introdusa.
        return 1;
    }

    for (i = 0; i<num_builtins(); i++){
        if (strcmp(args[1], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }

    return launch(args);
}

void loop(void){
    char * linie;
    char ** args;
    int status;

    do { //am facut un do-while pentru a procesa cel putin odata argumentele, iar in functie de status sa se opreasca sau sa continue.
        printf("\ndbxcli> ");
        linie = read_line(); //citeste un string din input
        args = split_line(linie); //separa string-ul intr-un program/argumente
        status = l_execute(args); //ruleaza comanda primita ca parametru

        free(linie);
        free(args);
    }while(status);
}

int s_cp(char ** args){
    //dbxcli cp test.c path

    char put[] = "put";
    char cp[] = "cp";
    char dbxcli[] = "dbxcli";
    char ** tokens = malloc(BUFSIZE * sizeof(char*));
    char ** tokens1 = malloc(BUFSIZE * sizeof(char*));
    tokens[0] = dbxcli;
    tokens[1] = put;
    tokens[2] = args[2];

    tokens1[0] = dbxcli;
    tokens1[1] = cp;
    tokens1[2] = args[2];
    tokens1[3] = args[3];

    if (args[2] == NULL || args[3] == NULL){
        printf("Expected arguments to \"cp\"\n");
    } else{
        launch(tokens);
        launch(tokens1);
    }

    free(tokens);
    free(tokens1);

    return 1;
}

int move(char ** args){
    //dbxcli mv fisier dest
    char put[] = "put";
    char mv[] = "mv";
    char dbxcli[] = "dbxcli";
    char ** tokens = malloc(BUFSIZE * sizeof(char*));
    char ** tokens1 = malloc(BUFSIZE * sizeof(char*));
    tokens[0] = dbxcli;
    tokens[1] = put;
    tokens[2] = args[2];

    tokens1[0] = dbxcli;
    tokens1[1] = mv;
    tokens1[2] = args[2];
    tokens1[3] = args[3];

    if (args[2] == NULL || args[3] == NULL){
        printf("Expected arguments to \"cp\"\n");
    } else{
        launch(tokens);
        launch(tokens1);
    }

    free(tokens);
    free(tokens1);

    return 1;
}

int clear(char ** args){
    system("clear");
    loop();
}

int main(int argc, char ** argv){
    printf("Custom terminal loaded succesfuly!");
    loop();

    return EXIT_SUCCESS;
}