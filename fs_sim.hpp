#include <ctime>
#include <fstream>
#include <string>
#include <vector>

enum class FileType {
    regular,
    directory,
    deleted,
};

class FileMeta {
  protected:
    std::string name;
    time_t created;
    time_t modified;
    time_t accessed;
    int first_block_address;
    int size;
    FileType type_;
  public:
    static const int max_name_len = 128;

    FileMeta(std::string name,
            time_t created,
            time_t modified,
            time_t accessed,
            int first_block_address,
            int size);
    ~FileMeta();
    FileType type() {
        return this->type_;
    }
    static FileMeta* read_meta(std::fstream& infile, int *address);
    // writes metadata (excluding the name) to file
    void write_meta(std::fstream& file, int name_address);
    void print_meta();
    void set_name(std::string new_name);
    std::string get_name();
    int get_address();
    time_t get_last_modified();
    void set_last_modified(time_t moment);
    void set_last_accessed(time_t moment);
    bool is_loaded;
    void remove();
};

class File : public FileMeta {
  private:
    std::string content;
  public:
    File(std::string name,
            time_t created,
            time_t modified,
            time_t accessed,
            int first_block_address,
            int size);
    ~File();

    std::string get_content();
    void set_content(std::string new_content);
    int get_size();
};

class Directory : public FileMeta {
  private:
      std::vector<FileMeta*> files;
  public:
    static const int max_files = 128;

    Directory(std::string name,
            time_t created,
            time_t modified,
            time_t accessed,
            int first_block_address,
            int size);
    ~Directory();
    void add_file(FileMeta *file);
    void push_file(FileMeta *file);
    void set_file_name(int index, std::string new_name);
    FileMeta* get_file(std::string name);
    FileMeta* get_file(int index);
    void del_file(std::string file_name);
    int get_file_count();
    int get_size();
    int find_inside(std::string curpath, std::string file);
    FileMeta* at(int index);
    bool is_loaded;
};

class Filesystem {
  private:
    std::vector<bool> bitmap;
    int allocation_table[25000];
    std::fstream filesystem_file;
    Directory *root;

    // offset to beginning of root directory
    static const int FAT_offset = 3125;
    static const int first_block_offset = 103125;

    void load_file(File *file);
    void save_file(File *file);

    void load_directory(Directory *dir);
    void save_directory(Directory *dir);

    void move_to_block(int block);
    int get_read_position();
    int get_write_position();

    void set_bit(int index, bool value);
    void set_FAT(int index, int value);
  public:
    static const long max_size = 100000000;
    static const long block_size = 4000;
    static const long block_count = 24968;

    Filesystem(std::string filesystem_path);
    ~Filesystem();
    void copy (std::string source_path, std::string dest_path);
    void mkdir (std::string dir_name);
    void rmdir (std::string dir_name);
    void cat (std::string file_path);
    void touch (std::string file_path);
    void rm (std::string file_path);
    void ls (std::string dir_name);
    void find (std::string dir_name, std::string file_name);
    void df();
};

std::vector<std::string> split_path(std::string file_path);
