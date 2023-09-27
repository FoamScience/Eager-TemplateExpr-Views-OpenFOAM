#include "ListOps.H"
#include "catch2/catch_all.hpp"
#include "catch2/catch_message.hpp"
#include "catch2/catch_test_macros.hpp"
#include "vector.H"
#include "Field.H"
#include "Random.H"
#include <functional>
#include <ranges>

// This file benchmarks a trivial expression template implementation against views
// for lazy evaluation

using namespace Foam;
label ELEMENTS_COUNT = 5;

// Default mode of computation
template<class T>
Field<T> eagerEvaluation(const scalarField& r) {
    Field<T> vf(ELEMENTS_COUNT, zero());
    auto W = [&](const scalar x) {
        return Foam::exp(-pow(1e-2*x,2));
    };
    scalarField rho(vf.size(), 1.0);
    scalarField m(vf.size(), 0.5);
    scalarField w(vf.size(), 0.0);
    for (int i = 0; i<vf.size(); ++i) {
        w[i] = W(r[i]);
    }
    vf = m * w / rho + (m + rho) * w / m;
    return vf;
}

// Trivial expression template implementation on containers
template<class T, class U, class Callable>
struct BinaryContainerExpression
{
    template<class Func>
    BinaryContainerExpression(const T& _left, const U& _right, Func&& _callable) :
    left_{&_left},
    right_{&_right},
    callable_{std::forward<Func>(_callable)}
    {
        assert(_left.size() == _right.size());
    }
    auto operator[](size_t index) const
    {
        return callable_((*left_)[index], (*right_)[index]);
    }
    size_t size() const
    {
        return left_->size();
    }
    const T* left_= nullptr;
    const U* right_= nullptr;
    Callable callable_;
};
template<class T, class U, class Func>
BinaryContainerExpression(const T&, const U&, Func&&) -> BinaryContainerExpression<T, U, Func>;

template<class U>
Field<U> expressionTemplatesEvaluation(const scalarField& r) {
    auto multiply = []<class T>(T lhs, T rhs){return lhs * rhs;};
    auto divide = []<class T>(T lhs, T rhs){return lhs / rhs;};
    auto add = []<class T>(T lhs, T rhs){return lhs + rhs;};
    auto subtract = []<class T>(T lhs, T rhs){return lhs - rhs;};
    Field<U> vf(ELEMENTS_COUNT, zero());
    auto W = [&](const scalar x) {
        return Foam::exp(-pow(1e-2*x,2));
    };
    scalarField rho(vf.size(), 1.0);
    scalarField m(vf.size(), 0.5);
    scalarField w(vf.size(), 0.0);
    for (int i = 0; i<vf.size(); ++i) {
        w[i] = W(r[i]);
    }
    auto mw = BinaryContainerExpression(m, w, multiply);
    auto mwdr = BinaryContainerExpression(mw, rho, divide);
    auto mpr = BinaryContainerExpression(m, rho, add);
    auto mprw = BinaryContainerExpression(mpr, w, multiply);
    auto mprwdm = BinaryContainerExpression(mpr, m, divide);
    auto vfv = BinaryContainerExpression(mwdr, mprwdm, add);
    //for(int i = 0; i<vf.size(); ++i) {
    //    vf[i] = vfv[i];
    //}
    return vf;
}

// Views implementation
using namespace std::ranges;
using namespace std::views;
//using std::ranges::views::zip_transform;

template<class T>
Field<T> viewsEvaluation(const scalarField& r) {
    Field<T> vf(ELEMENTS_COUNT, zero());
    auto W = [&](const scalar x) {
        return Foam::exp(-pow(1e-2*x,2));
    };
    scalarField rho(vf.size(), 1.0);
    scalarField m(vf.size(), 0.5);
    scalarField w(vf.size(), 0.0);
    for (int i = 0; i<vf.size(); ++i) {
        w[i] = W(r[i]);
    }
    auto mw = zip_transform(std::multiplies<>{}, m, w);
    auto mwdr = zip_transform(std::divides<>{}, mw, rho);
    auto mpr = zip_transform(std::plus<>{}, m, rho);
    auto mprw = zip_transform(std::multiplies<>{}, mpr, w);
    auto mprwdm = zip_transform(std::divides<>{}, mprw, m);
    auto vfv = zip_transform(std::plus<>{}, mwdr, mprwdm);
    //for(int i = 0; i<vf.size(); ++i) {
    //    vf[i] = vfv[i];
    //}
    return vf;
}



TEMPLATE_TEST_CASE
(
    "Evaluation time of field operations in OpenFOAM",
    "[cavity][serial][benchmark]",
    scalar
) {
    scalarField r(ELEMENTS_COUNT, 1.0);
    for (auto& ri: r) {
        ri = Random().sample01<scalar>();
    }
    auto ee = eagerEvaluation<TestType>(r);
    auto ete = expressionTemplatesEvaluation<TestType>(r);
    auto vw = viewsEvaluation<TestType>(r);
    auto testExp = Catch::Matchers::Predicate<Field<TestType>>
    (
        [&ee](const Field<TestType>& result) {
        if (result.size() != ee.size()) return false;
        for (int i = 0; i < result.size(); ++i)
            if (Catch::Approx(result[i]).margin(1e-3) != ee[i]) return false;
        return true;
        },
        "Field elements must approximately match"
    );
    //REQUIRE_THAT(ete, testExp);
    //REQUIRE_THAT(vw, testExp);
    BENCHMARK("Original eager evaluation") {
        return eagerEvaluation<TestType>(r);
    };
    BENCHMARK("Expression template implementation") {
        return expressionTemplatesEvaluation<TestType>(r);
    };
    BENCHMARK("Views implementation") {
        return viewsEvaluation<TestType>(r);
    };
}
