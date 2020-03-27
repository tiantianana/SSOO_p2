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
            if (command_counter > MAX_COMMANDS)
                printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
            else if (command_counter == 1) {
                //printf("Numero de comandos introducidos: %d \n", command_counter);
                // Print command
                //print_command(argvv, filev, in_background);

                int pid;
                int result;
                pid = fork(); //Creamos un hijo para la shell

                switch (pid) {
                    case -1: //Error
                        perror("Error el el fork");
                        exit(-1);
                    case 0:  //Hijo 1
                        execvp(argvv[0][0], argvv[0]);
                        perror("Error en el exec");
                        break;
                    default: //Padre
                        //MANDATOS INTERNOS
                        /**
                        result = strcmp(argvv[0][0], "mycalc");
                        if(result == 0) {
                            my_calc(argvv[1][0], argvv[2][0], argvv[3][0]);
                        }
                        **/

                        //background
                        if (in_background == 0) {
                            //proceso sin background
                            while (wait(&status) != pid);
                        } else {
                            // Proceso en background
                            printf("[1] %d \n", getpid());
                        }
                } //Cierro switch command 1

            } else if (command_counter == 2) {
                int pid;     //variable pid
                int status;  //variable de estado
                int fd[2];  //variable para la tuberia
                pipe(fd);   //tuberia
                pid = fork(); //Creamos un proceso hijo
                switch (pid) { // switch caso hijo 1
                    case -1: //Error
                        perror("Error el el fork");
                        exit(-1);
                    case 0: //Hijo
                        close(STDOUT_FILENO); //cerramos la salida
                        dup(fd[1]);
                        close(fd[0]);
                        close(fd[1]);
                        execvp(argvv[0][0], argvv[0]);

                    default: //padre
                        pid = fork(); //creamos otro proceso hijo
                        switch (pid) {  //vemos el hijo 2
                            case -1:   //Error en el fork del mandato 2
                                perror("Error el el fork");
                                exit(-1);
                            case 0:   //Hijo 2
                                close(STDIN_FILENO);//cerramos la entrada
                                dup(fd[0]);//duplicamos
                                close(fd[0]);//cerramos
                                close(fd[1]);//cerramos
                                execvp(argvv[1][0], argvv[1]);

                            default: //padre
                                close(fd[0]);//cerramos
                                close(fd[1]);//cerramos

                                if (in_background == 0) {
                                    //proceso sin background
                                    while (wait(&status) != pid);
                                } else {
                                    // Proceso en background
                                    printf("[1] %d \n", getpid());
                                }
                        } // Cierro Switch caso hijo
                }//Switch caso hijo 1


            } else if (num_commands == 3) {
                int pid;    //identificador
                int status; // variable estado
                int fd[2]; //variable tuberia 1
                int fd2[2]; //variable tuberia 2
                pipe(fd);   //tuberia
                pid = fork(); //proceso hijo 1

                switch (pid) {
                    case -1:   //Error en el fork del mandato 1
                        perror("Error el el fork");
                        exit(-1);
                    case 0:   //Hijo 1
                        close(STDOUT_FILENO);//cerramos la salida estandar
                        dup(fd[0]);//duplicamos
                        close(fd[0]);//cerramos
                        close(fd[1]);//cerramos
                        execvp(argvv[0][0], argvv[0]);
                        perror("Error en el execvp del primer mandato\n");
                        return -1;
                    default:
                        pipe(fd2); //tuberia 2
                        pid = fork(); //proceso hijo 2
                        switch (pid) {
                            case -1:   //Error en el fork del segundo mandato
                                perror("Error el el fork");
                                exit(-1);
                            case 0:   //Hijo 1
                                close(STDIN_FILENO);//cerramos la entrada estandar
                                dup(fd[0]);//duplicamos
                                close(STDOUT_FILENO);//cerramos la salida estandar
                                dup(fd2[1]);//duplicamoss
                                close(fd[0]);//cerramos
                                close(fd[1]);//cerramos
                                close(fd2[0]);//cerramos
                                close(fd2[1]);//cerramos
                                execvp(argvv[1][0], argvv[1]);
                                perror("Error en el execvp del segundo mandato\n");
                                return -1;
                            default: //padre
                                close(fd[0]); //cerramos
                                close(fd[1]); //cerramos
                                pid = fork(); //proceso hijo 3
                                switch (pid) { //casos del hijo 3
                                    case -1:  //Error en el fork del tercer mandato
                                        perror("Error el el fork");
                                        exit(-1);
                                    case 0:
                                        close(STDIN_FILENO);//cerramos la entrada estandar
                                        dup(fd2[0]);//duplicamos
                                        close(fd2[0]);//cerramos
                                        close(fd2[1]);//cerramos
                                        execvp(argvv[2][0], argvv[2]);
                                        perror("Error en el execvp del tercer mandato\n");
                                    default: //padre
                                        close(fd2[0]);//cerramos
                                        close(fd2[1]);//cerramos
                                        if (in_background == 0) {
                                            //proceso sin background
                                            while (wait(&status) != pid);
                                        } else {
                                            // Proceso en background
                                            printf("[1] %d \n", getpid());
                                        }
                                } // Fin switch casos del hijo 3
                        }// Fin switch casos del hijo 2
                } // Fin switch casos del hijo 1
            } // primer else if

        } // cierro if command > 0
    } // cierro while
	return 0;
} //cierro main

//my_calc(char  )
