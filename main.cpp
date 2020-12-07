#include "fs_sim.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <chrono> 

int main(int argc, char *argv[]) {
    Filesystem *f = nullptr;

    std::vector<std::string> commands;
    std::string temp, straux;
    char *aux;

    commands.push_back("");
    while (commands[0].compare("sai") != 0) {
        if (argc == 1) {
            commands.clear();  //limpa vetor
            printf("[ep3]: "); //printa promp

            std::getline(std::cin, temp);            //coloca linha completa no temp
            aux = strtok((char *)temp.c_str(), " "); //separa linha em tokens
            while (aux != NULL)                      //coloca tokens no vetor commands
            {
                straux.assign(aux);         //converte token para string para guardar no vetor
                commands.push_back(straux); //guarda no vetor
                aux = strtok(NULL, " ");    //pega proximo
            }
        } else {
            commands.clear();
            commands.push_back(argv[1]);
        }

        //Checa qual o comando e o executa
        if (commands[0].compare("mount") == 0) {
            f = new Filesystem(commands[1]);
        } else if (commands[0].compare("umount") == 0) {
            if (f != nullptr) {
                delete f;
                f = nullptr;
            }
        } else if (commands[0].compare("cp") == 0) {
            if (f != nullptr) {
                f->copy(commands[1], commands[2]);
            }
        } else if (commands[0].compare("mkdir") == 0) {
            if (f != nullptr) {
                f->mkdir(commands[1]);
            }
        } else if (commands[0].compare("rmdir") == 0) {
            if (f != nullptr) {
                f->rmdir(commands[1]);
            }
        } else if (commands[0].compare("cat") == 0) {
            if (f != nullptr) {
                f->cat(commands[1]);
            }
        } else if (commands[0].compare("touch") == 0) {
            if (f != nullptr) {
                f->touch(commands[1]);
            }
        } else if (commands[0].compare("rm") == 0) {
            if (f != nullptr) {
                f->rm(commands[1]);
            }
        } else if (commands[0].compare("ls") == 0) {
            if (f != nullptr) {
                f->ls(commands[1]);
            }
        } else if (commands[0].compare("find") == 0) {
            if (f != nullptr) {
                f->find(commands[1], commands[2]);
            }
        } else if (commands[0].compare("df") == 0) {
            if (f != nullptr) {
                f->df();
            }
        } 
    }

    if (f != nullptr) {
        delete f;
    }

    return 0;
}
