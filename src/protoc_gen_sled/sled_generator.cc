#include "sled_generator.h"
#include <google/protobuf/compiler/plugin.pb.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <sled/sled.h>
using namespace google::protobuf;
using namespace google::protobuf::compiler;
using namespace google::protobuf::io;

namespace sled {

void
PrintMessage(Printer &printer, const Descriptor *msg)
{
    printer.Print("- $name$\n", "name", msg->name());
    printer.Indent();
    for (int i = 0; i < msg->nested_type_count(); ++i) { PrintMessage(printer, msg->nested_type(i)); }
    printer.Outdent();
}

void
PrintService(Printer &printer, const ServiceDescriptor *service)
{
    inja::json data;
    data["classname"] = service->name();
    data["full_name"] = service->full_name();

    printer.Print("- $name$\n", "name", service->full_name());
    printer.Indent();
    for (int i = 0; i < service->method_count(); i++) {
        const MethodDescriptor *method = service->method(i);
        printer.Print("- $name$\n", "name", method->name());
        printer.Indent();
        printer.Print("request: $name$ $file$\n", "name", method->input_type()->name(), "file", method->file()->name());
        printer
            .Print("response: $name$ $file$\n", "name", method->output_type()->name(), "file", method->file()->name());
        printer.Outdent();
    }
    printer.Outdent();
}

bool
SledGenerator::Generate(const FileDescriptor *file,
                        const std::string &parameter,
                        GeneratorContext *generator_context,
                        std::string *error) const
{
    auto *out_stream = generator_context->Open(file->name() + ".txt");
    Printer printer(out_stream, '$');
    for (int i = 0; i < file->message_type_count(); i++) { PrintMessage(printer, file->message_type(i)); }
    for (int i = 0; i < file->service_count(); i++) { PrintService(printer, file->service(i)); }
    return true;
}
}// namespace sled
