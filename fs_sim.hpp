#include <ctime>
#include <fstream>
#include <string>
#include <vector>

enum class FileType {
    regular,
    directory,
};

class FileMeta {
  protected:
    std::string *name;
    struct tm *created;
    struct tm *modified;
    struct tm *accessed;
    int first_block_address;
    int size;
    FileType type_;
  public:
    static const int max_name_len = 128;

    FileMeta(std::string *name,
            struct tm *created,
            struct tm *modified,
            struct tm *accessed,
            int first_block_address,
            int size);
    ~FileMeta();
    FileType type() {
        return this->type_;
    }
    static FileMeta* read_meta(std::fstream& infile, int *address);
    // writes metadata (excluding the name) to file
    void write_meta(std::fstream& file, int name_address);
    void set_name(std::string new_name);
};

class File : public FileMeta {
  private:
    std::string content;
  public:
    File(std::string *name,
            struct tm *created,
            struct tm *modified,
            struct tm *accessed,
            int first_block_address,
            int size);
    ~File();
};

class Directory : public FileMeta {
  private:
    std::vector<FileMeta*> files;
  public:
    static const int max_files = 128;

    Directory(std::string *name,
            struct tm *created,
            struct tm *modified,
            struct tm *accessed,
            int first_block_address,
            int size);
    ~Directory();
    void add_file(FileMeta *file);
    void set_file_name(int index, std::string new_name);
};

class Filesystem {
  private:
    std::vector<bool> bitmap;
    int allocation_table[25000];
    std::fstream filesystem_file;
    Directory *root;

    void load_directory(Directory *dir);
  public:
    static const long max_size = 100000000;
    static const long block_size = 4000;
    static const long block_count = 25000;

    Filesystem(std::string filesystem_path);
    ~Filesystem();
    void copy (std::string source, std::string destination);
    void mkdir (std::string directory_name);
    void rmdir (std::string directory_name);
    void cat (std::string file_path);
    void touch (std::string file_path);
    void rm (std::string file_path);
    void ls (std::string directory_name);
    std::string find (std::string directory_name, std::string file_path);
    int df();
    };
