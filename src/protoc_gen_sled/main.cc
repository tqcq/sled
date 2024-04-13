#include "sled_generator.h"
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>
#include <google/protobuf/compiler/plugin.h>

int
main(int argc, char *argv[])
{
    google::protobuf::compiler::CommandLineInterface cli;
    // google::protobuf::compiler::cpp::CppGenerator cpp_generator;
    // cli.RegisterGenerator("--cpp_out", &cpp_generator, "Generate C++ source code.");

    sled::SledGenerator sled_generator;
    cli.RegisterGenerator("--sled_out", &sled_generator, "sled cpp rpc generator");

    return google::protobuf::compiler::PluginMain(argc, argv, &sled_generator);
}
