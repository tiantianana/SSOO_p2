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
int my_calc(char *op1, char *operador, char *op2){
    // COMPROBAR que los argumentos no son null/ vacíos ***************
    int Acc = 0; // variable de retorno
    int num1 = atoi(op1); // convertimos op1 y op2 a int
    int num2 = atoi(op2);
    /* Pruebas para ver que imprime cada número
    printf("numero 1: %d \n", num1);
    printf("numero 2: %d \n", num2);
    printf("operador: %s \n", operador);
    */
    if(strcmp(operador,"add") == 0){ // CASO ADD
        /* char mensaje[] = {"[OK] %d", Acc}
        write(STDERR_FILENO, mensaje, ); antigua solución, creemos que vale con un perror pq imprime un msg por stderr*/
        Acc = num1 + num2;
        // SI FUNCIONA LA SUMA: Escribir en la salida estandar de error el mensaje: [OK] num1 + num2 = Acc; Acc Acc
        //char *resultado;
        //resultado = ("[OK] %d + %d = %d; Acc %d ", num1, num2, Acc, Acc);
        printf("[OK] %d + %d = %d; Acc %d \n", num1, num2, Acc, Acc);
        //perror(resultado);
        // SI NO FUNCIONA: Escribir el resultado en la salida estandar el mensaje: [ERROR] -> AÑADIR ESTA OPCION EN LAS COMPROBACIONES
        exit(0);
    }

    else if(strcmp(operador, "mod") == 0){ //CASO MOD
        else if(strcmp(operador, "mod") == 0){ //CASO MOD
        int num1 = atoi(op1); // convertimos op1 y op2 a int
        int num2 = atoi(op2);
        int cociente = num1/num2;
        int resto = num1%num2;
        printf("[OK] %d %% %d = %d * %d + %d \n", num1, num2, num2, resto, cociente);
        exit(0);
    }
        
    else{ // OPERADOR ERRÓNEO
        perror("ERROR: no se encuentra la operación especificada \n");
        exit(-1);
    }
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
            if (command_counter > MAX_COMMANDS) {
                printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
                exit(-1); // NECESITAMOS ESTE EXIT? O MEJOR UN RETURN?
            }

            int fich;
                /****** SECUENCIAS DE COMANDOS **********/

           if (command_counter == 1) { //UN SOLO COMANDO
                //printf("Numero de comandos introducidos: %d \n", command_counter);
                //print_command(argvv, filev, in_background);

               /*************** MANDATOS INTERNOS ***************/
                             if(strcmp(argvv[0][0], "mycalc") == 0) { // argvv[0][0] es mycalc
                                 my_calc(argvv[0][1], argvv[0][2], argvv[0][3]);
                             }
                             else if(strcmp(argvv[0][0], "mycp") == 0) { // argvv[0][0] es mycp
                                 //my_cp(argvv[0][1], argvv[0][2]);
                             }


               int pid;
                int result;
                pid = fork(); //Creamos un hijo para la minishell
                switch (pid) {
                    case -1: //Error
                        perror("Error el el fork");
                        exit(-1);
                    case 0:  //Hijo 1

                        /****** REDIRECCIONES *****/
                        if(strcmp(filev[0], "0") != 0){ // redirección de entrada
                            printf("filev[0] = %s \n", filev[0]);
                            fich = open(filev[0], O_RDONLY);
                            if(fich<0){
                                perror("Error al abrir el fichero especificado");
                                exit(-1);
                            }
                            dup2(fich, STDIN_FILENO);
                            close(fich);
                        }

                        else if(strcmp(filev[1], "0") != 0){ // redirección de salida - escribimos la salida de la minishell (STDOUT_FILENO) en el fichero especificado
                            fich = open(filev[1],O_CREAT|O_WRONLY, 0666);
                            if(fich<0){
                                perror("Error al abrir el fichero especificado");
                                exit(-1);
                            }
                            dup2(fich, STDOUT_FILENO);
                            close(fich);
                        }

                        else if(strcmp(filev[2], "0") != 0){ // redirección de salida error
                            fich = open(filev[2], O_WRONLY);
                            if(fich<0){
                                perror("Error al abrir el fichero especificado");
                                exit(-1);
                            }
                            dup2(fich, STDERR_FILENO);
                            close(fich);
                        }

                        execvp(argvv[0][0], argvv[0]);
                        perror("Error en el exec");
                        break;

                    default: //Padre

                        /*************** BACKGROUND ***************/

                        if (in_background == 0) { //proceso sin background
                            while (wait(&status) != pid);
                        }

                        else {  // Proceso en background
                            printf("[1] %d \n", getpid());
                        }
                } //Cierro switch command 1

            } else if (command_counter == 2) { // 2 COMANDOS
                int pid;
                int status;
                int fd[2];  //variable para la tuberia
                pipe(fd);   //tuberia
                pid = fork(); //PRIMER FORK

                switch (pid) { // switch caso hijo 1
                    case -1: //Error
                        perror("Error el el fork");
                        exit(-1);
                    case 0: //HIJO1
                        close(STDOUT_FILENO); //cerramos la salida
                        dup(fd[1]); //fd1 => salida estandar
                        close(fd[0]);
                        close(fd[1]);

                        if(strcmp(filev[0], "0") != 0){ // redirección de entrada
                            printf("filev[0] = %s \n", filev[0]);
                            fich = open(filev[0], O_RDONLY);
                            if(fich<0){
                                perror("Error al abrir el fichero especificado");
                                exit(-1);
                            }
                            dup2(fich, STDIN_FILENO);
                            close(fich);
                        }

                        else if(strcmp(filev[1], "0") != 0){ // redirección de salida - escribimos la salida de la minishell (STDOUT_FILENO) en el fichero especificado
                            fich = open(filev[1],O_CREAT|O_WRONLY, 0666);
                            if(fich<0){
                                perror("Error al abrir el fichero especificado");
                                exit(-1);
                            }
                            dup2(fich, STDOUT_FILENO);
                            close(fich);
                        }

                        else if(strcmp(filev[2], "0") != 0){ // redirección de salida error
                            fich = open(filev[2], O_WRONLY);
                            if(fich<0){
                                perror("Error al abrir el fichero especificado");
                                exit(-1);
                            }
                            dup2(fich, STDERR_FILENO);
                            close(fich);
                        }
                        execvp(argvv[0][0], argvv[0]);

                    default: //PADRE (MINISHELL)
                        pid = fork(); //SEGUNDO FORK
                        switch (pid) {  //switch caso hijo 2
                            case -1:   //Error en el fork del mandato 2
                                perror("Error el el fork");
                                exit(-1);
                            case 0:   //HIJO 2
                                close(STDIN_FILENO);//cerramos la entrada
                                dup(fd[0]);//duplicamos
                                close(fd[0]);//cerramos
                                close(fd[1]);//cerramos

                                /****** REDIRECCIONES *****/
                                if(strcmp(filev[0], "0") != 0){ // redirección de entrada
                                    printf("filev[0] = %s \n", filev[0]);
                                    fich = open(filev[0], O_RDONLY);
                                    if(fich<0){
                                        perror("Error al abrir el fichero especificado");
                                        exit(-1);
                                    }
                                    dup2(fich, STDIN_FILENO);
                                    close(fich);
                                }

                                else if(strcmp(filev[1], "0") != 0){ // redirección de salida - escribimos la salida de la minishell (STDOUT_FILENO) en el fichero especificado
                                    fich = open(filev[1],O_CREAT|O_WRONLY, 0666);
                                    if(fich<0){
                                        perror("Error al abrir el fichero especificado");
                                        exit(-1);
                                    }
                                    dup2(fich, STDOUT_FILENO);
                                    close(fich);
                                }

                                else if(strcmp(filev[2], "0") != 0){ // redirección de salida error
                                    fich = open(filev[2], O_WRONLY);
                                    if(fich<0){
                                        perror("Error al abrir el fichero especificado");
                                        exit(-1);
                                    }
                                    dup2(fich, STDERR_FILENO);
                                    close(fich);
                                }

                                execvp(argvv[1][0], argvv[1]);

                            default: //PADRE (MINISHELL)
                                close(fd[0]);//cerramos
                                close(fd[1]);//cerramos

                                /*************** BACKGROUND ***************/
                                // Sólo hay un &, el del final de la linea: el padre espera por toda la secuencia (al final del ultimo comando)
                                if (in_background == 0) { //proceso sin background
                                    while (wait(&status) != pid);
                                }

                                else { // Proceso en background
                                    printf("[1] %d \n", getpid());
                                }

                        } // Cierro Switch caso hijo 2
                }//Switch caso hijo 1


            } else if (num_commands == 3) { // 3 COMANDOS
                int pid;
                int status;
                int fd[2]; // tuberia 1
                int fd2[2]; // tuberia 2
                pipe(fd);   //TUBERIA 1
                pid = fork(); //PRIMER FORK
                switch (pid) {
                    case -1:   //Error en el fork del mandato 1
                        perror("Error el el fork");
                        exit(-1);
                    case 0:   //Hijo 1
                        close(STDOUT_FILENO);//cerramos la salida estandar
                        dup(fd[0]);//duplicamos
                        close(fd[0]);//cerramos
                        close(fd[1]);//cerramos

                        /****** REDIRECCIONES *****/
                        if(strcmp(filev[0], "0") != 0){ // redirección de entrada
                            printf("filev[0] = %s \n", filev[0]);
                            fich = open(filev[0], O_RDONLY);
                            if(fich<0){
                                perror("Error al abrir el fichero especificado");
                                exit(-1);
                            }
                            dup2(fich, STDIN_FILENO);
                            close(fich);
                        }

                        else if(strcmp(filev[1], "0") != 0){ // redirección de salida - escribimos la salida de la minishell (STDOUT_FILENO) en el fichero especificado
                            fich = open(filev[1],O_CREAT|O_WRONLY, 0666);
                            if(fich<0){
                                perror("Error al abrir el fichero especificado");
                                exit(-1);
                            }
                            dup2(fich, STDOUT_FILENO);
                            close(fich);
                        }

                        else if(strcmp(filev[2], "0") != 0){ // redirección de salida error
                            fich = open(filev[2], O_WRONLY);
                            if(fich<0){
                                perror("Error al abrir el fichero especificado");
                                exit(-1);
                            }
                            dup2(fich, STDERR_FILENO);
                            close(fich);
                        }

                        execvp(argvv[0][0], argvv[0]);
                        perror("Error en el execvp del primer mandato\n");
                        return -1;
                    default:
                        pipe(fd2); //TUBERIA 2
                        pid = fork(); //SEGUNDO FORK
                        switch (pid) {
                            case -1:   //Error en el fork del segundo mandato
                                perror("Error el el fork");
                                exit(-1);
                            case 0:   //HIJO 1
                                close(STDIN_FILENO);//cerramos la entrada estandar
                                dup(fd[0]);//duplicamos
                                close(STDOUT_FILENO);//cerramos la salida estandar
                                dup(fd2[1]);//duplicamos
                                close(fd[0]);
                                close(fd[1]);
                                close(fd2[0]);
                                close(fd2[1]);

                                /****** REDIRECCIONES *****/
                                if(strcmp(filev[0], "0") != 0){ // redirección de entrada
                                    printf("filev[0] = %s \n", filev[0]);
                                    fich = open(filev[0], O_RDONLY);
                                    if(fich<0){
                                        perror("Error al abrir el fichero especificado");
                                        exit(-1);
                                    }
                                    dup2(fich, STDIN_FILENO);
                                    close(fich);
                                }

                                else if(strcmp(filev[1], "0") != 0){ // redirección de salida - escribimos la salida de la minishell (STDOUT_FILENO) en el fichero especificado
                                    fich = open(filev[1],O_CREAT|O_WRONLY, 0666);
                                    if(fich<0){
                                        perror("Error al abrir el fichero especificado");
                                        exit(-1);
                                    }
                                    dup2(fich, STDOUT_FILENO);
                                    close(fich);
                                }

                                else if(strcmp(filev[2], "0") != 0){ // redirección de salida error
                                    fich = open(filev[2], O_WRONLY);
                                    if(fich<0){
                                        perror("Error al abrir el fichero especificado");
                                        exit(-1);
                                    }
                                    dup2(fich, STDERR_FILENO);
                                    close(fich);
                                }

                                execvp(argvv[1][0], argvv[1]);
                                perror("Error en el execvp del segundo mandato\n");
                                return -1;
                            default: //PADRE 1 (MINISHELL)
                                close(fd[0]);
                                close(fd[1]);
                                pid = fork(); // FORK 3
                                switch (pid) {
                                    case -1:  //Error en el fork del tercer mandato
                                        perror("Error el el fork");
                                        exit(-1);
                                    case 0:
                                        close(STDIN_FILENO);//cerramos la entrada estandar
                                        dup(fd2[0]);//duplicamos
                                        close(fd2[0]);
                                        close(fd2[1]);

                                        /****** REDIRECCIONES *****/
                                        if(strcmp(filev[0], "0") != 0){ // redirección de entrada
                                            printf("filev[0] = %s \n", filev[0]);
                                            fich = open(filev[0], O_RDONLY);
                                            if(fich<0){
                                                perror("Error al abrir el fichero especificado");
                                                exit(-1);
                                            }
                                            dup2(fich, STDIN_FILENO);
                                            close(fich);
                                        }

                                        else if(strcmp(filev[1], "0") != 0){ // redirección de salida - escribimos la salida de la minishell (STDOUT_FILENO) en el fichero especificado
                                            fich = open(filev[1],O_CREAT|O_WRONLY, 0666);
                                            if(fich<0){
                                                perror("Error al abrir el fichero especificado");
                                                exit(-1);
                                            }
                                            dup2(fich, STDOUT_FILENO);
                                            close(fich);
                                        }

                                        else if(strcmp(filev[2], "0") != 0){ // redirección de salida error
                                            fich = open(filev[2], O_WRONLY);
                                            if(fich<0){
                                                perror("Error al abrir el fichero especificado");
                                                exit(-1);
                                            }
                                            dup2(fich, STDERR_FILENO);
                                            close(fich);
                                        }

                                        execvp(argvv[2][0], argvv[2]);
                                        perror("Error en el execvp del tercer mandato\n");
                                    default: // PADRE 2
                                        close(fd2[0]);
                                        close(fd2[1]);

                                        /*************** BACKGROUND ***************/
                                        if (in_background == 0) { //proceso sin background
                                            while (wait(&status) != pid);
                                        }

                                        else { // Proceso en background
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

