#include "fs_sim.hpp"
#include <fstream>
#include <string>
#include <iostream>

Filesystem::Filesystem(std::string filesystem_path) {
    int bitmap_char_count = this->block_count / 8;
    long cur_pos, offset;

    this->bitmap = std::vector<bool>(this->block_count, false);

    this->filesystem_file.open(filesystem_path, std::fstream::in);
    if (this->filesystem_file) {
        std::cout << "File exists" << std::endl;
        
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
            this->filesystem_file >> this->allocation_table[i];
        }

        // move to beginning next block
        cur_pos = this->filesystem_file.tellg();
        offset = this->block_size - cur_pos % this->block_size;
        this->filesystem_file.seekg(offset, this->filesystem_file.beg);
    } else {
        std::cout << "File does not exist" << std::endl;
        
        // create new filesystem file
        this->filesystem_file.open(filesystem_path, std::fstream::out);

        for (int i = 0; i < bitmap_char_count; i++) {
            this->filesystem_file << '\0';
        }

        // create empty FAT
        for (unsigned int i = 0; i < this->block_count * sizeof(int); i++) {
            this->filesystem_file << '\0';
        }

        // fill rest of block with zeroes
        cur_pos = this->filesystem_file.tellg();
        offset = this->block_size - cur_pos % this->block_size;
        for (int i = 0; i < offset; i++) {
            this->filesystem_file << '\0';
        }
    }
}

Filesystem::~Filesystem() {
    if (this->filesystem_file.is_open()) {
        this->filesystem_file.close();
    }
}
