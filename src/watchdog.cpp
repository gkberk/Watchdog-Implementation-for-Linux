/** \mainpage CMPE322 project1
 *
 * \author Sabri Gökberk Yılmaz - 2017400144
 * \section sec1 main idea:
 * This project requires to write a watchdog.cpp which forks itself as much as given in arguments by number N
 * and then loads processes that's implemented in process.cpp , watchdog runs all the time in the background,
 * it detects when a child dies via wait function. When a child dies watchdog is required to restart the process
 * If the dead process is with id 1, wathcdog is supposed to kill all its children and restart because it is assumed
 * that process with id 1 is a vital process and the other processes need to restart when it dies
 *  It writes all created processes with their id and pid to the named pipe to let executor know.
 *  Named pipes are used in this project in order to facilitate the communication between executor process and the watchdog,
 *  executor process reads the instructions, watchdog creates and sometimes kills processes to maintain processes' run
 *  all the time as asked for in the description. The specific directory is used for this pipe, whenever a new process
 *  is created, watchdog writes it to the pipe with its id and pid.
 *
 *
 * \section sec2 bonus case:
 * The bonus case is based around the idea that in all situations, the program should avoid
 * orphan children and provide graceful termination. It does so by signal handler, if a SIGTERM or SIGINT signal is
 * given, it sends kill signals to all pids on pidlist and waits for the children's termination. Then, it exits.
 *
 * \section sec3 conclusion:
 * It was an educational project, I learned the inside mechanisms of what I've been learning
 * inc cmpe322 for months. There were some points that I couldn't understand why something doesn't work as
 * it's supposed to work. By trying alternative approaches I overcame the issues I faced. It required a calm attitude
 * and an error-detecting approach since a problem may be caused due to various reasons. It made me use my search engine a lot.
 *
 * At the end I provided what's asked for, my outputs are correct and I handled the bonus case too in a way that
 * one may think of it  as rather classic. However, I find this graceful termination essential, and providing it should be
 * the action to be taken in a program like this.
*/

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include <sys/wait.h>
using namespace std;

/**
 * named pipe's name
 */
int namedPipe;
/**
 * path of watchdog's output file
 */
string watchdogfile;
/**
 * data structure for pids, index reflects id of the process.
 */
vector<pid_t> pidlist;
/**
 * signal handler function, provides a graceful termination.
 * For the bonus case, when watchdog receives SIGTERM or SIGINT signal it sends SIGTERM signal to all children
 * and waits for their exit status so that no children is left orphan
 * @param signo no of signal
 */
void sigtermHandler(int signo){//signal handler for a graceful termination.
    // for the bonus case, when watchdog receives SIGTERM or SIGINT signal it sends SIGTERM signal to its children
    //waits for their exit status so that no children is left orphan.
    /**
     * variable f: fstream object for the output
     */
    fstream f;
    f.open(watchdogfile, ios_base::app | ios_base::in);
    close(namedPipe);//closing the pipe

    for(int s=1; s<pidlist.size(); s++){//kills all children
        kill(pidlist[s], SIGTERM);
        usleep(500);
    }
    while(1){
        /**
         * variable status: status of the dead children
         */
        int status=wait(NULL);
        if(status==-1)
            break;
    }
    f<<"Watchdog is terminating gracefully"<<endl;
    exit(0);
}


/**
 * for modularity purposes, there is a function for fork then exec.
 * @param id id of the process to be executed
 * @param processpath path of the process' output file
 * @return child's pid
 */
pid_t forkfunc(int id, string &processpath){
    pid_t cpid=fork();
    if(cpid==0){    //child
        /**
         * variable s: string version of id
         */
        string s=to_string(id);
        /**
         * variable arg: char array version of id
         */
        char arg[s.length()+1];
        strcpy(arg, s.c_str());
        /**
         * variable procpath: char array version of process path
         */
        char procpath[processpath.length()];
        strcpy(procpath, processpath.c_str());
        execl("./process", arg, procpath, NULL);
    }
       //only the parent returns here
    return cpid;
}

/**
 * for modularity, there exists a function to start a process. It calls forkfunc function to fork and exec,
 * it gets child's pid from forkfunc, refreshes pidlist and writes the id and pid of the new process to the named pipe
 * @param namedPipe pipe's name
 * @param id id of process that is to be restarted
 * @param pidlist list of pids
 * @param processpath path of process' output
 * @param file watchdog output file, passed by reference
 */
void processRunner(int &namedPipe, int id, vector<pid_t> &pidlist, string &processpath, fstream &file){
    /**
     * variable cpid: child's pid that's returned by forkfunc
     */
    pid_t cpid=forkfunc(id, processpath);
    if(cpid>0) {
        string temp2s;
        temp2s.push_back('P');
        string s = to_string(id); //id to char
        char c[s.length()];
        strcpy(c, s.c_str());
        for (int k = 0; k < s.length(); k++) { //append id to temp2s
            temp2s.push_back(c[k]);
        }
        temp2s.push_back(' ');


        s = to_string(cpid); //cpid to char
        char c2[s.length()];
        strcpy(c2, s.c_str());
        for (int l = 0; l < s.length(); l++) { //append cpid to temp2s
            temp2s.push_back(c2[l]);
        }

        pidlist[id] = cpid;

        /**
         * variable message: char array that is written to the pipe
         */
        char message[30]; //write to pipe
        strcpy(message, temp2s.c_str());
        write(namedPipe, message, 30);

        file << "P" << id << " is started and it has a pid of " << pidlist[id] << endl;
    }
}

/**
 * the main function that creates new processes by fork and loads the process by exec function.
 * it calls processRunner whenever a new process is needed to start.
 * It detects when a child is dead, if id 1 is dead, it kills all processes then restarts all, if anyone except for id 1
 * is dead, it just restarts the named process by calling processRunner function
 * @param argc number of arguments given
 * @param argv array of given arguments
 * @return 0 by default
 */
int main(int argc, char* argv[]){
    signal(SIGTERM, sigtermHandler);
    signal(SIGINT, sigtermHandler);
    /**
     * variable N: number of processes to be created
     */
    int N = stoi(argv[1]);
    /**
     * variable processpath: path of process' output
     */
    string processpath = argv[2];
    /**
     * variable  watchdogpath: path of watchdog's output
     */
    string watchdogpath = argv[3];
    watchdogfile=watchdogpath;

    for(int p=0; p<=N; p++){
        pidlist.push_back(0);
    }

    /**
     * variable file: fstream object to write to file
     */
    fstream file; //file
    file.open(watchdogpath, ios_base::app | ios_base::in);

    /**
     * variable myfifo: pointer to the string of directory of pipe which is a fifo file
     */
    char *myfifo = (char*) "/tmp/myfifo";  //open pipe
    mkfifo(myfifo, 0666);
    /**
     * variable temp: char array to be written to the pipe
     */
    char temp[30];
    namedPipe=open(myfifo, O_WRONLY);

    temp[0]='P';
    temp[1]='0';
    temp[2]=' ';
    /**
     * variable ind: index of char array
     */
    int ind=2;
    /**
     * variable pid: pid of wathcdog
     */
    pid_t pid=getpid();
    /**
     * variable pidstr: string version of wathcdog's pid
     */
    string pidstr=to_string(pid);
    for(int u=0; u<pidstr.length(); u++){
        ind++;
        temp[ind]=pidstr.at(u);
    }


   // vector<char> holder;
    write(namedPipe, temp, 30); //writing itself
    pidlist[0]=pid;

    for(int id=1; id<=N; id++){
        processRunner(namedPipe, id, pidlist, processpath, file);
        usleep(500);
    }
    while(1) { //endless loop for wathcdog to run in background, it sleeps until a child dies.
        // it doesn't leave the loop until it receives SIGTERM or SIGINT
        /**
         * variable deadpid: when a child dies watchdog detects it here, it gets the pid of dead child
         */
        pid_t deadpid = wait(NULL); //makes wathcdog sleep until a child dies
        /**
         * variable deadid: id of the dead child
         */
        int deadid=-1;
        for (int i = 1; i <= N; i++) {
            if (pidlist[i] == deadpid)
                deadid = i;
        }
        if (deadid == 1) {
            file<<"P1 is killed, all processes must be killed"<<endl;
            for (int i = 2; i <= N; i++) {
                kill(pidlist[i], SIGTERM);
                wait(NULL);
            }
            file << "Restarting all processes" << endl;
            for (int i = 1; i <= N; i++) {
                processRunner(namedPipe, i, pidlist, processpath, file);
                usleep(500);
            }
        }
        else if(deadid<1 || deadid>15);
        else {
            file<<"P"<<deadid << " is killed"<<endl;
            file << "Restarting" << " P" << deadid << endl;
            processRunner(namedPipe, deadid, pidlist, processpath, file);
        }
    }

}