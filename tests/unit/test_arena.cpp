#include "execore/common/arena.hpp"
#include <iostream>
#include <cassert>
#include <string>

struct DestructionTracker {
    static int destructed_count;
    int id{0};
    std::string text;

    DestructionTracker(int i, std::string t) : id(i), text(std::move(t)) {}
    ~DestructionTracker() {
        ++destructed_count;
    }
};

int DestructionTracker::destructed_count = 0;

struct TriviallyDestructible {
    int x;
    double y;
};

void test_arena_basic() {
    execore::ArenaAllocator arena(1024);

    auto* p1 = arena.create<TriviallyDestructible>(42, 3.14);
    assert(p1 != nullptr);
    assert(p1->x == 42);
    assert(p1->y == 3.14);

    DestructionTracker::destructed_count = 0;
    auto* p2 = arena.create<DestructionTracker>(1, "first");
    auto* p3 = arena.create<DestructionTracker>(2, "second");
    assert(p2->id == 1 && p2->text == "first");
    assert(p3->id == 2 && p3->text == "second");

    arena.reset();
    // Non-trivially destructible objects should have their destructors called
    assert(DestructionTracker::destructed_count == 2);
    assert(arena.total_allocated() == 0);

    std::cout << "[PASS] test_arena_basic\n";
}

void test_arena_growth() {
    execore::ArenaAllocator arena(64); // Small chunk size to force growth

    std::vector<int*> ints;
    for (int i = 0; i < 100; ++i) {
        ints.push_back(arena.create<int>(i * 10));
    }

    for (int i = 0; i < 100; ++i) {
        assert(*ints[static_cast<size_t>(i)] == i * 10);
    }

    assert(arena.chunk_count() > 1);
    std::cout << "[PASS] test_arena_growth\n";
}

int main() {
    test_arena_basic();
    test_arena_growth();
    std::cout << "All ArenaAllocator tests passed successfully!\n";
    return 0;
}
