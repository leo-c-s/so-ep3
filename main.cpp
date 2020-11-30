#include "fs_sim.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>



int main() {
    Filesystem *f;

    std::vector<std::string> commands;
    std::string temp, straux;
    char* aux;

    commands.push_back("");
    while (commands[0].compare("sai") != 0)
    {
        commands.clear();                           //limpa vetor
        printf("[ep3]: ");                          //printa promp

        std::getline (std::cin,temp);               //coloca linha completa no temp
        aux = strtok((char*) temp.c_str(), " ");    //separa linha em tokens
        while(aux != NULL)                          //coloca tokens no vetor commands
        {
            straux.assign(aux);                     //converte token para string para guardar no vetor
            commands.push_back(straux);             //guarda no vetor
            aux = strtok(NULL, " ");                //pega proximo
        }

        //Checa qual o comando e o executa
        if(commands[0].compare("mount") == 0)
        {
            f = new Filesystem(commands[1]);
        }
        else if(commands[0].compare("umount") == 0)
        {
            delete f;
            f = nullptr;
        }
        else if(commands[0].compare("cp") == 0)
        {
            f->copy(commands[1], commands[2]);
        }
        else if(commands[0].compare("mkdir") == 0)
        {
            f->mkdir(commands[1]);
        }
        else if(commands[0].compare("rmdir") == 0)
        {
            f->rmdir(commands[1]);
        }
        else if(commands[0].compare("cat") == 0)
        {
            f->cat(commands[1]);
        }
        else if(commands[0].compare("touch") == 0)
        {
            f->touch(commands[1]);
        }
        else if(commands[0].compare("rm") == 0)
        {
            f->rm(commands[1]);
        }
        else if(commands[0].compare("ls") == 0)
        {
            f->ls(commands[1]);
        }
        else if(commands[0].compare("find") == 0)
        {
            f->find(commands[1], commands[2]);
        }
        else if(commands[0].compare("df") == 0)
        {
            f->df();
        }
    }
    if(f != nullptr)
    {
        delete f;
    }

    return 0;
}