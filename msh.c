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
# define BUFFER_SIZE 102

// ficheros por si hay redirección
char filev[3][64];
char *argv_execvp[8];

//Contador y acumualdor.
char *Acc;
int contador;

//to store the execvp second parameter


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
    if(op1 == NULL || op2 == NULL || operador == NULL){
        fprintf(stdout, "%s", "[ERROR] La estructura del comando es <operando 1> <add/mod> <operando 2>\n");
        return -1;
    }
    // COMPROBAR que los argumentos no son null/ vacíos ***************
    int resultado; // variable de retorno
    int num1 = atoi(op1); // convertimos op1 y op2 a int
    int num2 = atoi(op2);

    if( num1==0 && op1[0]!=0|| num2==0 && op2[0]!=0){
        fprintf(stdout, "%s", "[ERROR] La estructura del comando es <operando 1> <add/mod> <operando 2>\n");
        return -1;
    }

    if(strcmp(operador,"add") == 0){ // CASO ADD
        resultado = num1 + num2;
        contador += resultado;
        //El contador es un int y necesitamos convertirlo en string
        asprintf(&Acc,"%d",contador);
        //Establecemos en nuevo valor del acumulador
        setenv("Acc", Acc , 1);
        fprintf(stderr, "[OK] %d + %d = %d; Acc %s \n", num1, num2, resultado, Acc);
        return 1;
    }
    else if(strcmp(operador, "mod") == 0){ //CASO MOD
        int cociente = num1/num2;
        int resto = num1%num2;
        fprintf(stderr, "[OK] %d %% %d = %d * %d + %d \n", num1, num2, num2, cociente, resto);
        return 1;
    }

    else{ // OPERADOR ERRÓNEO
        fprintf(stdout, "%s", "[ERROR] La estructura del comando es <operando 1> <add/mod> <operando 2>\n");
        return -1;
    }


}

int my_cp(char *ficheroOrigen, char *ficheroDestino) {

    //Comprobamos parametros
    if (ficheroDestino == NULL || ficheroOrigen == NULL) { //Comprobamos que tenga la sintaxis que requiere el mandato
        write(STDOUT_FILENO, "[ERROR] La estructura del comando es mycp <fichero origen> <fichero destino>\n",
              strlen("[ERROR] La estructura del comando es mycp <fichero origen> <fichero destino>\n"));
        return -1;
    } else {
        int copia;
        char buf[1024];
        //Abrimos el fich1 en modo solo lectura.
        int fich1 = open(ficheroOrigen, O_RDONLY);
        if (fich1 < 0) {
            fprintf(stdout, "%s", "[ERROR] Error al abrir el archivo origen");
            return -1;
        }
        //Abrimos el fich2, si no existe lo crea, en modo lectura y escritura.
        int fich2 = open(ficheroDestino, O_CREAT | O_WRONLY, 0666);
        if (fich2 < 0) {
            fprintf(stdout, "%s", "[ERROR] Error al abrir el archivo destino");
            return -1;
        }
        //Creamos una copia que se va actalizando a lo largo del while
        copia = read(fich1, &buf, sizeof(buf));
        while (copia) {
            if (write(fich2, &buf, copia) == -1) {
                fprintf(stdout, "%s", "[ERROR] Error al escribir\n");
                return -1;
            }

            copia = read(fich1, &buf, sizeof(buf));
            if (copia == -1) {
                fprintf(stdout, "%s", "[Error] al leer\n");
                return -1;
            }
        }
        close(fich1);   //cerramos el archivo origen
        close(fich2);   //cerramos el archivo destino
        fprintf(stdout, "[OK] Copiado con exito el fichero %s a %s\n", ficheroOrigen, ficheroDestino);
        return 1;

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
    setenv("Acc", "0", 1);

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
                return -1;
            }
            //print_command(argvv, filev, in_background);
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
                int savestderr = dup(STDERR_FILENO); // guardo la salida de error estandar en el primer fd vacio
                int fich2 = 0; //redirección salida error
                for(int i = 0; i < command_counter; i++){
                    pipe(fd);  // creo la tubería (posterior)

                    if(i == 0){// PRIMER HIJO - solo compruebo si hay redirecciones
                        // (cierra fd[0] al ejecutarse tras el fork) no comprobamos si es el único pq igualmente el ultimo hijo cierra fd[1]

                        // REDIRECCIÓN DE ENTRADA - TIENE QUE SER LA ENTRADA DE LA TUBERÍA!! (PRIMER HIJO)
                        if(strcmp(filev[0], "0") != 0){
                            int fich0;
                            close(STDIN_FILENO);
                            fich0 = open(filev[0], O_RDONLY); // Open utiliza el primer descriptor disponible de la tabla (el que acabamos de cerrar)
                            if(fich0<0){ // si hay un error en la redirección me salgo del bucle sin crear ningún hijo (no hace falta el exit(-1))
                                fprintf(stdout, "%s", "Error al abrir el fichero especificado");
                                break; //restauro stdin fuera del bucle
                            }
                        }

                        // REDIRECCIÓN DE SALIDA DE ERROR - se propaga en todos los hijos (no toco stderr en el fork)
                        if(strcmp(filev[2], "0") != 0){
                            close(STDERR_FILENO);
                            fich2 = open(filev[2], O_CREAT|O_WRONLY, 0666);
                            if(fich2<0){ // si hay un error en la redirección me salgo del bucle sin crear ningún hijo (y restauro stderr)
                                fprintf(stdout, "%s", "Error al abrir el fichero especificado");
                                dup2(savestderr, fich2); // dejo la salida de error estandar en su sitio
                                close(savestderr);
                                break;
                            }
                        }
                    }

                    if(i == command_counter-1){ //ÚLTIMO HIJO - No necesito la última tubería (cierro fd[1], el proceso hijo y el padre cierran fd[0])
                        close(fd[1]);  // Si solo hay un hijo, se cierra la salida de la tubería aqui por lo que mantiene la estandar

                        // REDIRECCIÓN DE SALIDA - escribimos la salida de la minishell (STDOUT_FILENO) en el fichero especificado
                        if(strcmp(filev[1], "0") != 0){
                            int fich1;
                            close(STDOUT_FILENO);
                            fich1 = open(filev[1],O_CREAT|O_WRONLY, 0666);
                            if(fich1<0){
                                fprintf(stdout, "%s", "Error al abrir el fichero especificado");
                                dup2(savestdout, fich1); // restauro la salida estandar y me salgo antes de crear el último hijo
                                close(savestdout);
                                break;
                            }
                        }

                    } else{ // DEMÁS HIJOS
                        dup2(fd[1], STDOUT_FILENO); //Dejo la escritura de la nueva tuberia en la salida (y la cierro)
                        close(fd[1]);               // No cierro aquí la lectura de la tubería porque la necesita el padre para concatenarla al siguiente hijo
                    }

                    pid = fork();
                    if(pid == 0){
                        close(fd[0]); // La entrada del hijo es la lectura de la tubería anterior (o, en caso del primer hijo, la entrada estandar o un fichero de redirección de entrada)
                        execvp(argvv[i][0], argvv[i]); // En este punto el hijo lee de la tubería de la iteración anterior y escribe en la actual
                        break;
                    } else if (pid < 0){
                        fprintf(stdout, "%s", "Error el el fork");
                        if(strcmp(filev[2], "0") != 0){
                            dup2(savestderr, fich2); // dejo la salida de error estandar en su sitio
                            close(savestderr);
                        }
                        break;
                    } else{ //padre
                        if(i == command_counter-1){ // Al crear el último hijo, compruebo si está en bg (el valor de una secuencia es el valor de su último mandato)
                            if (in_background == 1) { //proceso (o secuencia) en background
                                printf("[1] %d \n", getpid());
                                dup2(savestdout, STDOUT_FILENO); // Dejo la salida estándar en su sitio (1)
                                close(fd[0]);
                                break; // me salgo sin hacer wait
                            }
                        }
                        wait(&status); // espera que el hijo acabe antes de crear el siguiente
                        dup2(fd[0], STDIN_FILENO); // Dejo la entrada/lectura de la tubería actual en la entrada (del siguiente hijo que creemos)
                        close(fd[0]);
                        dup2(savestdout, STDOUT_FILENO); // Dejo la salida estándar en su sitio (1)

                        if(status != 0){ // si ha habido un fallo en la ejecución del hijo, me salgo del bucle (no creo más hijos)
                            if(strcmp(filev[2], "0") != 0){ // Si había redireccion de la salida de error, la restauro
                                dup2(savestderr, fich2);
                                close(savestderr);
                            }
                            break;
                        }
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