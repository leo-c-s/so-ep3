#include "fs_sim.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <ctime>
#include <vector>

int read_int(std::fstream& file) {
    char c;
    int n = 0;

    for (int i = 0; i < 4; i++) {
        file >> c;
        n = n << 8;
        n |= c;
    }

    return n;
}

void write_int(std::fstream& file, int n) {
    char c;

    for (int i = 0; i < 4; i++) {
        c = (n & 0xff000000) >> 24;
        file << c;
        n = n << 8;
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

File::File(std::string *n,
        struct tm *c,
        struct tm *m,
        struct tm *a,
        int f,
        int s) : FileMeta(n, c, m, a, f, s)
{
    this->type_ = FileType::regular;
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

void Filesystem::load_directory(Directory *dir) {
    std::vector<int> file_name_address;
    std::vector<std::string> file_name;
    int address;

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
                    "Failed to load directory: missing next block in directory";
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

    this->bitmap = std::vector<bool>(this->block_count, false);

    this->filesystem_file.open(filesystem_path,
            std::fstream::in | std::fstream::binary);
    if (this->filesystem_file) {
        std::cout << "Reading filesystem file" << std::endl;

        // read bitmap
        char c;
        for (int i = 0; i <= bitmap_char_count; i++) {
            this->filesystem_file >> c;

            for (int j = 0; j < 8 && 8 * i + j < this->block_count; j++) {
                this->bitmap[8 * i + j] = (c << j) & 0x80;
            }
        }

        // move to beginning next block
        cur_pos = this->filesystem_file.tellg();
        offset = this->block_size - cur_pos % this->block_size;
        this->filesystem_file.seekg(offset, this->filesystem_file.beg);

        // read FAT
        for (int i = 0; i < this->block_count; i++) {
            this->allocation_table[i] = read_int(this->filesystem_file);
        }

        // move to beginning next block
        cur_pos = this->filesystem_file.tellg();
        offset = this->block_size - cur_pos % this->block_size;
        this->filesystem_file.seekg(offset, this->filesystem_file.beg);

        this->root = (Directory *) FileMeta::read_meta(this->filesystem_file,
                nullptr);

        this->load_directory(this->root);
    } else {
        std::cout << "Creating filesystem file" << std::endl;

        // create new filesystem file
        this->filesystem_file.open(filesystem_path,
                std::fstream::out | std::fstream::binary);

        // create bitmap
        for (int i = 0; i < bitmap_char_count; i++) {
            this->filesystem_file << (char) 'a';
        }

        // create empty FAT
        for (unsigned int i = 0; i < this->block_count * sizeof(int); i++) {
            this->filesystem_file << 'b';
        }

        // fill rest of block with zeroes
        cur_pos = this->filesystem_file.tellp();
        offset = this->block_size - cur_pos % this->block_size;
        for (int i = 0; i < offset; i++) {
            this->filesystem_file << 'c';
        }

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
            this->filesystem_file << 'd';
        }
    }
}

Filesystem::~Filesystem() {
    if (this->filesystem_file.is_open()) {
        this->filesystem_file.close();
    }
}
