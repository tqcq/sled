#include <sled/ioc/ioc.h>
#include <string>

class IRunner {
public:
    virtual ~IRunner()     = default;
    virtual int RunSpeed() = 0;
};

class IWalker {
public:
    virtual ~IWalker() {}

    virtual int WalkSpeed() = 0;
};

class Cheetah : public IRunner, public IWalker {
public:
    int WalkSpeed() override { return 4; }

    int RunSpeed() override { return 100; }
};

class Formatter {
public:
    std::string Format(int value) { return std::to_string(value); }
};

class Service {
public:
    Service(std::shared_ptr<Formatter> formatter) : formatter_(formatter) {}

    std::string Format(int value) { return formatter_->Format(value); }

private:
    std::shared_ptr<Formatter> formatter_;
};

TEST_SUITE("Inversion Of Control")
{
    TEST_CASE("base")
    {
        sled::ioc::Container container;
        container.Bind<IRunner, IWalker>().To<Cheetah>();
        auto runner = container.Get<IRunner>();
        auto walker = container.Get<IWalker>();
        CHECK(runner);
        CHECK(walker);
        CHECK_EQ(runner.use_count(), 1);
        CHECK_EQ(walker.use_count(), 1);
        CHECK_EQ(runner->RunSpeed(), 100);
        CHECK_EQ(walker->WalkSpeed(), 4);
    }

    TEST_CASE("deps")
    {
        sled::ioc::Container container;
        container.Bind<Formatter>().ToSelf();
        container.Bind<Service>().ToSelf();
        auto service = container.Get<Service>();
        CHECK_EQ(service->Format(42), "42");
        CHECK_EQ(service->Format(44), "44");
    }
}
