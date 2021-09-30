#include <iostream>
#include <functional>

template <typename FuncType>
struct Finally {
    Finally(FuncType&& _f) noexcept : f(std::forward<FuncType>(_f)) {}
    Finally(Finally&& other) = default;
    Finally& operator=(Finally&& other) = default;   
    ~Finally() {
        try {
            f();
        } catch(const std::exception &e) {
            std::cerr << e.what() << '\n';
        } catch(...) {
            std::cerr << "unknown exception\n";
        }
    }
private:
    FuncType f;
};

template <typename FuncType>
auto makeFinally(FuncType &&f) noexcept -> Finally<FuncType> {
    return Finally<FuncType>(std::forward<FuncType>(f));
}
