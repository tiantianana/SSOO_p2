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
# define BUFFER_SIZE 1024

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

//******************* MANDATOS INTERNOS (AÚN NO COMPILA ESTE BLOQUE )************************

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
        fprintf(stdout, "[OK] %d + %d = %d; Acc %d \n", num1, num2, Acc, Acc);
        //perror(resultado);
        // SI NO FUNCIONA: Escribir el resultado en la salida estandar el mensaje: [ERROR] -> AÑADIR ESTA OPCION EN LAS COMPROBACIONES
    }
    else if(strcmp(operador, "mod") == 0){ //CASO MOD
        int cociente = num1/num2;
        int resto = num1%num2;
        fprintf(stdout, "[OK] %d %% %d = %d * %d + %d \n", num1, num2, num2, resto, cociente);
    }
    else{ // OPERADOR ERRÓNEO
        fprintf(stdout, "%s", "ERROR: no se encuentra la operación especificada \n");
        exit(-1);
    }
}

int my_cp(char *ficheroOrigen, char *ficheroDestino) {

    //Comprobamos parametros
    if (ficheroDestino == NULL && ficheroOrigen == NULL || ficheroDestino == NULL ||
        ficheroOrigen == NULL) { //Comprobamos que tenga la sintaxis que requiere el mandato
        write(STDOUT_FILENO, "[ERROR] La estructura del comando es mycp <fichero origen> <fichero destino> \n", strlen("[ERROR] La estructura del comando es mycp <fichero origen> <fichero destino> \n"));
        exit(-1);
    } else if (open(ficheroOrigen, O_RDONLY) < 0) { //Comprobamos que el fichero origen exista
        fprintf(stdout, "%s", "[ERROR] Error al abrir el fichero origen \n");
        exit(-1);
    }   else{
        int copia;
        char buf[BUFFER_SIZE];
        //Abrimos el fich1 en modo solo lectura.
        int fich1 = open(ficheroOrigen, O_RDONLY);
        if(fich1 < 0) {
            fprintf(stdout, "%s", "[ERROR] Error al abrir el archivo origen");
        }
        //Abrimos el fich2, si no existe lo crea, en modo lectura y escritura.
        int fich2 = open(ficheroDestino, O_CREAT|O_WRONLY, 0666);
        if(fich2 < 0) {
            fprintf(stdout, "%s", "[ERROR] Error al abrir el archivo destino");
        }

        copia = read(fich1,buf,BUFFER_SIZE);
        while(copia>0){
            if(write(fich2,buf,copia) == -1){
                fprintf(stdout, "%s", "[ERROR] Error al escribir\n");
                return -1;
            }
            if(read(fich1,buf,BUFFER_SIZE) == -1){
                fprintf(stdout, "%s", "[Error] al leer\n");
                return -1;
            }
        }
        close(fich1);   //cerramos el archivo origen
        close(fich2);   //cerramos el archivo destino
        fprintf(stdout, "[OK] Copiado con exito el fichero %s a %s\n", ficheroOrigen, ficheroDestino);


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
                fprintf(stdout, "%s", "Error: Numero máximo de comandos es 8");
                exit(-1);
            }
            print_command(argvv, filev, in_background);
            if(strcmp(argvv[0][0], "mycalc") == 0) { // argvv[0][0] es mycalc
                my_calc(argvv[0][1], argvv[0][2], argvv[0][3]);
            }

            else if(strcmp(argvv[0][0], "mycp") == 0) { // argvv[0][0] es mycp
                my_cp(argvv[0][1], argvv[0][2]);
            }

            else {
                int fd[2];
                int pid;
                int savestdin = dup(STDIN_FILENO); // guardo la entrada estandar en el primer fd vacio
                int savestdout = dup(STDOUT_FILENO); // guardo la salida estandar en el primer fd vacio
                for(int i = 0; i < command_counter; i++){
                    pipe(fd);  // creo la tubería (posterior)
                    if(i == command_counter-1){ //ÚLTIMO HIJO - No necesito la última tubería (cierro fd[1], el proceso hijo y el padre cierran fd[0])
                        close(fd[1]);

                        // ***************  REDIRECCIONES (las defino dentro del último hijo) **********
                        int fich;
                        if(strcmp(filev[0], "0") != 0){ // REDIRECCIÓN DE ENTRADA
                            //printf("filev[0] = %s \n", filev[0]); //??
                            close(STDIN_FILENO);
                            fich = open(filev[0], O_RDONLY); // Open utiliza el primer descriptor disponible de la tabla (el que acabamos de cerrar)
                            if(fich<0){
                                fprintf(stdout, "%s", "Error al abrir el fichero especificado");
                                exit(-1);
                            }
                        }
                        else if(strcmp(filev[1], "0") != 0){ // REDIRECCIÓN DE SALIDA - escribimos la salida de la minishell (STDOUT_FILENO) en el fichero especificado
                            close(STDOUT_FILENO);
                            fich = open(filev[1],O_CREAT|O_WRONLY, 0666);
                            if(fich<0){
                                fprintf(stdout, "%s", "Error al abrir el fichero especificado");
                                exit(-1);
                            }
                        }
                        else if(strcmp(filev[2], "0") != 0){ // REDIRECCIÓN DE SALIDA ERROR
                            close(STDERR_FILENO);
                            fich = open(filev[2], O_WRONLY, 0666);
                            if(fich<0){
                                fprintf(stdout, "%s", "Error al abrir el fichero especificado");
                                exit(-1);
                            }
                        }
                    }

                    else{ // DEMÁS HIJOS
                        dup2(fd[1], STDOUT_FILENO); //Dejo la escritura de la nueva tuberia en la salida (y la cierro)
                        close(fd[1]);               // No cierro aquí la lectura de la tubería porque la necesita el padre para concatenarla al siguiente hijo
                    }

                    pid = fork();
                    if(pid == 0){
                        close(fd[0]); // La entrada del hijo es la lectura de la tubería anterior (no necesita la de la nueva, la cierra)
                        execvp(argvv[i][0], argvv[i]); // En este punto el hijo lee de la tubería de la iteración anterior y escribe en la actual
                        break;
                    } else if (pid < 0){
                        fprintf(stderr, "%s", "Error el el fork");
                        exit(-1);
                    } else{ //padre
                        if(i == command_counter-1){ // Al crear el último hijo, compruebo si está en bg (el valor de una secuencia es el valor de su último mandato)
                            if (in_background == 1) { //proceso (o secuencia) en background
                                //printf("[1] %d \n", getpid());
                                dup2(savestdout, STDOUT_FILENO); // Dejo la salida estándar en su sitio (1)
                                close(fd[0]);
                                break; // me salgo sin hacer wait
                            }
                        }
                        wait(&status); // espera que el hijo acabe antes de crear el siguiente
                        dup2(fd[0], STDIN_FILENO); // Dejo la entrada/lectura de la tubería actual en la entrada (del siguiente hijo que creemos)
                        dup2(savestdout, STDOUT_FILENO); // Dejo la salida estándar en su sitio (1)
                        close(fd[0]);
                    }

                }// for

                dup2(savestdin, STDIN_FILENO); // Devuelvo la entrada estándar a su sitio (la salida ya lo está)
                close(savestdin);
                close(savestdout);

            }// else
        } // cierro if command > 0
        wait(&status); //recojo a los hijos
    } // cierro while
    return 0;
} //cierro main