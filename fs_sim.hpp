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
    std::string name;
    struct tm created;
    struct tm modified;
    struct tm acessed;
    int first_block_address;
    FileType type_;
  public:
    FileType type() {
        return this->type_;
    }
};

class File : public FileMeta {
  private:
    int size;
    std::string content;
  public:
    File();
    ~File();
};

class Directory : public FileMeta {
  private:
    std::vector<FileMeta> files;
  public:
    Directory();
    ~Directory();
};

class Filesystem {
  private:
    std::vector<bool> bitmap;
    int allocation_table[25000];
    std::fstream filesystem_file;
    const static long max_size = 100000000;
    const static long block_size = 4000;
    const static long block_count = 25000;
  public:
    Filesystem(std::string filesystem_path);
    ~Filesystem();
    void mount (std::string filesystem_name);
    void copy (std::string source, std::string destination);
    void mkdir (std::string directory_name);
    void rmdir (std::string directory_name);
    void cat (std::string file_path);
    void rm (std::string file_path);
    void ls (std::string directory_name);
    std::string find (std::string directory_name, std::string file_path);
    int df();
    void umount();
    void exit();
};
