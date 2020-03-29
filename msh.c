//  MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 8
# define STDIN_FILENO 0
# define STDOUT_FILENO 1
# define STDERR_FILENO 2


// ficheros por si hay redirección
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
    printf("****  Saliendo del MSH **** \n");
    //signal(SIGINT, siginthandler);
    exit(0);
}

/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
    //reset first
    for(int j = 0; j < 8; j++)
        argv_execvp[j] = NULL;

    int i = 0;
    for ( i = 0; argvv[num_command][i] != NULL; i++)
        argv_execvp[i] = argvv[num_command][i];
}

//******************* MANDATOS INTERNOS ************************



/**
 * Main sheell  Loop
 */
int main(int argc, char* argv[])
{
    /**** Do not delete this code.****/
    int end = 0;
    int executed_cmd_lines = -1;
    char *cmd_line = NULL;
    char *cmd_lines[10];

    if (!isatty(STDIN_FILENO)) {
        cmd_line = (char*)malloc(100);
        while (scanf(" %[^\n]", cmd_line) != EOF){
            if(strlen(cmd_line) <= 0) return 0;
            cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
            strcpy(cmd_lines[end], cmd_line);
            end++;
            fflush (stdin);
            fflush(stdout);
        }
    }

    /*********************************/

    char ***argvv = NULL;
    int num_commands;


    while (1) {
        int status = 0;
        int command_counter = 0;
        int in_background = 0;
        signal(SIGINT, siginthandler);

        // Prompt
        write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

        // Get command
        //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
        executed_cmd_lines++;
        if (end != 0 && executed_cmd_lines < end) {
            command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
        } else if (end != 0 && executed_cmd_lines == end)
            return 0;
        else
            command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
        //************************************************************************************************


        /************************ STUDENTS CODE ********************************/
        if (command_counter > 0) {
            if (command_counter > MAX_COMMANDS) {
                printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
                exit(-1); // NECESITAMOS ESTE EXIT? O MEJOR UN RETURN?
            }

            else { 
            int fd[2];
            int pid;
            int savestdin = dup(STDIN_FILENO); // guardo la entrada estandar en el primer fd vacio
            int savestdout = dup(STDOUT_FILENO); // guardo la salida estandar en el primer fd vacio
            for(int i = 0; i < command_counter; i++){
                pipe(fd);  // creo la tubería (posterior)
                if(i == command_counter-1){ //ÚLTIMO HIJO 
                    close(fd[1]);
                }
                else{ // DEMÁS HIJOS
                    dup2(fd[1], STDOUT_FILENO); //Dejo la escritura de la nueva pipe en la salida (y la cierro)
                    close(fd[1]);               // No cierro aquí la lectura de la tubería porque la necesita el padre para concatenarla al siguiente hijo
                }

                pid = fork();
                if(pid == 0){
                    close(fd[0]); // La entrada del hijo es la lectura de la tubería anterior (no necesita la de la nueva, la cierra) 
                    execvp(argvv[i][0], argvv[i]); // En este punto el hijo lee de la tubería de la iteración anterior y escribe en la actual 
                    break;
                } else if (pid < 0){
                    perror("Error el el fork");
                    exit(-1);
                } else{ 
                    wait(&status); // espera que el hijo acabe antes de crear el siguiente
                    dup2(fd[0], STDIN_FILENO); // Dejo la entrada/lectura de la tubería actual en la entrada (del siguiente hijo que creemos)
                    dup2(savestdout, STDOUT_FILENO); // Dejo la salida estándar en su sitio (1)
                    close(fd[0]);
                }

            }// for
            dup2(savestdin, STDIN_FILENO);
            close(savestdin);
            close(savestdout);

            

            }// else

        } // cierro if command > 0
    } // cierro while
    return 0;
} //cierro main

