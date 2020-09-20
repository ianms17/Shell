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
    /*
    vector<string> commands;
    int lastSplitPos = inputline.find(delimiter);
    int count = 0;

    // there was no space found in the command, return whole line to vector
    if (lastSplitPos < 0) {
        commands.push_back(inputline);
        return commands;
    }

    // get first command and add it to the vector
    commands.push_back(inputline.substr(0, lastSplitPos));

    // declare quote counters, if either of them are odd then you are inside a quote
    int singleCount = 0;
    int doubleCount = 0;
    int quotePos = 0;
    int i = lastSplitPos + 1;
    bool splitToEnd = false;

    while (i < inputline.length()) {
    //for (int i = lastSplitPos + 1; i < inputline.length(); i++) {
        if (inputline.at(i) == '\'') {
            singleCount++;
        } else if (inputline.at(i) == '\"') {
            doubleCount++;
        }

        //cerr << singleCount << " " << doubleCount << endl;
        
        // if the quote counters are even then you aren't inside of quotes anymore, proceed as normal
        if (inputline.at(i) != '\'' && inputline.at(i) != '\"') {
            count++;
            if (inputline.at(i) == delimiter) {
                commands.push_back(inputline.substr(lastSplitPos + 1, count - 1));
                count = 0;
                lastSplitPos = i;
            }
            i++;
        } else {
            int quoteLen = 0;
            if (inputline.at(i) == '\'') {
                while (inputline.at(i) != '\'') {
                    quoteLen++;
                    i++;
                    cerr << i << endl;
                }
            } else {
                while (inputline.at(i) != '\"') {
                    quoteLen++;
                    i++;
                    cerr << i << endl;
                }
            }
            commands.push_back(inputline.substr(lastSplitPos, quoteLen));
            if (i == inputline.length()) {
                splitToEnd = true;
            }
        }
    } // end string iteration
    
    if (!splitToEnd) {
        commands.push_back(inputline.substr(lastSplitPos + 1, inputline.length() - lastSplitPos));
    }

    return commands;
    */

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
    /*
    for (int i = 0; i < commands.size(); i++) {
        cerr << commands[i] << endl;
    }
    */
    

    return commands;
}

/*
 * Function: Trim
 */ 
string trim(string str) {
    if (str.at(0) == ' ') {
        str.erase(0, 1);
    }
    if (str.at(str.length() - 1) == ' ') {
        str.erase(str.length() - 1, 1);
    }
    return str;
}

/*
 * Function: VectorAsCharArr
 * Converts a vector of strings to an array of cstrings
 * Args in execvp must take an array of cstrings
 */
char** VectorAsCharArr(vector<string> vct) {
    //vector<char*> vct_tmp(vct.size());
    char** result = new char*[vct.size() + 1];
    for (int i = 0; i < vct.size(); i++) {
        result[i] = new char[sizeof(vct[i]) + 1];
        strncpy(result[i], vct[i].c_str(), vct[i].length());
        // vct_tmp[i] = (char*)vct[i].c_str();
    }
    /*
    // char** result;
    for (int i = 0; i < vct_tmp.size(); i++) {
        result[i] = vct_tmp[i];
    }
    */
    result[vct.size()] = NULL;

    return result;
} 


/***************************/

int main () {
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    int kb = dup(0);
    int terminal = dup(1);

    while(true) {
        time_t currTime = time(NULL);
        string time = ctime(&currTime);
        int pos = 0;
        pos = time.find('\n');
        time.erase(pos, 1);
        cout << pw->pw_name << "--" << time << "$ ";
        dup2(kb, 0);
        dup2(terminal, 1);
        bool twoFound = false;

        string inputLine;
        getline (cin, inputLine);

        // exit command reached, close terminal
        if (inputLine == string("exit")) {
            cout << "End of Shell" << endl;
            break;
        }

        
        vector<string> pipeCmd = split(inputLine, '|');
        for (int i = 0; i < pipeCmd.size(); i++) {
            pipeCmd[i] = trim(pipeCmd[i]);
            cerr << pipeCmd[i] << endl;
        }
        

        for (int i = 0; i < pipeCmd.size(); i++) {
            int fds[2];
            pipe(fds);
            
            int pid = fork();
            
            // pid == 0 denotes that this is the child process
            if (pid == 0) {
                int found = 0;
                int found2 = 0;

                inputLine = pipeCmd[i];

                // there is a > in the the inputLine
                if ((found = inputLine.find('>')) >= 0) {
                    if ((found2 = inputLine.find('<')) >= 0) {
                        cout << "Found Both <>" << endl;
                        continue;
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
                
                /*
                cerr << "Size=" << commands.size() << endl;

                for (int i = 0; i < commands.size(); i++) {
                    cerr << args[i] << endl;
                }
                */
                

                execvp(args[0], args);
            } // end child conditional

            else {
                // parent function will wait for child to finish executing
                waitpid(pid, 0, 0);
                
                dup2(fds[0], 0);
                close(fds[1]);

            } // end parent conditional
        } // end for loop
    } // end while loop
    return 0;
    
   
   /*
   vector<string> test = split("ls -l /sbin");
   char** test2 = VectorAsCharArr(test);
   for (int i = 0; i < test.size(); i++) {
       cout << test2[i] << endl;
   }
   */
   


}