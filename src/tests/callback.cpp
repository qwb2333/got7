#include <cstdio>
#include <functional>

class TestClass {
public:
    TestClass(std::function<void()> func = TestClass::blank) {
        this->func = func;
    }
    void run() {
        this->func();
    }

    static void blank() {

    }

private:
    std::function<void()> func;
};

int main() {
    TestClass test([](){
        std::printf("hello world");
    });
    test.run();
    return 0;
}