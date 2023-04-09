#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <vector>
void f();
namespace {
bool exec(std::string cmd) {
  system(cmd.c_str());
  return true;
}
} // namespace
// TODO: add create to create a new project.
namespace b77 {
class Base {
public:
  virtual ~Base() = default;
  virtual bool config() = 0;
  virtual bool build() = 0;
  virtual bool clean() = 0;
  virtual bool run() = 0;
  virtual bool package() = 0;
  virtual bool install() = 0;
  virtual bool check(const std::vector<std::string> &files_to_check) = 0;
};
std::unique_ptr<Base>
detect_env(const std::filesystem::path &path = std::filesystem::current_path());
class Tp : public Base {
public:
  ~Tp() = default;
  bool config() override { return false; }
  bool build() override { return false; }
  bool clean() override { return false; }
  bool run() override { return false; }
  bool package() override { return false; }
  bool install() override { return false; }
  bool check(const std::vector<std::string> &files_to_check) override {
    return false;
  }
};

class Cpp : public Base {
public:
  Cpp(const std::filesystem::path &path) : project_dir_(path) {}
  ~Cpp() = default;
  bool config() override {
    exec("cmake -B build");
    return false;
  }
  bool build() override {
    if (!exists(project_dir_ / "build")) {
      config();
    }
    exec("cmake --build build -j");
    return false;
  }
  bool clean() override {
    exec("rm -r build");
    return false;
  }
  bool run() override {
    auto d = project_dir_ / "build";
    for (std::filesystem::directory_iterator begin{d}, end; begin != end;
         ++begin) {
      if (!begin->is_regular_file()) {
        continue;
      }
      auto t = begin->status().type();
      auto p = begin->status().permissions();
      if (int(p & decltype(p)::owner_exec)) {
        exec(begin->path().string().data());
      }
    }
    return false;
  }
  bool package() override {
    exec("cmake --build build -j --target package");
    return false;
  }
  bool install() override {
    exec("cmake --build build -j --target install");
    return false;
  }
  bool check(const std::vector<std::string> &files_to_check) override {

    f();
    return false;
  }

private:
  std::filesystem::path project_dir_;
};
std::unique_ptr<Base> detect_env(const std::filesystem::path &path) {
  if (!exists(path)) {
    return nullptr;
  }
  // check cmake
  if (exists(path / "CMakeLists.txt")) {
    return std::make_unique<Cpp>(path);
  }

  return nullptr;
}
} // namespace b77

int main(int argc, char **argv) {

  std::string operation = "build";
  if (argc > 1) {
    operation = argv[1];
  }
  auto env = b77::detect_env();
  if (!env) {
    std::cout << "Not supported enviroment." << std::endl;
    return 1;
  }
  std::vector<std::string> command_list;
  command_list.push_back("check [targets...]");
#define CK(op)                                                                 \
  else if (operation == #op || [&] {                                           \
             command_list.push_back(#op);                                      \
             return false;                                                     \
           }()) {                                                              \
    env->op();                                                                 \
  }


  if (false) {
  }
  CK(config)
  CK(build)
  CK(run)
  CK(package)
  CK(install)
  else if (operation == "check") {
    std::vector<std::string> file_to_check;
    for (int i = 2; i < argc; i++) {
      file_to_check.push_back(argv[i]);
    }
    env->check(file_to_check);
  }
  else if (operation == "help" || operation == "--help") {
    command_list.push_back("help");
    std::cout << "usage: " << argv[0] << " [command:default=build]\n";
    for(auto cmd:command_list){
        std::cout << cmd <<"\n";
    }
  }
  else {
    std::cout << "Unknown operation: " << operation << std::endl;
    return 1;
  }
}

std::string code_gen(std::vector<std::string> names) {
  std::string result;
  result += " std::vector<std::string> commands = {";
  for (auto &n : names) {
    result += "\"" + n + "\",";
  }
  result += "};";
  result += "if(false){}\n";
  for (auto &n : names) {
    result += R"**("else if (operation == ")**" + n +
              R"**(") {
    env->")**" +
              n +
              R"**(();
  })**";
  }
  result += R"**(else {
    std::cout << "Unknown operation: " << operation << std::endl;
    return 1;
  })**";
  return result;
}

#include <json/json.h>

std::string read_file(std::string path) {
  std::ifstream f{path};
  std::string result, line;
  while (getline(f, line)) {
    result += line;
  }
  return result;
}
void f() {
  Json::Reader read;
  Json::Value v;
  std::cout << "reading json"
            << read.parse(read_file("build/compile_commands.json"), v) << "\n";
  std::string names;
  for (int i = 0; i < v.size(); i++) {

    // for (auto m : v[i].getMemberNames()) {
    //   std::cout << m << ": " << v[i][m] << "\n";
    std::string key = v[i]["command"].asCString();
    std::string name;
    bool ff = false;
    for (auto w : std::views::split(key, std::string_view{" "})) {
      // std::cout << "sp "<< std::string_view{w.begin(), w.end()}<<"\n";
      if (ff) {
        name = {w.begin(), w.end()};
        std::cout << "check " << v[i]["file"] << " target is " << name << "\n";
        names += name + " ";

        break;
      }
      if (std::string_view{w.data(), w.size()} == "-o") {
        // std::cout <<"output file found!\n";
        ff = true;
      }
    }
    // }
  }
  if (!names.empty()) {

    std::string cmd = "cmake --build build --target " + names;
    exec(cmd);
  }
}