#include "shared.h"

#include <catch.hpp>

#include "allocations_checker.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Empty") {
    SECTION("Empty state") {
        SharedPtr<int> a, b;

        b = a;
        SharedPtr c(a);
        b = std::move(c);

        REQUIRE(a.Get() == nullptr);
        REQUIRE(b.Get() == nullptr);
        REQUIRE(c.Get() == nullptr);
    }

    SECTION("No allocations in default ctor") {
        EXPECT_ZERO_ALLOCATIONS(SharedPtr<int>());
        EXPECT_ZERO_ALLOCATIONS(SharedPtr<int>(nullptr));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Copy/move") {
    SharedPtr<std::string> a(new std::string("aba"));
    std::string* ptr;
    {
        SharedPtr b(a);
        SharedPtr c(a);
        ptr = c.Get();
    }
    REQUIRE(ptr == a.Get());
    REQUIRE(*ptr == "aba");

    SharedPtr<std::string> b(new std::string("caba"));
    {
        SharedPtr c(b);
        SharedPtr d(b);
        d = std::move(a);
        REQUIRE(*c == "caba");
        REQUIRE(*d == "aba");
        b.Reset(new std::string("test"));
        REQUIRE(*c == "caba");
    }
    REQUIRE(*b == "test");

    SharedPtr<std::string> end;
    {
        SharedPtr<std::string> d(new std::string("delete"));
        d = b;
        SharedPtr c(std::move(b));
        REQUIRE(*d == "test");
        REQUIRE(*c == "test");
        d = d;  // NOLINT
        c = end;
        d.Reset(new std::string("delete"));
        end = d;
    }

    REQUIRE(*end == "delete");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ModifiersB {
    static int count;

    ModifiersB() {
        ++count;
    }
    ModifiersB(const ModifiersB&) {
        ++count;
    }
    virtual ~ModifiersB() {
        --count;
    }
};

int ModifiersB::count = 0;

struct ModifiersA : public ModifiersB {
    static int count;

    ModifiersA() {
        ++count;
    }
    ModifiersA(const ModifiersA& other) : ModifiersB(other) {
        ++count;
    }
    ~ModifiersA() {
        --count;
    }
};

int ModifiersA::count = 0;

struct ModifiersC {
    static int count;

    ModifiersC() {
        ++count;
    }
    ModifiersC(const ModifiersC&) {
        ++count;
    }
    ~ModifiersC() {
        --count;
    }
};

int ModifiersC::count = 0;

TEST_CASE("Modifiers") {
    SECTION("Reset") {
        {
            SharedPtr<ModifiersB> p(new ModifiersB);
            p.Reset();
            REQUIRE(ModifiersA::count == 0);
            REQUIRE(ModifiersB::count == 0);
            REQUIRE(p.UseCount() == 0);
            REQUIRE(p.Get() == nullptr);
        }
        REQUIRE(ModifiersA::count == 0);
        {
            SharedPtr<ModifiersB> p;
            p.Reset();
            REQUIRE(ModifiersA::count == 0);
            REQUIRE(ModifiersB::count == 0);
            REQUIRE(p.UseCount() == 0);
            REQUIRE(p.Get() == nullptr);
        }
        REQUIRE(ModifiersA::count == 0);
    }

    SECTION("Reset ptr") {
        {
            SharedPtr<ModifiersB> p(new ModifiersB);
            ModifiersA* ptr = new ModifiersA;
            p.Reset(ptr);
            REQUIRE(ModifiersA::count == 1);
            REQUIRE(ModifiersB::count == 1);
            REQUIRE(p.UseCount() == 1);
            REQUIRE(p.Get() == ptr);
        }
        REQUIRE(ModifiersA::count == 0);
        {
            SharedPtr<ModifiersB> p;
            ModifiersA* ptr = new ModifiersA;
            p.Reset(ptr);
            REQUIRE(ModifiersA::count == 1);
            REQUIRE(ModifiersB::count == 1);
            REQUIRE(p.UseCount() == 1);
            REQUIRE(p.Get() == ptr);
        }
        REQUIRE(ModifiersA::count == 0);
    }

    SECTION("Swap") {
        {
            ModifiersC* ptr1 = new ModifiersC;
            ModifiersC* ptr2 = new ModifiersC;
            SharedPtr<ModifiersC> p1(ptr1);
            {
                SharedPtr<ModifiersC> p2(ptr2);
                p1.Swap(p2);
                REQUIRE(p1.UseCount() == 1);
                REQUIRE(p1.Get() == ptr2);
                REQUIRE(p2.UseCount() == 1);
                REQUIRE(p2.Get() == ptr1);
                REQUIRE(ModifiersC::count == 2);
            }
            REQUIRE(p1.UseCount() == 1);
            REQUIRE(p1.Get() == ptr2);
            REQUIRE(ModifiersC::count == 1);
        }
        REQUIRE(ModifiersC::count == 0);
        {
            ModifiersC* ptr1 = new ModifiersC;
            ModifiersC* ptr2 = nullptr;
            SharedPtr<ModifiersC> p1(ptr1);
            {
                SharedPtr<ModifiersC> p2;
                p1.Swap(p2);
                REQUIRE(p1.UseCount() == 0);
                REQUIRE(p1.Get() == ptr2);
                REQUIRE(p2.UseCount() == 1);
                REQUIRE(p2.Get() == ptr1);
                REQUIRE(ModifiersC::count == 1);
            }
            REQUIRE(p1.UseCount() == 0);
            REQUIRE(p1.Get() == ptr2);
            REQUIRE(ModifiersC::count == 0);
        }
        REQUIRE(ModifiersC::count == 0);
        {
            ModifiersC* ptr1 = nullptr;
            ModifiersC* ptr2 = new ModifiersC;
            SharedPtr<ModifiersC> p1;
            {
                SharedPtr<ModifiersC> p2(ptr2);
                p1.Swap(p2);
                REQUIRE(p1.UseCount() == 1);
                REQUIRE(p1.Get() == ptr2);
                REQUIRE(p2.UseCount() == 0);
                REQUIRE(p2.Get() == ptr1);
                REQUIRE(ModifiersC::count == 1);
            }
            REQUIRE(p1.UseCount() == 1);
            REQUIRE(p1.Get() == ptr2);
            REQUIRE(ModifiersC::count == 1);
        }
        REQUIRE(ModifiersC::count == 0);
        {
            ModifiersC* ptr1 = nullptr;
            ModifiersC* ptr2 = nullptr;
            SharedPtr<ModifiersC> p1;
            {
                SharedPtr<ModifiersC> p2;
                p1.Swap(p2);
                REQUIRE(p1.UseCount() == 0);
                REQUIRE(p1.Get() == ptr2);
                REQUIRE(p2.UseCount() == 0);
                REQUIRE(p2.Get() == ptr1);
                REQUIRE(ModifiersC::count == 0);
            }
            REQUIRE(p1.UseCount() == 0);
            REQUIRE(p1.Get() == ptr2);
            REQUIRE(ModifiersC::count == 0);
        }
        REQUIRE(ModifiersC::count == 0);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
