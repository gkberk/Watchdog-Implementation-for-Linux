/**
 * \author Sabri Gökberk Yılmaz - 2017400144
 * \section id1 explanation:
 * this cpp file is only meant to be compiled, then be loaded from watchdog's execl call. It takes its own id
 * and process output directory as arguments. It is expected to handle signals by printing the received signals's no
 * and it should exit if it receives SIGTERM.
 *
 */
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <fstream>

using namespace std;

/**
 * id of the process
 */
string id;
/**
 * process' output path
 */
string processpath;

/**
 * Signal handler
 * @param signo no of signal
 */
void signalHandler(int signo){
    /**
     * variable file: fstream object for output
     */
    fstream file;
    file.open(processpath, ios_base::app | ios_base::in);
    if(signo==15){
        if(file.is_open()) {
            file<<"P"<<id<<" received signal 15, terminating gracefully"<<endl;
        }
        exit(0);
    }
    else{
        if(file.is_open()) {
            file<<"P"<<id<<" received signal "<<signo<<endl;
        }
    }
}

/**
 * main of process, execl function of watchdog loads starting from here
 * @param argc number of arguments
 * @param argv argument array
 * @return 0
 */
int main(int argc, char *argv[]){
    /**
     * variable id: id of process
     */
    id=argv[0];
    /**
     * variable processpath: path of process output
     */
    processpath=argv[1];
    /**
     * variable file: fstream object for output
     */
    fstream file;
    file.open(processpath, ios_base::app | ios_base::in);
    if(file.is_open())
        file<<"P"<<id<<" is waiting for a signal"<<endl;
    signal(SIGHUP, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGTRAP, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGXCPU, signalHandler);

    while(1){
        sleep(1);
    }
}
