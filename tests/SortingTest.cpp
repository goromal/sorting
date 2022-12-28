#include <boost/test/unit_test.hpp>
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
    state.l   = 2;
    state.r   = 3;
    state.c   = 1;
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
    BOOST_CHECK_EQUAL(state.r, state2.r);
    BOOST_CHECK_EQUAL(state.c, state2.c);
}

BOOST_AUTO_TEST_CASE(TestIncrementalSorting)
{
    // TODO
}

BOOST_AUTO_TEST_SUITE_END()