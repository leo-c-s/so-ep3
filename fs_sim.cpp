#include "fs_sim.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <ctime>
#include <vector>
#include <string.h>

int read_int(std::fstream& file) {
    char c[4];
    unsigned int n = 0;

    file.read(c, 4);

    for (int i = 0; i < 4; i++) {
        n = n << 8;
        n |= (unsigned char) c[i];
    }

    return n;
}

void write_int(std::fstream& file, int n) {
    unsigned char c;
    unsigned int m = n;

    for (int i = 0; i < 4; i++) {
        c = (m & 0xff000000) >> 24;
        file << c;
        m = m << 8;
    }
}

FileMeta::FileMeta(std::string *n,
        struct tm *c,
        struct tm *m,
        struct tm *a,
        int f,
        int s) :
    name(n),
    created(c),
    modified(m),
    accessed(a),
    first_block_address(f),
    size(s)
{}

FileMeta* FileMeta::read_meta(std::fstream& infile, int *address) {
    int file_address = read_int(infile);
    time_t created = read_int(infile);
    time_t modified = read_int(infile);
    time_t accessed = read_int(infile);
    int first_block = read_int(infile);
    int size = read_int(infile);
    int type = read_int(infile);

    if (address != nullptr) {
        *address = file_address;
    }

    if (type == 0) {
        return new File(nullptr,
                localtime(&created),
                localtime(&modified),
                localtime(&accessed),
                first_block,
                size);
    } else {
        return new Directory(nullptr,
                localtime(&created),
                localtime(&modified),
                localtime(&accessed),
                first_block,
                size);
    }
}

void FileMeta::write_meta(std::fstream& file, int name_address) {
    write_int(file, name_address);
    write_int(file, mktime(this->created));
    write_int(file, mktime(this->modified));
    write_int(file, mktime(this->accessed));
    write_int(file, this->first_block_address);
    write_int(file, this->size);
}

void FileMeta::set_name(std::string new_name) {
    if (new_name.size() < FileMeta::max_name_len) {
        this->name = new std::string(new_name);
    } else {
        throw "Failed to rename: new name too large";
    }
}

std::string* FileMeta::get_name() {
    return this->name;
}

int FileMeta::get_address() {
    return this->first_block_address;
}

struct tm* FileMeta::get_last_modified()
{
    return this->modified;
}

File::File(std::string *n,
        struct tm *c,
        struct tm *m,
        struct tm *a,
        int f,
        int s) : FileMeta(n, c, m, a, f, s)
{
    this->type_ = FileType::regular;
}

std::string File::get_content() {
    return this->content;
}

void File::set_content(std::string new_content) {
    this->content = new_content;
}

int File::get_size()
{
    return this->size;
}

Directory::Directory(std::string *n,
        struct tm *c,
        struct tm *m,
        struct tm *a,
        int f,
        int s) : FileMeta(n, c, m, a, f, s)
{
    this->type_ = FileType::directory;
}

void Directory::set_file_name(int index, std::string new_name) {
    this->files[index]->set_name(new_name);
}

void Directory::add_file(FileMeta *file) {
    this->files.push_back(file);
}

int Directory::get_file_count() {
    return (int) this->files.size();
}

FileMeta* Directory::get_file(std::string name) {
    FileMeta *file = nullptr;

    for (int i = 0; i < this->files.size() && file == nullptr; i++) {
        if (this->files[i]->get_name()->compare(name) == 0) {
            file = this->files[i];
        }
    }

    return file;
}

FileMeta* Directory::get_file(int index) {
    FileMeta *file = nullptr;

    if(index < this->get_file_count())
    {
        file = this->files[index];
    }

    return file;
}

void Filesystem::load_directory(Directory *dir) {
    std::vector<int> file_name_address;
    std::vector<std::string> file_name;
    int address;

    this->filesystem_file.seekg(dir->get_address() * this->block_size,
            this->filesystem_file.beg);

    for (int i = 0; i < Directory::max_files; i++) {
        address = read_int(this->filesystem_file);

        if (address == 0) {
            break;
        }

        file_name_address.push_back(address);

        FileMeta *file = FileMeta::read_meta(this->filesystem_file, &address);

        dir->add_file(file);
    }

    int cur_block = (int) this->filesystem_file.tellg() / this->block_size;
    int offset = this->allocation_table[cur_block] * this->block_size;
    this->filesystem_file.seekg(offset, this->filesystem_file.beg);

    std::string s;
    for (int i = 0; i < (int) file_name_address.size(); i++) {
        this->filesystem_file >> s;

        if (s.empty() ||
                this->filesystem_file.tellg() % this->block_size == 0) {
            if (this->allocation_table[cur_block] != -1) {
                offset = this->allocation_table[cur_block] * this->block_size;
                this->filesystem_file.seekg(offset, this->filesystem_file.beg);
                cur_block = this->allocation_table[cur_block];
            } else {
                throw
                    "Failed to load directory: missing next block";
            }
        } else {
            file_name.push_back(s);
        }
    }

    for (int i = 0; i < (int) file_name_address.size(); i++) {
        dir->set_file_name(i, file_name[file_name_address[i]]);
    }
}

Filesystem::Filesystem(std::string filesystem_path) {
    int bitmap_char_count = this->block_count / 8;
    long cur_pos, offset;

    this->bitmap = std::vector<bool>(this->block_count, true);

    this->filesystem_file.open(filesystem_path,
            std::fstream::in | std::fstream::binary);
    if (this->filesystem_file) {
        std::cout << "Reading filesystem file" << std::endl;

        std::cout << "Reading bitmap..." << std::endl;
        // read bitmap
        char c;
        for (int i = 0; i < bitmap_char_count; i++) {
            this->filesystem_file >> c;

            for (int j = 0; j < 8 && 8 * i + j < this->block_count; j++) {
                this->bitmap[8 * i + j] = (c << j) & 0x80;
            }
        }

        std::cout << "bitmap[0..6]: ";
        for (int i = 0; i < 7; i++) {
            std::cout << this->bitmap[i] << " ";
        }
        std::cout << std::endl;
        std::cout << "FAT starts at " << (int) this->filesystem_file.tellg()
            << std::endl;

        // read FAT
        std::cout << "Reading FAT..." << std::endl;
        for (int i = 0; i < this->block_count; i++) {
            this->allocation_table[i] = read_int(this->filesystem_file);
        }

        std::cout << "FAT[0..6]: ";
        for (int i = 0; i < 7; i++) {
            std::cout << this->allocation_table[i] << " ";
        }
        std::cout << std::endl;

        this->root = (Directory *) FileMeta::read_meta(this->filesystem_file,
                nullptr);

        this->load_directory(this->root);
    } else {
        std::cout << "Creating filesystem file" << std::endl;

        // create new filesystem file
        this->filesystem_file.open(filesystem_path,
                std::fstream::out | std::fstream::binary);

        // create bitmap
        this->filesystem_file << (char) 0b00000011; // reserve root directory
        for (int i = 0; i < bitmap_char_count - 1; i++) {
            this->filesystem_file << (char) 0xff;
        }

        // mark root blocks as used
        for (int i = 0; i < 6; i++) {
            this->bitmap[i] = false;
        }

        // create FAT
        for (int i = 0; i < 5; i++) { // make linked list for root directory
            write_int(this->filesystem_file, i + 1);
        }
        write_int(this->filesystem_file, 0xffffffff); // 0xffffffff == -1
        for (unsigned int i = 0; i < this->block_count - 6; i++) {
            write_int(this->filesystem_file, 0);
        }

        // fill rest of block with zeroes
        cur_pos = this->filesystem_file.tellp();
        offset = this->block_size - cur_pos % this->block_size;
        for (int i = 0; i < offset; i++) {
            this->filesystem_file << '\0';
        }

        // create root directory
        time_t cur_time = time(nullptr);
        this->root = new Directory(nullptr,
                localtime(&cur_time),
                localtime(&cur_time),
                localtime(&cur_time),
                0,
                0);
        this->root->write_meta(this->filesystem_file, 0);

        // fill rest of block with zeroes
        cur_pos = this->filesystem_file.tellp();
        offset = this->block_size - cur_pos % this->block_size;
        for (int i = 0; i < offset; i++) {
            this->filesystem_file << '\0';
        }

        // create remaining blocks for root directory and add them to FAT
        int cur_block = cur_pos / 4000;
        for (int i = 0; i < 5; i++) {
            this->allocation_table[i] = i + 1; // mark next block in FAT

            for (int i = 0; i < this->block_size; i++) { // fill block with 0
                this->filesystem_file << '\0';
            }
            cur_block ++;
        }
        this->allocation_table[5] = -1; // final block of root directory
    }
}

Filesystem::~Filesystem() {
    if (this->filesystem_file.is_open()) {
        this->filesystem_file.close();
    }
}

void Filesystem::copy (std::string source_path, std::string dest_path) {
}

void Filesystem::mkdir (std::string dir_name) {
}

void Filesystem::rmdir (std::string dir_name) {
}

void Filesystem::cat (std::string file_path) {
}

void Filesystem::touch (std::string file_path) {
    std::vector<std::string> path_names;
    path_names = split_path(file_path);
    int i = 0;
    Directory* cur = this->root;
    FileMeta* temp;
    while(i < path_names.size()-1)
    {
        temp = cur->get_file(path_names[i]);
        if(cur == nullptr || cur->type() != FileType::directory)
        {
            printf("Path inválido!\n");
            return;
        }
        else
        {
            i++;
            cur = (Directory*) temp;
        }
    }
    //cur aponta para diretorio onde estará o arquivo
    temp = cur->get_file(path_names[i]);
    if(temp == nullptr)
    {
        //TODO: cria arquivo novo
    }
    else
    {
        //atualiza tempo do arquivo
    }
    
}

void Filesystem::rm (std::string file_path) {
}

void Filesystem::ls (std::string dir_name) {
    std::vector<std::string> path_names;
    path_names = split_path(dir_name);
    int i = 0;
    Directory* cur = this->root;
    FileMeta* temp;
    while(i < path_names.size())
    {
        temp = cur->get_file(path_names[i]);
        if(cur == nullptr || cur->type() != FileType::directory)
        {
            printf("Path inválido!\n");
            return;
        }
        else
        {
            i++;
            cur = (Directory*) temp;
        }
    }
    //cur points to the right directory
    for(i=0;i<cur->get_file_count();i++)
    {
        temp = cur->get_file(i);
        if(temp->type() == FileType::directory)
        {
            //printa info diretorio
            std::cout << temp->get_name() << "   Diretorio\nLast modified: " << asctime(temp->get_last_modified()) << std::endl;
        }
        else
        {
            //printa info arquivo
            File* f = (File*) temp;
            std::cout << temp->get_name() << "\nLast modified: " << asctime(temp->get_last_modified()) << std::endl;
            std::cout << "Size: " << f->get_size() << std::endl;
        }
        printf("\n");
    }
}

std::string Filesystem::find (std::string dir_name, std::string file_path) {
    return "";
}

int Filesystem::df() {
    return 0;
}

std::vector<std::string> split_path(std::string file_path) {
    std::vector<std::string> words;
    std::string straux;
    char* aux;
    char* pathchar;
    pathchar = (char *) file_path.c_str();

    if(pathchar[0] != '/')
    {
        printf("Path inválido!\n");
        return words;
    }

    aux = strtok(&pathchar[1], "/");
    while(aux != NULL)
    {
        straux.assign(aux);
        words.push_back(straux);
        aux = strtok(NULL, " ");
    }

    return words;
}