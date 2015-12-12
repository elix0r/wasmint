
#include <iostream>
#include <fstream>
#include <vector>
#include <binary_parsing/ByteStream.h>
#include <Module.h>
#include <binary_parsing/ModuleParser.h>
#include <interpreter/at/MachineState.h>
#include <sexpr_parsing/ModuleParser.h>
#include <sexpr_parsing/SExprParser.h>
#include <builtins/StdioModule.h>
#include <builtins/SDLModule.h>

using namespace wasm_module;
using namespace wasmint;

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

int main(int argc, char** argv) {
    bool runMain = true;

    if (argc == 1) {
        std::cerr << "No modules given. Call programm like this: \n$ wasmint module1.wasm" << std::endl;
        return 1;
    }

    Module* mainModule = nullptr;

    MachineState environment;

#ifdef WASMINT_HAS_SDL
    environment.useModule(*SDLModule::create(), true);
#endif
    environment.useModule(*StdioModule::create(), true);

    for(int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg.find("--") == 0) {
            if (arg == "--no-run") {
                runMain = false;
            } else {
                std::cerr << "Unknown argument " << arg << std::endl;
                return 2;
            }
        }


        const std::string& modulePath = argv[i];

        Module* m;

        bool binary = !ends_with(modulePath, ".wasm");

        if (binary) {
            std::streampos size;
            std::vector<uint8_t> data;

            std::ifstream file(modulePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
            if (file.is_open()) {
                size = file.tellg();
                data.resize(size);
                file.seekg(0, std::ios::beg);
                file.read((char *) data.data(), size);
                file.close();
            }
            else std::cerr << "Unable to open module " << modulePath;

            binary::ByteStream stream(std::deque<uint8_t>(data.begin(), data.end()));


            try {
                m = binary::ModuleParser::parse(stream);
                m->name(modulePath);
            } catch (const std::exception& e) {
                std::cerr << "Got exception while parsing binary module "
                << modulePath << ": " << e.what() << " (typeid name " << typeid(e).name() << ")"
                << std::endl;
                return 1;
            }

        } else {

            std::ifstream moduleFile(modulePath);
            std::string moduleData((std::istreambuf_iterator<char>(moduleFile)),
                            std::istreambuf_iterator<char>());
            sexpr::CharacterStream stream(moduleData);


            try {
                sexpr::SExpr expr = sexpr::SExprParser(stream).parse(true);
                m = sexpr::ModuleParser::parse(expr[0]);
            } catch (const std::exception& e) {
                std::cerr << "Got exception while parsing sexpr module "
                << modulePath << ": " << e.what() << " (typeid name " << typeid(e).name() << ")"
                << std::endl;
                return 1;
            }
        }

        try {
            environment.useModule(*m, true);

            if (runMain) {
                try {
                    m->getFunction("main");

                    if (mainModule != nullptr) {
                        std::cerr << "Multiple modules with a main function! Aborting..." << std::endl;
                        std::cerr << "Module 1 was " << m->name() << ", Module 2 was " << mainModule->name() << std::endl;
                        return 1;
                    }
                    mainModule = m;
                } catch (const std::exception& ex) {
                    // has no main function
                }
            }


        } catch (const std::exception& e) {
            std::cerr << "Got exception while parsing module " << modulePath << ": " << e.what() << std::endl;
        }
    }

    if (!runMain) {
        return 0;
    }

    if (mainModule == nullptr) {
        std::cerr << "No module loaded with a main function! Aborting..." << std::endl;
        return 1;
    }

    try {
        InterpreterThread & thread = environment.createThread().startAtFunction(mainModule->name(), "main");
        thread.stepUntilFinished();
        if (thread.gotTrap()) {
            std::cerr << "Got trap while executing program" << std::endl;
            return 2;
        }
    } catch(const wasm_module::NoFunctionWithName& e) {
        if (std::string(e.what()) == "main") {
            std::cerr << "None of the given modules has a main function. Exiting..." << std::endl;
        } else {
            std::cerr << "Exiting because we can't find function with name: " << e.what() << std::endl;
        }
    } catch(const std::exception& ex) {
        std::cerr << "Got exception while executing: " << ex.what() << " (typeid name " << typeid(ex).name() << ")"
        << std::endl;
        return 1;
    }
}