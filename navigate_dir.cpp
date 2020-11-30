std::vector<std::string> split_path(std::string file_path) {
    std::vector<std::string> words;
    auto start = 0U;
    auto end = file_path.find("/", start);

    while (end != std::string::npos) {
        words.push_back(file_path.substr(start, end - start));
        start = end + 1;
        end = file_path.find("/", start);
    }

    return words;
}

std::vector<std::string> path_names = split_path(path);
Directory *dir = this->root;

for (int i = 0; i < dest_path.size() - 1; i++) {
    dir = (Directory *) dir->get_file(dest_path_names[i]);
    this->load_directory(dir);
}
