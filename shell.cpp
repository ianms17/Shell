#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <time.h>

using namespace std;

/**** HELPER FUNCTIONS *****/

/*
 * Function: split
 * Takes a line of text and seperates it into a vector based on the given delimiter
 * Note that the default delimiter is a single space
 */ 
vector<string> split(string inputline, char delimiter = ' ') {
   
    vector<string> commands;
    int substringLen = 0;
    int lastSplitPos = inputline.find(delimiter);
    int i = lastSplitPos + 1;
    string substring;
    bool splitToEnd = false;

    // logic to get a command that has no delimiters
    if (lastSplitPos < 0) {
        commands.push_back(inputline);
        return commands;
    }

    // get the first command
    commands.push_back(inputline.substr(0, lastSplitPos));

    while (i < inputline.length() - 1) {
        if (inputline.at(i) == delimiter) {
            substring = inputline.substr(lastSplitPos + 1, substringLen);
            commands.push_back(substring);
            lastSplitPos = i;
            substringLen = 0;
            i++;
        } else if (inputline.at(i) == '\'') {
            i++;
            while (inputline.at(i) != '\'') {
                substringLen++;
                if (i < inputline.length() - 1) {
                    i++;
                }
            }

            commands.push_back(inputline.substr(lastSplitPos + 2, substringLen));
            if (i == inputline.length() - 1) {
                splitToEnd = true;
            }
            substringLen = 0;
        } else if (inputline.at(i) == '\"') {
            i++;
            while (inputline.at(i) != '\"') {
                substringLen++;
                if (i < inputline.length() - 1) {
                    i++;
                }
            }

            commands.push_back(inputline.substr(lastSplitPos + 2, substringLen));
            if (i == inputline.length() - 1) {
                splitToEnd = true;
            }
            substringLen = 0;
        } else if (splitToEnd == true) {
            break;
        } else {
            i++;
            substringLen++;
        }
    }
    // logic to get everything from last split to the end
    if (!splitToEnd) {
        commands.push_back(inputline.substr(lastSplitPos + 1, inputline.length() - lastSplitPos));
    }

    return commands;
    
}

/*
 * Function: Trim
 */ 
string trim(string str) {
    if (str.at(0) == ' ') {
        str = str.erase(0, 1);
    }
    if (str.at(str.length() - 1) == ' ') {
        str = str.erase(str.length() - 1, 1);
    }
    return str;
}

/*
 * Function: VectorAsCharArr
 * Converts a vector of strings to an array of cstrings
 * Args in execvp must take an array of cstrings
 */
char** VectorAsCharArr(vector<string> vct) {
    char** results = new char* [vct.size() + 1];
    for (int i = 0; i < vct.size(); i++) {
        results[i] = (char*) vct[i].c_str();
    }
    results[vct.size()] = NULL;
    return results;
} 


/***************************/

int main () {
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    int kb = dup(0);
    //int terminal = dup(1);
    vector<int> pids;

    while(true) {
        time_t currTime = time(NULL);
        string time = ctime(&currTime);
        int pos = 0;
        pos = time.find('\n');
        time.erase(pos, 1);
        cout << pw->pw_name << "--" << time << "$ ";
        dup2(kb, 0);
        // dup2(terminal, 1);
        bool twoFound = false;
        bool background = false;
        int ampersand = 0;
        int clear = -1;

        string inputLine;
        getline (cin, inputLine);

        for (int i = 0; i < pids.size(); i++) {
            waitpid(pids[i], 0, WNOHANG);
        }

        // exit command reached, close terminal
        if (inputLine == string("exit")) {
            cout << "End of Shell" << endl;
            break;
        }

        vector<string> pipeCmd = split(inputLine, '|');
        for (int i = 0; i < pipeCmd.size(); i++) {
            pipeCmd[i] = trim(pipeCmd[i]);
        }
        

        for (int i = 0; i < pipeCmd.size(); i++) {
            int fds[2];
            pipe(fds);
            
            int pid = fork();
            
            // pid == 0 denotes that this is the child process
            if (pid == 0) {
                if ((ampersand = pipeCmd[i].find('&')) > 0) {
                    background = true;
                    pipeCmd[i] = pipeCmd[i].erase(ampersand - 1, 2);
                    trim(pipeCmd[i]);
                    pids.push_back(getpid());
                }

                if (inputLine.find("pwd") == 0) {
                    char path [1024];
                    getcwd(path, sizeof(path));
                    printf("%s\n", path);
                    continue;
                }

                if (inputLine.find("cd") == 0) {
                    string directory = split(pipeCmd[i])[1];
                    chdir(directory.c_str());
                    continue;
                }
                
                int found = 0;
                int found2 = 0;

                inputLine = pipeCmd[i];

                // there is a > in the the inputLine
                if ((found = inputLine.find('>')) >= 0) {
                    if ((found2 = inputLine.find('<')) >= 0) {
                        twoFound = true;
                        if (found < twoFound) {
                            vector<string> vct = split(inputLine, '>');
                            vector<string> vct_x = split(vct[1], '<');
                            vct[1] = vct_x[0];
                            vct.push_back(vct_x[1]);
                        }
                    } else {
                        vector<string> vct = split(inputLine, '>');
                        for (int j = 0; j < vct.size(); j++) {
                            vct[j] = trim(vct[j]);
                        }

                        inputLine = vct[0];
                        string filename = vct[1];

                        int fd = open(filename.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                        dup2(fd, 1);
                        close(fd);
                    }
                }

                if ((found = inputLine.find('<')) >= 0) {
                    vector<string> vct = split(inputLine, '<');
                    for (int j = 0; j < vct.size(); j++) {
                        vct[j] = trim(vct[j]);
                    }

                    inputLine = vct[0];
                    string filename = vct[1];

                    int fd = open(filename.c_str(), O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);
                    dup2(fd, 0);
                    close(fd);
                }
                
                if (i < pipeCmd.size() - 1) {
                    dup2(fds[1], 1);
                }
                


                vector<string> commands = split(inputLine);
                char** args = VectorAsCharArr(commands);
                execvp(args[0], args);
            } // end child conditional

            else {
                // parent function will wait for child to finish executing
                if (!background) {
                    if (i == pipeCmd.size() - 1) {
                        waitpid(pid, 0, 0);
                    }
                }
                dup2(fds[0], 0);
                close(fds[1]);

            } // end parent conditional
        } // end for loop
    } // end while loop
    return 0;
}