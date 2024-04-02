#include <sled/sigslot.h>

struct DeleteSelf : public sigslot::has_slots<> {
    void DeleteThis() {
        delete this;
    }
};

TEST_SUITE("sigslot") {
    TEST_CASE("delete this") {
        DeleteSelf* d = new DeleteSelf();
        sigslot::signal0<> sig;
        sig.connect(d, &DeleteSelf::DeleteThis);
        sig.emit();
    }
}
