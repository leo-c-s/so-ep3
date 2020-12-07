#include "fs_sim.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <chrono> 

int main(int argc, char *argv[])
{
    Filesystem *f;

    std::vector<std::string> commands;
    std::string temp, straux;
    char *aux;

    commands.push_back("");
    while (commands[0].compare("sai") != 0)
    {
        if (argc == 1)
        {
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
        }
        else
        {
            commands.clear();
            commands.push_back(argv[1]);
        }

        //Checa qual o comando e o executa
        if (commands[0].compare("mount") == 0)
        {
            f = new Filesystem(commands[1]);
        }
        else if (commands[0].compare("umount") == 0)
        {
            delete f;
            f = nullptr;
        }
        else if (commands[0].compare("cp") == 0)
        {
            f->copy(commands[1], commands[2]);
        }
        else if (commands[0].compare("mkdir") == 0)
        {
            f->mkdir(commands[1]);
        }
        else if (commands[0].compare("rmdir") == 0)
        {
            f->rmdir(commands[1]);
        }
        else if (commands[0].compare("cat") == 0)
        {
            f->cat(commands[1]);
        }
        else if (commands[0].compare("touch") == 0)
        {
            f->touch(commands[1]);
        }
        else if (commands[0].compare("rm") == 0)
        {
            f->rm(commands[1]);
        }
        else if (commands[0].compare("ls") == 0)
        {
            f->ls(commands[1]);
        }
        else if (commands[0].compare("find") == 0)
        {
            f->find(commands[1], commands[2]);
        }
        else if (commands[0].compare("df") == 0)
        {
            f->df();
        }
        else if (commands[0].compare("teste1-4") == 0)
        {
            std::string fs("fs");
            f = new Filesystem(fs);
            std::string ori("1mb.txt");
            std::string dest("/1mb.txt");
            

            auto start = std::chrono::high_resolution_clock::now();

            // Call the function, here sort()
            f->copy(ori, dest);

            // Get ending timepoint
            auto stop = std::chrono::high_resolution_clock::now();

            // Get duration. Substart timepoints to
            // get durarion. To cast it to proper unit
            // use duration cast method
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "Tempo tomado pela função copy: "
            << duration.count() << " microsegundos" << std::endl;

            start = std::chrono::high_resolution_clock::now();

            // Call the function, here sort()
            f->rm(dest);

            // Get ending timepoint
            stop = std::chrono::high_resolution_clock::now();

            // Get duration. Substart timepoints to
            // get durarion. To cast it to proper unit
            // use duration cast method
            duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "Tempo tomado pela função rm: "
            << duration.count() << " microsegundos" << std::endl;
            commands[0].assign("sai");
        }
        else if (commands[0].compare("teste2-5") == 0)
        {
            std::string fs("fs");
            f = new Filesystem(fs);
            std::string ori("10mb.txt");
            std::string dest("/1mb.txt");
            

            auto start = std::chrono::high_resolution_clock::now();

            // Call the function, here sort()
            f->copy(ori, dest);

            // Get ending timepoint
            auto stop = std::chrono::high_resolution_clock::now();

            // Get duration. Substart timepoints to
            // get durarion. To cast it to proper unit
            // use duration cast method
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "Tempo tomado pela função copy: "
            << duration.count() << " microsegundos" << std::endl;

            start = std::chrono::high_resolution_clock::now();

            // Call the function, here sort()
            f->rm(dest);

            // Get ending timepoint
            stop = std::chrono::high_resolution_clock::now();

            // Get duration. Substart timepoints to
            // get durarion. To cast it to proper unit
            // use duration cast method
            duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "Tempo tomado pela função rm: "
            << duration.count() << " microsegundos" << std::endl;
            commands[0].assign("sai");
        }
        else if (commands[0].compare("teste3-6") == 0)
        {
            std::string fs("fs");
            f = new Filesystem(fs);
            std::string ori("30mb.txt");
            std::string dest("/1mb.txt");
            

            auto start = std::chrono::high_resolution_clock::now();

            // Call the function, here sort()
            f->copy(ori, dest);

            // Get ending timepoint
            auto stop = std::chrono::high_resolution_clock::now();

            // Get duration. Substart timepoints to
            // get durarion. To cast it to proper unit
            // use duration cast method
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "Tempo tomado pela função copy: "
            << duration.count() << " microsegundos" << std::endl;

            start = std::chrono::high_resolution_clock::now();

            // Call the function, here sort()
            f->rm(dest);

            // Get ending timepoint
            stop = std::chrono::high_resolution_clock::now();

            // Get duration. Substart timepoints to
            // get durarion. To cast it to proper unit
            // use duration cast method
            duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "Tempo tomado pela função rm: "
            << duration.count() << " microsegundos" << std::endl;
            commands[0].assign("sai");
        }
        else if (commands[0].compare("teste78") == 0)
        {
            std::string fs("fs");
            f = new Filesystem(fs);
            std::string dir("/pasta1");
            

            auto start = std::chrono::high_resolution_clock::now();

            // Call the function, here sort()
            f->rmdir(dir);

            // Get ending timepoint
            auto stop = std::chrono::high_resolution_clock::now();

            // Get duration. Substart timepoints to
            // get durarion. To cast it to proper unit
            // use duration cast method
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "Tempo tomado pela função rmdir: "
            << duration.count() << " microsegundos" << std::endl;

            commands[0].assign("sai");
        }
        else if (commands[0].compare("make8") == 0)
        {
            std::string fullpath("");
            std::string semipath("/");
            for(int i = 1; i < 32; i++)
            {
                semipath.append("pasta" + std::to_string(i) + "/");
                for(int j = 0; j<10; j++)
                {
                    fullpath.assign(semipath + std::to_string(j) + ".txt");
                    f->touch(fullpath);
                }
            }

        }
        else if (commands[0].compare("make7") == 0)
        {
            std::string fullpath("");
            std::string semipath("");
            for(int i = 1; i < 32; i++)
            {
                semipath.append("/pasta" + std::to_string(i));
                f->mkdir(semipath);
            }

        }
        
    }
    if (f != nullptr)
    {
        delete f;
    }

    return 0;
}