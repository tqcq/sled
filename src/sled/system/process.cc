#include "sled/system/process.h"

namespace sled {
Process::Process(const Option &option) : option(option) {}

Process::~Process() noexcept { CloseAllFileDescriptors(); }

}// namespace sled
