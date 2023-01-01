#include <boost/test/unit_test.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <limits>
#include <map>

#include "sorting/Sorting.h"

BOOST_AUTO_TEST_SUITE(TestSorting)

BOOST_AUTO_TEST_CASE(TestPersistence)
{
    sorting::QuickSortState state;
    BOOST_CHECK(!sorting::validateState(state));
    BOOST_CHECK(!sorting::persistStateToDisk("sort_state.log", state));
    state.n = 2;
    state.arr.push_back(0);
    state.arr.push_back(1);
    state.stack.push_back(2);
    state.stack.push_back(3);
    state.top = 0;
    state.p   = 8;
    state.i   = 0;
    state.j   = 1;
    state.l   = 1;
    state.c   = 2;
    BOOST_CHECK(sorting::persistStateToDisk("sort_state.log", state));
    BOOST_CHECK(!sorting::sortStateFromDisk("fake_log.log").first);
    auto [read_success, state2] = sorting::sortStateFromDisk("sort_state.log");
    BOOST_CHECK(read_success);
    BOOST_CHECK_EQUAL(state.n, state2.n);
    BOOST_CHECK_EQUAL(state.arr[0], state2.arr[0]);
    BOOST_CHECK_EQUAL(state.arr[1], state2.arr[1]);
    BOOST_CHECK_EQUAL(state.stack[0], state2.stack[0]);
    BOOST_CHECK_EQUAL(state.stack[1], state2.stack[1]);
    BOOST_CHECK_EQUAL(state.top, state2.top);
    BOOST_CHECK_EQUAL(state.p, state2.p);
    BOOST_CHECK_EQUAL(state.i, state2.i);
    BOOST_CHECK_EQUAL(state.j, state2.j);
    BOOST_CHECK_EQUAL(state.l, state2.l);
    BOOST_CHECK_EQUAL(state.c, state2.c);
}

BOOST_AUTO_TEST_CASE(TestIncrementalSortingShort)
{
    std::map<uint32_t, double> values  = {{0, 4.8}, {1, 10.0}, {2, 1.0}, {3, 2.5}, {4, 5.0}};
    std::vector<uint32_t>      indices = {0, 1, 2, 3, 4};
    std::vector<uint32_t>      stack(5, 0);

    sorting::QuickSortState state;
    state.n     = 5;
    state.arr   = indices;
    state.stack = stack;

    auto updateComparator = [](const double& a, const double& b) {
        if (a < b)
        {
            return sorting::LEFT_LESS;
        }
        else if (a > b)
        {
            return sorting::LEFT_GREATER;
        }
        else
        {
            return sorting::LEFT_EQUAL;
        }
    };

    uint64_t       iter     = 0;
    const uint64_t maxIters = 50;
    while (!(state.top == std::numeric_limits<uint32_t>::max() && state.c != 0) && iter < maxIters)
    {
        auto [iter_success, state_out] = sorting::restfulRandomizedQuickSort(state);
        // auto [iter_success, state_out] = sorting::restfulQuickSort(state);
        BOOST_CHECK(iter_success);
        state = state_out;
        if (state.l == sorting::LEFT_I)
        {
            state.c = updateComparator(values[state.arr[state.i]], values[state.arr[state.p]]);
        }
        else if (state.l == sorting::LEFT_J)
        {
            state.c = updateComparator(values[state.arr[state.j]], values[state.arr[state.p]]);
        }
        iter++;
    }

    BOOST_CHECK_EQUAL(state.arr[0], 2);
    BOOST_CHECK_EQUAL(state.arr[1], 3);
    BOOST_CHECK_EQUAL(state.arr[2], 0);
    BOOST_CHECK_EQUAL(state.arr[3], 4);
    BOOST_CHECK_EQUAL(state.arr[4], 1);
}

BOOST_AUTO_TEST_CASE(TestIncrementalSortingLong)
{
    auto updateComparator = [](const uint32_t& a, const uint32_t& b) {
        if (a < b)
        {
            return sorting::LEFT_GREATER;
        }
        else if (a > b)
        {
            return sorting::LEFT_LESS;
        }
        else
        {
            return sorting::LEFT_EQUAL;
        }
    };

    const uint32_t n = 20;

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < n; i++)
    {
        indices.push_back(i);
    }
    std::vector<uint32_t> stack(n, 0);

    sorting::QuickSortState state;
    state.n     = n;
    state.arr   = indices;
    state.stack = stack;

    uint64_t       iter     = 0;
    const uint64_t maxIters = 500;

    while (!(state.top == std::numeric_limits<uint32_t>::max() && state.c != 0) && iter < maxIters)
    {
        auto [iter_success, state_out] = sorting::restfulQuickSort(state);
        BOOST_CHECK(iter_success);
        state = state_out;
        if (state.l == sorting::LEFT_I)
        {
            state.c = updateComparator(state.arr[state.i], state.arr[state.p]);
        }
        else if (state.l == sorting::LEFT_J)
        {
            state.c = updateComparator(state.arr[state.j], state.arr[state.p]);
        }
        iter++;
    }

    std::stringstream ss;
    ss << "Long sort result: ";
    for (uint32_t i = 0; i < n; i++)
    {
        ss << state.arr[i] << " ";
    }
    BOOST_LOG_TRIVIAL(debug) << ss.str();

    for (uint32_t i = 0; i < n; i++)
    {
        BOOST_CHECK_EQUAL(state.arr[i], n - 1 - i);
    }
}

BOOST_AUTO_TEST_CASE(TestIncrementalSortingMultiRefine)
{
    const uint32_t numRefines = 4;

    auto updateComparator = [](const double& a, const double& b) {
        if (a < b)
        {
            return sorting::LEFT_LESS;
        }
        else if (a > b)
        {
            return sorting::LEFT_GREATER;
        }
        else
        {
            return sorting::LEFT_EQUAL;
        }
    };

    std::map<uint32_t, double> values  = {{0, 4.8}, {1, 10.0}, {2, 1.0}, {3, 2.5}, {4, 5.0}, {5, 7.5}};
    std::vector<uint32_t>      indices = {0, 1, 2, 3, 4, 5}; // 2 3 0 4 5 1
    std::vector<uint32_t>      stack(6, 0);

    sorting::QuickSortState state;
    state.n     = 6;
    state.arr   = indices;
    state.stack = stack;

    for (uint32_t j = 0; j < numRefines; j++)
    {
        uint64_t       iter     = 0;
        const uint64_t maxIters = 50;
        while (!(state.top == std::numeric_limits<uint32_t>::max() && state.c != 0) && iter < maxIters)
        {
            auto [iter_success, state_out] = sorting::restfulQuickSort(state);
            BOOST_CHECK(iter_success);
            state = state_out;
            if (state.l == sorting::LEFT_I)
            {
                state.c = updateComparator(values[state.arr[state.i]], values[state.arr[state.p]]);
            }
            else if (state.l == sorting::LEFT_J)
            {
                state.c = updateComparator(values[state.arr[state.j]], values[state.arr[state.p]]);
            }
            iter++;
        }

        // // ----
        // std::cout << iter << std::endl;
        // for (auto e : state.arr)
        // {
        //     std::cout << e << " ";
        // }
        // std::cout << std::endl;
        // // ----

        BOOST_CHECK_EQUAL(state.arr[0], 2);
        BOOST_CHECK_EQUAL(state.arr[1], 3);
        BOOST_CHECK_EQUAL(state.arr[2], 0);
        BOOST_CHECK_EQUAL(state.arr[3], 4);
        BOOST_CHECK_EQUAL(state.arr[4], 5);
        BOOST_CHECK_EQUAL(state.arr[5], 1);

        sorting::QuickSortState state2;
        state2.n     = state.n;
        state2.arr   = state.arr;
        state2.stack = stack;
        state        = state2;
    }
}

BOOST_AUTO_TEST_SUITE_END()
