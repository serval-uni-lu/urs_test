#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>

namespace fs = std::filesystem;

struct SourceFile {
    std::string src_path;
    std::string obj_path;
    std::string elf_path;
};

std::string gen_rule(SourceFile const& s) {
    std::string res;
    res += "build " + s.obj_path + ": ";
    res += "compile " + s.src_path + "\n";

    res += "build " + s.obj_path + "d: ";
    res += "compile_debug " + s.src_path + "\n";
    return res;
}

std::string gen_ld_rule(SourceFile const& s, std::vector<SourceFile> const& deps) {
    std::string res;
    res += "build " + s.elf_path + ": link " + s.obj_path;
    for(auto const& c : deps) {
        if(c.elf_path == "") {
            res += " " + c.obj_path;
        }
    }
    res += "\n";

    res += "build " + s.elf_path + "_debug: link_debug " + s.obj_path + "d";
    for(auto const& c : deps) {
        if(c.elf_path == "") {
            res += " " + c.obj_path + "d";
        }
    }
    res += "\n";
    return res;
}

std::string gen_variable(std::string const& var, std::vector<std::string> const& vals) {
    std::string res = var + " =";
    for(auto const& v : vals) {
        res += " " + v;
    }
    return res;
}

class Project {
private:
    std::set<std::string> obj_folders;

    std::vector<SourceFile> src;
    std::vector<std::string> headers;
    std::vector<std::string> cc_options;
    std::vector<std::string> ld_options;

    std::vector<std::string> cc_r_options;
    std::vector<std::string> ld_r_options;

    std::vector<std::string> cc_d_options;
    std::vector<std::string> ld_d_options;

public:
    void add_source_folder(std::string const& path, std::set<std::string> const& extensions) {
        for(auto const& f : fs::recursive_directory_iterator(path)) {
            if(f.is_regular_file()) {
                if(extensions.find(f.path().extension()) != extensions.end()) {
                    add_source_file(f.path());
                }
            }
        }
    }

    void add_source_file(std::string const& path, bool const is_main = false) {
        SourceFile f;
        f.src_path = path;
        f.obj_path = "build/obj/" + path + ".o";
        if(is_main) {
            f.elf_path = "build/" + fs::path(path).stem().string();
        }
        src.push_back(f);
        obj_folders.insert(fs::path(f.obj_path).parent_path());
    }

    void set_main_property(std::string const& path, bool const is_main = true) {
        for(auto & e : src) {
            if(e.src_path == path) {
                if(is_main) {
                    e.elf_path = "build/" + fs::path(path).stem().string();
                }
                else {
                    e.elf_path = "";
                }
            }
        }
    }

    void add_header_file(std::string const& path) {
        headers.push_back(path);
    }

    void add_compile_option(std::string const& o) {
        cc_options.push_back(o);
    }

    void add_linker_option(std::string const& o) {
        ld_options.push_back(o);
    }

    void add_realease_compile_option(std::string const& o) {
        cc_r_options.push_back(o);
    }

    void add_release_linker_option(std::string const& o) {
        ld_r_options.push_back(o);
    }

    void add_debug_compile_option(std::string const& o) {
        cc_d_options.push_back(o);
    }

    void add_debug_linker_option(std::string const& o) {
        ld_d_options.push_back(o);
    }

    void gen_makefile() {
        std::ofstream out("build.ninja");

        out << "cxx = g++\n";
        out << "cc = gcc\n";
        out << gen_variable("cxxflags", cc_options) << "\n";
        out << gen_variable("ldflags", ld_options) << "\n";
        out << gen_variable("cxxflagsrel", cc_r_options) << "\n";
        out << gen_variable("ldflagsrel", ld_r_options) << "\n";
        out << gen_variable("cxxflagsdebug", cc_d_options) << "\n";
        out << gen_variable("ldflagsdebug", ld_d_options) << "\n\n";

        out << "rule compile\n";
        out << "    command = $cxx -fdiagnostics-color=always $cxxflags $cxxflagsrel -c $in -o $out\n";
        out << "    description = Compiling $in\n\n";

        out << "rule compile_debug\n";
        out << "    command = $cxx -fdiagnostics-color=always $cxxflags $cxxflagsdebug -c $in -o $out\n";
        out << "    description = Compiling (DEBUG) $in\n\n";

        out << "rule link\n";
        out << "    command = $cxx -fdiagnostics-color=always $in -o $out $ldflags $ldflagsrel\n";
        out << "    description = Linking $out\n\n";

        out << "rule link_debug\n";
        out << "    command = $cxx -fdiagnostics-color=always $in -o $out $ldflags $ldflagsdebug\n";
        out << "    description = Linking (DEBUG) $out\n\n";

        out << "rule clean\n";
        out << "    command = rm -rf build\n";
        out << "    description = Cleaning\n\n";

        out << "rule generate\n";
        out << "    command = $cxx -fdiagnostics-color=always gen.cpp -o gen && ./gen\n";
        out << "    description = Generating build.ninja\n\n";

        for(auto const& cpp : src) {
            out << gen_rule(cpp);

            if(cpp.elf_path != "") {
                out << gen_ld_rule(cpp, src);
            }
            out << "\n";
        }

        out << "build all_release: phony";
        for(auto const& f : src) {
            if(f.elf_path != "") {
                out << " " << f.elf_path;
            }
        }
        out << "\n";

        out << "build all_debug: phony";
        for(auto const& f : src) {
            if(f.elf_path != "") {
                out << " " << f.elf_path << "_debug";
            }
        }
        out << "\n";

        out << "build all: phony all_release all_debug\n";
        out << "build clean: clean\n";
        out << "build generate: generate\n";
        out << "default all_release\n";
    }
};

int main(void) {
    Project prj;
    prj.add_compile_option("-Wall");
    prj.add_compile_option("-Wextra");
    prj.add_compile_option("-Wpedantic");
    prj.add_compile_option("-fopenmp");
    prj.add_compile_option("-pthread");

    prj.add_realease_compile_option("-O3");

    prj.add_debug_compile_option("-g");

    prj.add_linker_option("-lboost_program_options");
    prj.add_linker_option("-fopenmp");
    prj.add_linker_option("-pthread");

    prj.add_source_folder("src", {".cpp"});

    prj.set_main_property("src/projection.cpp");

    prj.gen_makefile();
    
    return 0;
}
