#ifndef SLED_SLED_GENERATOR_SLED_GENERATOR_H
#define SLED_SLED_GENERATOR_SLED_GENERATOR_H

#pragma once
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>

using namespace google::protobuf;
using namespace google::protobuf::compiler;

namespace sled {
class SledGenerator : public google::protobuf::compiler::CodeGenerator {
public:
    ~SledGenerator() override = default;
    bool Generate(const FileDescriptor *file,
                  const std::string &parameter,
                  GeneratorContext *generator_context,
                  std::string *error) const override;
};
}// namespace sled

#endif// SLED_SLED_GENERATOR_SLED_GENERATOR_H
