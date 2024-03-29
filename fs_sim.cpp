#include "fs_sim.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>
#include <queue>
#include <string.h>

int read_int(std::fstream &file) {
    char c[4];
    unsigned int n = 0;

    file.read(c, 4);

    for (int i = 0; i < 4; i++) {
        n = n << 8;
        n |= (unsigned char)c[i];
    }

    return n;
}

void write_int(std::fstream &file, int n) {
    unsigned char c;
    unsigned int m = n;

    for (int i = 0; i < 4; i++) {
        c = (m & 0xff000000) >> 24;
        file << c;
        m = m << 8;
    }
}

std::string read_string(std::fstream &file) {
    char c;
    std::string s;

    file >> c;
    while (c != '\0') {
        s.push_back(c);
        file >> c;
    }

    return s;
}

FileMeta::FileMeta(std::string n,
        time_t c,
        time_t m,
        time_t a,
        int f,
        int s) : name(n),
    created(c),
    modified(m),
    accessed(a),
    first_block_address(f),
    size(s)
{
    this->is_loaded = false;
}

FileMeta::~FileMeta() {
}

FileMeta *FileMeta::read_meta(std::fstream &infile, int *address) {
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

    if (type == 1) {
        return new File("",
                created,
                modified,
                accessed,
                first_block,
                size);
    } else if (type == 2) {
        return new Directory("",
                created,
                modified,
                accessed,
                first_block,
                size);
    } else if (type == 3) {
        FileMeta *f = new FileMeta("",
                created,
                modified,
                accessed,
                first_block,
                size);
        f->remove();
        return f;
    } else { // if type is invalid or zero, the file does not exist
        return nullptr;
    }
}

void FileMeta::write_meta(std::fstream &file, int name_address) {
    write_int(file, name_address);
    write_int(file, this->created);
    write_int(file, this->modified);
    write_int(file, this->accessed);
    write_int(file, this->first_block_address);
    write_int(file, this->size);

    if (this->type_ == FileType::regular) {
        write_int(file, 1);
    } else if (this->type_ == FileType::directory) {
        write_int(file, 2);
    } else if (this->type_ == FileType::deleted) {
        write_int(file, 3);
    } else {
        write_int(file, 0);
    }
}

void FileMeta::set_name(std::string new_name) {
    if (new_name.size() < FileMeta::max_name_len) {
        this->name = new_name;
    } else {
        std::cout << "set_name:";
        std::cout << "Failed to rename: new name too large" << std::endl;
        std::cout << "name: " << new_name << std::endl;
    }
}

std::string FileMeta::get_name() {
    return this->name;
}

int FileMeta::get_address() {
    return this->first_block_address;
}

time_t FileMeta::get_last_modified() {
    return this->modified;
}

void FileMeta::set_last_modified(time_t moment) {
    this->modified = moment;
}

void FileMeta::set_last_accessed(time_t moment) {
    this->accessed = moment;
}

void FileMeta::remove() {
    this->type_ = FileType::deleted;
}

File::File(std::string n,
        time_t c,
        time_t m,
        time_t a,
        int f,
        int s) : FileMeta(n, c, m, a, f, s)
{
    this->type_ = FileType::regular;
    this->content = std::string("");
}

std::string File::get_content() {
    return this->content;
}

void File::set_content(std::string new_content) {
    this->content = new_content;
}

int File::get_size() {
    return this->size;
}

Directory::Directory(std::string n,
        time_t c,
        time_t m,
        time_t a,
        int f,
        int s) : FileMeta(n, c, m, a, f, s)
{
    this->type_ = FileType::directory;
    this->files = std::vector<FileMeta *>();
}

Directory::~Directory() {
    FileMeta *file;
    for (unsigned int i = 0; i < this->files.size(); i++) {
        file = this->files[i];

        if (file != nullptr) {
            delete file;
        }
    }
}

void Directory::set_file_name(int index, std::string new_name) {
    this->files[index]->set_name(new_name);
}

void Directory::add_file(FileMeta *file) {
    for (int i = 0; i < this->files.size(); i++) {
        if (this->files[i]->type() == FileType::deleted) {
            delete this->files[i];
            this->files[i] = file;
            return;
        }
    }

    this->files.push_back(file);
}

void Directory::push_file(FileMeta *file) {
    this->files.push_back(file);
}

int Directory::get_file_count() {
    int n = this->files.size(), count = 0;

    for (int i = 0; i < n; i++) {
        if (this->files[i]->type() != FileType::deleted) {
            count ++;
        }
    }

    return count;
}

int Directory::get_size() {
    return this->files.size();
}

int Directory::find_inside(std::string curpath, std::string file) {
    int output = 0;

    for (unsigned int i = 0; i < this->files.size(); i++) {
        if (this->files[i]->type() == FileType::directory) {
            Directory *inside = (Directory *)this->files[i];
            std::string path = curpath;
            path.append(inside->get_name());
            output += inside->find_inside(path, file);
        } else {
            if (this->files[i]->get_name().compare(file) == 0) {
                std::cout
                    << curpath
                    << "/"
                    << this->files[i]->get_name()
                    << std::endl;
                output += 1;
            }
        }
    }

    return output;
}

FileMeta *Directory::get_file(std::string name) {
    FileMeta *file = nullptr, *temp;

    for (unsigned int i = 0; i < this->files.size() && file == nullptr; i++) {
        temp = this->files[i];
        if (temp->get_name() == name && temp->type() != FileType::deleted) {
            file = temp;
        }
    }

    return file;
}

FileMeta *Directory::get_file(int index) {
    FileMeta *file = nullptr;
    int j = 0;

    for (int i = 0; i < this->files.size() && file == nullptr; i++) {
        if (this->files[i]->type() != FileType::deleted) {
            if (j == index) {
                file = this->files[i];
            } else {
                j ++;
            }
        }
    }

    return file;
}

FileMeta* Directory::at(int index) {
    if (index < this->files.size()) {
        return this->files[index];
    }

    return nullptr;
}

void Directory::del_file(std::string file_name) {
    int file_count = this->files.size();
    for (int i = 0; i < file_count; i++) {
        if (this->files[i]->get_name() == file_name) {
            this->files[i]->remove();
            break;
        }
    }
}

Filesystem::Filesystem(std::string filesystem_path) {
    int bitmap_char_count = 3125;

    this->bitmap = std::vector<bool>(25000, true);

    this->filesystem_file.open(filesystem_path,
            std::fstream::in | std::fstream::binary);
    if (this->filesystem_file) {
        // read bitmap
        char c;
        for (int i = 0; i < bitmap_char_count; i++) {
            this->filesystem_file >> c;

            for (int j = 0; j < 8 && 8 * i + j < this->block_count; j++) {
                this->bitmap[8 * i + j] = (c << j) & 0x80;
            }
        }

        // read FAT
        for (int i = 0; i < this->block_count; i++) {
            this->allocation_table[i] = read_int(this->filesystem_file);
        }

        this->root = (Directory *)FileMeta::read_meta(this->filesystem_file,
                nullptr);
        this->root->set_name(std::string(""));
        this->load_directory(this->root);
        time_t mod_time = this->root->get_last_modified();
    } else {
        // create new filesystem file
        this->filesystem_file.open(filesystem_path,
                std::fstream::out | std::fstream::binary);

        // create bitmap
        this->filesystem_file << (char)0b00000011; // reserve root directory
        for (int i = 0; i < bitmap_char_count - 1; i++) {
            this->filesystem_file << (char)0xff;
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

        // create root directory
        time_t cur_time = time(nullptr);
        this->root = new Directory("",
                cur_time,
                cur_time,
                cur_time,
                0,
                0);
        this->root->write_meta(this->filesystem_file, 0);
        this->root->is_loaded = true;

        // fill rest of block with zeroes
        long cur_pos = this->get_write_position();
        long offset = this->block_size - cur_pos % this->block_size;
        for (int i = 0; i < offset; i++) {
            this->filesystem_file << '\0';
        }

        // create remaining blocks for root directory and add them to FAT
        int cur_block = cur_pos / this->block_size;
        for (int i = 0; i < 5; i++) {
            this->allocation_table[i] = i + 1; // mark next block in FAT

            for (int i = 0; i < this->block_size; i++) { // fill block with 0
                this->filesystem_file << '\0';
            }
            cur_block++;
        }
        this->allocation_table[5] = -1; // final block of root directory
    }

    // reopen file allowing both read and write
    this->filesystem_file.close();
    this->filesystem_file.open(filesystem_path,
            std::fstream::in | std::fstream::out | std::fstream::binary);
}

Filesystem::~Filesystem() {
    if (this->filesystem_file.is_open()) {
        this->filesystem_file.close();
    }
}

void Filesystem::load_file(File *file) {
    std::string content = "";
    int cur_block = file->get_address();

    if (file->is_loaded) {
        return;
    }

    char *buf = new char[this->block_size];
    while (cur_block != -1) {
        this->move_to_block(cur_block);
        this->filesystem_file.read(buf, this->block_size);
        content.append(buf);
        cur_block = this->allocation_table[cur_block];
    }

    delete[] buf;

    // remove extra null bytes
    auto pos = content.find('\0');
    if (pos != std::string::npos) {
        content.erase(content.find('\0'));
    }

    file->set_content(content);
    file->is_loaded = true;
}

void Filesystem::save_file(File *file) {
    char *buf = new char[this->block_size];
    std::stringstream ss(file->get_content());
    int cur_block = file->get_address();

    while (ss && cur_block != -1) {
        for (int i = 0; i < this->block_size; i++) {
            buf[i] = '\0';
        }
        ss.read(buf, this->block_size - 1);

        this->move_to_block(cur_block);
        this->filesystem_file.write(buf, this->block_size);
        cur_block = this->allocation_table[cur_block];
    }

    delete[] buf;
}

void Filesystem::load_directory(Directory *dir) {
    std::vector<int> file_name_address;
    std::vector<std::string> file_name;
    int address;

    if (dir == nullptr || dir->is_loaded) {
        return;
    }

    this->move_to_block(dir->get_address());
    if (dir == this->root) {
        delete FileMeta::read_meta(this->filesystem_file, nullptr);
    }

    // first, read the metadata of files in this directory
    for (int i = 0; i < Directory::max_files; i++) {
        FileMeta *file = FileMeta::read_meta(this->filesystem_file, &address);

        if (file == nullptr) {
            break;
        }

        if (file->type() != FileType::deleted) {
            file_name_address.push_back(address);
        }

        dir->push_file(file);
    }

    // then, move to the next block of the directory
    int cur_block = this->get_read_position() / this->block_size;
    int next_block = this->allocation_table[cur_block];
    this->move_to_block(next_block);

    // and read all file names
    std::string s;
    int cur_offset;
    for (int i = 0; i < (int)file_name_address.size(); i++) {
        s = read_string(this->filesystem_file);

        file_name.push_back(s);

        /*
         * - if cur_offset is 0 after reading a string, then we're at the
         * beginning of the adjecent block and should move to the correct one
         *
         * - if next character is '\0', then the next name was too large and
         * was put in the next block, and so we move to it
         */
        cur_offset = this->get_read_position() % this->block_size;
        if (cur_offset == 0 || this->filesystem_file.peek() == '\0') {
            next_block = this->allocation_table[cur_block];

            if (this->allocation_table[cur_block] != -1) {
                this->move_to_block(next_block);

                cur_block = next_block;
            } else {
                std::cout << "load_directory: "
                    << "Failed to load directory: missing next block"
                    << std::endl;
                return;
            }
        }
    }

    // finally, set the names of the files in the directory
    FileMeta *f;
    for (int i = 0; i < (int) file_name_address.size(); i++) {
        f = dir->get_file(i);
        f->set_name(file_name[file_name_address[i]]);
    }

    dir->is_loaded = true;
}

void Filesystem::save_directory(Directory *dir) {
    int cur_block = dir->get_address(), offset;
    int dir_size = dir->get_size();
    FileMeta *file;
    std::string name, next_name = "";

    this->move_to_block(cur_block);

    // write metadata for root directory
    if (cur_block == 0) {
        this->root->write_meta(this->filesystem_file, 0);
    }

    int address = 0;
    for (int i = 0; i < dir_size; i++) {
        file = dir->at(i);

        file->write_meta(this->filesystem_file, address);

        if (file->type() != FileType::deleted) {
            address ++;
        }
    }

    cur_block = this->allocation_table[cur_block];
    this->move_to_block(cur_block);

    int file_count = dir->get_file_count();
    for (int i = 0; i < file_count; i++) {
        name = dir->get_file(i)->get_name();
        this->filesystem_file << name << '\0';

        offset = this->get_write_position() % this->block_size;
        if (i < file_count - 1) {
            next_name = dir->get_file(i + 1)->get_name();

            if (next_name.length() >= this->block_size - offset) {
                for (; offset > 0; offset--) {
                    this->filesystem_file << '\0';
                }
            }
        }

        if (offset == 0) {
            cur_block = this->allocation_table[cur_block];
            this->move_to_block(cur_block);
        }
    }

    while (cur_block != -1) {
        offset = this->get_write_position() % this->block_size;
        for (; offset < this->block_size; offset++) {
            this->filesystem_file << '\0';
        }

        cur_block = this->allocation_table[cur_block];
        this->move_to_block(cur_block);
    }
}

void Filesystem::move_to_block(int block) {
    int address = this->first_block_offset + this->block_size * block;

    this->filesystem_file.seekg(address, this->filesystem_file.beg);
    this->filesystem_file.seekp(address, this->filesystem_file.beg);
}

int Filesystem::get_read_position() {
    int pos = this->filesystem_file.tellg();
    return pos - this->first_block_offset;
}

int Filesystem::get_write_position() {
    int pos = this->filesystem_file.tellp();
    return pos - this->first_block_offset;
}

void Filesystem::set_bit(int index, bool value) {
    int i = index / 8;
    char prev, new_bit = 0b10000000 >> (index % 8);

    this->bitmap[index] = value;
    this->filesystem_file.seekg(i, this->filesystem_file.beg);
    this->filesystem_file >> prev;
    this->filesystem_file.seekp(i, this->filesystem_file.beg);

    if (value) {
        this->filesystem_file << (char)(prev | new_bit);
    } else {
        this->filesystem_file << (char)(prev & ~new_bit);
    }
}

void Filesystem::set_FAT(int index, int value) {
    int pos = this->FAT_offset + index * 4;
    this->allocation_table[index] = value;
    this->filesystem_file.seekp(pos, this->filesystem_file.beg);
    write_int(this->filesystem_file, value);
}

void Filesystem::copy(std::string source_path, std::string dest_path) {
    std::vector<std::string> path_names = split_path(dest_path);
    int i = 0;
    Directory *cur = this->root;
    FileMeta *temp;
    int last = path_names.size() - 1;
    while (i < last) {
        temp = cur->get_file(path_names[i]);

        if (temp == nullptr || temp->type() != FileType::directory) {
            throw "Path inválido!";
        } else {
            i++;
            cur = (Directory *)temp;
            this->load_directory(cur);
        }
    }

    //cur tem diretorio onde arquivo sera colocado
    temp = cur->get_file(path_names[i]);
    if (temp != nullptr) {
        std::cout << "Arquivo já existe!" << std::endl;
    } else {
        // read input file
        std::fstream source;
        source.open(source_path, std::fstream::in | std::fstream::ate);
        auto file_size = source.tellg();
        int space_found = 0;

        std::vector<int> blocks;
        int j;
        for (j = 0; space_found < file_size && j < this->block_count; j++) {
            if (this->bitmap[j]) {
                blocks.push_back(j);
                space_found += this->block_size;
            }
        }

        if (space_found < file_size) {
            std::cout
                << "copy: not enough blocks for new file"
                << std::endl;
            source.close();
            return;
        }

        char *buf = new char[file_size];
        source.seekg(0, source.beg);
        source.read(buf, file_size);
        source.close();

        std::string content(buf);
        delete[] buf;

        for (j = 0; j < blocks.size() - 1; j++) {
            this->set_bit(blocks[j], false);
            this->set_FAT(blocks[j], blocks[j + 1]);
        }
        this->set_bit(blocks[j], false);
        this->set_FAT(blocks[j], -1);

        time_t t = time(nullptr);

        std::string file_name_pointer = path_names[i];
        File *new_file = new File(file_name_pointer,
                t,
                t,
                t,
                blocks[0],
                (int)file_size);

        new_file->set_content(content);
        cur->add_file(new_file);
        this->save_directory(cur);
        this->save_file(new_file);
    }
}

void Filesystem::mkdir(std::string dir_name) {
    std::vector<std::string> path_names;
    path_names = split_path(dir_name);
    int i = 0;
    Directory *cur = this->root, *new_dir;
    FileMeta *temp;
    int last = path_names.size() - 1;
    while (i < last) {
        temp = cur->get_file(path_names[i]);

        if (temp == nullptr || temp->type() != FileType::directory) {
            throw "Path inválido!";
        } else {
            i++;
            cur = (Directory *)temp;
            this->load_directory(cur);
        }
    }

    //cur tem diretorio onde novo diretorio tem que estar
    temp = cur->get_file(path_names[i]);
    if (temp != nullptr) {
        std::cout << "Diretório já existe!" << std::endl;
    } else {
        std::vector<int> blocks;
        int j;
        for (j = 0; blocks.size() < 6 && j < this->block_count; j++) {
            if (this->bitmap[j]) {
                blocks.push_back(j);
            }
        }

        if (blocks.size() != 6) {
            std::cout
                << "mkdir: not enough blocks for new directory"
                << std::endl;
            return;
        }

        for (j = 0; j < 5; j++) {
            this->set_bit(blocks[j], false);
            this->set_FAT(blocks[j], blocks[j + 1]);
        }
        this->set_bit(blocks[j], false);
        this->set_FAT(blocks[j], -1);

        time_t t = time(nullptr);

        std::string dirnamepointer = path_names[i];
        new_dir = new Directory(dirnamepointer,
                t,
                t,
                t,
                blocks[0],
                0);
        cur->add_file(new_dir);
        this->save_directory(cur);
        this->save_directory(new_dir);
    }
}

void Filesystem::rmdir(std::string dir_name) {
    std::vector<std::string> path_names;
    path_names = split_path(dir_name);
    int i = 0;
    Directory *cur = this->root;
    FileMeta *temp;
    int last = path_names.size();
    while (i < last) {
        temp = cur->get_file(path_names[i]);

        if (temp == nullptr || temp->type() != FileType::directory) {
            std::cout << "rmdir: ";
            std::cout << "Path inválido!" << std::endl;
            return;
        } else {
            i++;
            cur = (Directory *)temp;
            this->load_directory(cur);
        }
    }

    while (cur->get_file_count() > 0) {
        temp = cur->get_file(cur->get_file_count() - 1);
        std::string newpath = "/";
        for(i = 0; i<last;i++)
        {
            newpath.append(path_names[i]);
            newpath.append("/");
        }
        newpath.append(temp->get_name());
        if (temp->type() == FileType::directory) {
            Directory* dirpointer = (Directory*) temp;
            if(dirpointer->get_file_count() > 0)
                this->rmdir(newpath);
            else
            {
                this->rm(newpath);
            }
            
        } else {
            this->rm(newpath);
        }
    }
    this->rm(dir_name);
}

void Filesystem::cat(std::string file_path) {
    std::vector<std::string> path_names;
    path_names = split_path(file_path);
    int i = 0;
    Directory *cur = this->root;
    File *file;
    FileMeta *temp;
    int last = path_names.size() - 1;
    while (i < last) {
        temp = cur->get_file(path_names[i]);

        if (temp == nullptr || temp->type() != FileType::directory) {
            throw "Path inválido!";
        } else {
            i++;
            cur = (Directory *)temp;
            this->load_directory(cur);
        }
    }

    //cur tem diretorio onde novo diretorio tem que estar
    temp = cur->get_file(path_names[i]);
    if (temp == nullptr) {
        std::cout << "Arquivo não existe" << std::endl;
    } else {
        file = (File *)temp;
        this->load_file(file);
        std::cout << file->get_content() << std::endl;
    }
}

void Filesystem::touch(std::string file_path) {
    std::vector<std::string> path_names;
    path_names = split_path(file_path);
    int i = 0;
    Directory *cur = this->root;
    FileMeta *temp;
    int last = path_names.size() - 1;
    while (i < last) {
        temp = cur->get_file(path_names[i]);

        if (temp == nullptr || temp->type() != FileType::directory) {
            std::cout << "touch: ";
            std::cout << "Path inválido!" << std::endl;
            return;
        } else {
            i++;
            cur = (Directory *)temp;
            this->load_directory(cur);
        }
    }

    //cur aponta para diretorio onde estará o arquivo
    temp = cur->get_file(path_names[i]);
    if (temp == nullptr) {
        int j = 0;
        while (!this->bitmap[j] && j < this->block_count) {
            j++;
        }

        if (j != this->block_count) {
            time_t t = time(nullptr);

            std::string file_name = path_names[i];

            temp = new File(file_name, t, t, t, j, 0);
            cur->add_file(temp);

            this->set_bit(j, false);
            this->set_FAT(j, -1);

            this->save_directory(cur);
        } else {
            std::cout << "Sistema de arquivos cheio!" << std::endl;
        }
    } else {
        time_t t = time(nullptr);

        temp->set_last_accessed(t);
        temp->set_last_modified(t);
        this->save_directory(cur);
    }
}

void Filesystem::rm(std::string file_path) {
    std::vector<std::string> path_names;
    path_names = split_path(file_path);
    int i = 0;
    Directory *cur = this->root;
    FileMeta *temp;

    int last = path_names.size() - 1;
    while (i < last) {
        temp = cur->get_file(path_names[i]);

        if (temp == nullptr || temp->type() != FileType::directory) {
            std::cout << "ls: ";
            std::cout << "Path inválido!" << std::endl;
            return;
        } else {
            i++;
            cur = (Directory *)temp;
            this->load_directory(cur);
        }
    }

    FileMeta *f = cur->get_file(path_names[i]);
    if (f == nullptr) {
        std::cout << "Arquivo nao encontrado!" << std::endl;
        return;
    }

    int add = f->get_address();
    while (add != -1) {
        int next = this->allocation_table[add];
        this->set_bit(add, true);
        add = next;
    }

    cur->del_file(f->get_name());
    this->save_directory(cur);
}

void Filesystem::ls(std::string dir_name) {
    std::vector<std::string> path_names;
    path_names = split_path(dir_name);
    int i = 0;
    Directory *cur = this->root;
    FileMeta *temp;
    int last = path_names.size();
    while (i < last) {
        temp = cur->get_file(path_names[i]);

        if (temp == nullptr || temp->type() != FileType::directory) {
            std::cout << "ls: ";
            std::cout << "Path inválido!" << std::endl;
            return;
        } else {
            i++;
            cur = (Directory *)temp;
            this->load_directory(cur);
        }
    }

    //cur points to the right directory
    cur->set_last_accessed(time(nullptr));
    this->save_directory(cur);

    for (i = 0; i < cur->get_file_count(); i++) {
        temp = cur->get_file(i);

        if (temp->type() == FileType::directory) {
            //printa info diretorio
            time_t mod_time = temp->get_last_modified();
            std::cout
                << temp->get_name()
                << "   Diretorio\nLast modified: "
                << asctime(localtime(&mod_time))
                << std::endl;
        } else if (temp->type() == FileType::regular) {
            //printa info arquivo
            File *f = (File *)temp;

            time_t mod_time = temp->get_last_modified();
            std::cout
                << temp->get_name()
                << "\nLast modified: "
                << asctime(localtime(&mod_time));

            std::cout << "Size: " << f->get_size() << std::endl;
            std::cout << std::endl;
        } else {
            std::cout << "ls: invalid file type" << std::endl;
        }
    }
}

void Filesystem::find(std::string dir_name, std::string file_name) {
    std::vector<std::string> path_names;
    path_names = split_path(dir_name);
    int i = 0;
    Directory *cur = this->root;
    FileMeta *temp;
    int last = path_names.size();
    while (i < last) {
        temp = cur->get_file(path_names[i]);

        if (temp == nullptr || temp->type() != FileType::directory) {
            std::cout << "find: ";
            std::cout << "Path inválido!" << std::endl;
            return;
        } else {
            i++;
            cur = (Directory *)temp;
            this->load_directory(cur);
        }
    }

    //cur has folder to start looking through
    int found_one;
    found_one = cur->find_inside(dir_name, file_name);
    if (found_one == 0) {
        std::cout << "find: ";
        std::cout << "Nenhum arquivo com este nome encontrado." << std::endl;
    }
    return;
}

void Filesystem::df() {
    Directory *cur, *dir;
    File *file;
    FileMeta *temp;
    std::queue<Directory *> dirs;
    int dir_count = 0, file_count = 0, free_space = 0, wasted_space = 0;
    int n, size;

    dirs.push(this->root);

    while (!dirs.empty()) {
        cur = dirs.front();
        this->load_directory(cur);
        n = cur->get_file_count();

        for (int i = 0; i < n; i++) {
            temp = cur->get_file(i);

            if (temp->type() == FileType::directory) {
                dir = (Directory *) temp;
                dir_count ++;
                dirs.push(dir);
            } else {
                file = (File *) temp;
                file_count ++;
                size = file->get_size();
                wasted_space += this->block_size - (size % this->block_size);
            }
        }
        dirs.pop();
    }

    for (bool b : this->bitmap) {
        if (b) {
            free_space += this->block_size;
        }
    }

    std::cout
        << "Numero de diretórios: " << dir_count            << std::endl
        << "Numero de arquivos:   " << file_count           << std::endl
        << "Espaco livre:         " << free_space   << "KB" << std::endl
        << "Espaco desperdicado:  " << wasted_space << "KB" << std::endl;
}

std::vector<std::string> split_path(std::string file_path) {
    std::vector<std::string> words;
    std::string straux;
    char *aux;
    char *pathchar;
    pathchar = (char *)file_path.c_str();

    if (pathchar[0] != '/') {
        std::cout << "split_path: Path inválido!" << std::endl;
        return words;
    }

    aux = strtok(&pathchar[1], "/");
    while (aux != NULL) {
        straux.assign(aux);
        words.push_back(straux);
        aux = strtok(NULL, " /");
    }

    return words;
}
