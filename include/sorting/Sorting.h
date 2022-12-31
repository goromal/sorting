#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <utility>
#include <limits>
#include <cstdlib>

namespace sorting
{

// State required for sporadic, RESTful client-server-type sorting.
struct QuickSortState
{
    // Whether the array is sorted (1) or not (0)
    uint32_t sorted = 0;
    // Number of elements in the sortable array
    uint32_t n = 0;
    // Sortable array
    std::vector<uint32_t> arr;
    // Auxiliary stack for iterative quick sort
    std::vector<uint32_t> stack;
    // Index of the top of the stack
    uint32_t top = std::numeric_limits<uint32_t>::max();
    // Current parition pivot element
    uint32_t p;
    // Current partition leftmost element
    uint32_t i;
    // Current partition rightmost element
    uint32_t j;
    // Current left input to client comparator (right input is the pivot)
    // 0: i; 1: j
    uint32_t l = 0;
    // Current output of the client comparator given (l, r)
    // 0: NOT COMPARED; 1: l < p; 2: l > p; 3: l = p
    uint32_t c = 0;
};

// Enumerated types for interpreting certain QuickSortState fields.
enum class ComparatorLeft
{
    I = 0,
    J = 1
};
enum class ComparatorResult
{
    NOT_COMPARED = 0,
    LEFT_LESS    = 1,
    LEFT_GREATER = 2,
    LEFT_EQUAL   = 3
};
static constexpr uint32_t LEFT_I       = static_cast<uint32_t>(ComparatorLeft::I);
static constexpr uint32_t LEFT_J       = static_cast<uint32_t>(ComparatorLeft::J);
static constexpr uint32_t NOT_COMPARED = static_cast<uint32_t>(ComparatorResult::NOT_COMPARED);
static constexpr uint32_t LEFT_LESS    = static_cast<uint32_t>(ComparatorResult::LEFT_LESS);
static constexpr uint32_t LEFT_GREATER = static_cast<uint32_t>(ComparatorResult::LEFT_GREATER);
static constexpr uint32_t LEFT_EQUAL   = static_cast<uint32_t>(ComparatorResult::LEFT_EQUAL);

// Verify that the input state is formatted logically.
inline bool validateState(const QuickSortState& state)
{
    if (state.n == 0 || state.arr.size() != state.n | state.stack.size() != state.n)
    {
        return false;
    }
    if (!(state.l == LEFT_I || state.l == LEFT_J))
    {
        return false;
    }
    if (!(state.c == NOT_COMPARED || state.c == LEFT_LESS || state.c == LEFT_GREATER || state.c == LEFT_EQUAL))
    {
        return false;
    }
    return true;
}

// Persist the sorting state to disk. Reports success status.
inline bool persistStateToDisk(const std::string& filename, const QuickSortState& state)
{
    std::ofstream f(filename);
    if (!f)
    {
        return false;
    }
    if (!validateState(state))
    {
        return false;
    }

    f.write((char*)&state.sorted, sizeof(uint32_t));
    f.write((char*)&state.n, sizeof(uint32_t));
    for (size_t i = 0; i < state.arr.size(); i++)
    {
        f.write((char*)&state.arr[i], sizeof(uint32_t));
    }
    for (size_t i = 0; i < state.stack.size(); i++)
    {
        f.write((char*)&state.stack[i], sizeof(uint32_t));
    }
    f.write((char*)&state.top, sizeof(uint32_t));
    f.write((char*)&state.p, sizeof(uint32_t));
    f.write((char*)&state.i, sizeof(uint32_t));
    f.write((char*)&state.j, sizeof(uint32_t));
    f.write((char*)&state.l, sizeof(uint32_t));
    f.write((char*)&state.c, sizeof(uint32_t));

    f.close();

    return true;
}

#define _READ_STATE_FIELD(fieldName)                                                                                   \
    {                                                                                                                  \
        f.read((char*)&read, sizeof(uint32_t));                                                                        \
        if (!f.fail())                                                                                                 \
        {                                                                                                              \
            state.fieldName = read;                                                                                    \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            return {false, state};                                                                                     \
        }                                                                                                              \
    }

// Recover the sorting state from disk. Reports success status.
inline std::pair<bool, QuickSortState> sortStateFromDisk(const std::string& filename)
{
    QuickSortState state;
    std::ifstream  f(filename);
    if (!f)
    {
        return {false, state};
    }

    uint32_t read;
    _READ_STATE_FIELD(sorted)
    _READ_STATE_FIELD(n)
    if (state.n == 0)
    {
        return {false, state};
    }
    for (size_t i = 0; i < state.n; i++)
    {
        f.read((char*)&read, sizeof(uint32_t));
        if (!f.fail())
        {
            state.arr.push_back(read);
        }
        else
        {
            return {false, state};
        }
    }
    for (size_t i = 0; i < state.n; i++)
    {
        f.read((char*)&read, sizeof(uint32_t));
        if (!f.fail())
        {
            state.stack.push_back(read);
        }
        else
        {
            return {false, state};
        }
    }
    _READ_STATE_FIELD(top)
    _READ_STATE_FIELD(p)
    _READ_STATE_FIELD(i)
    _READ_STATE_FIELD(j)
    _READ_STATE_FIELD(l)
    _READ_STATE_FIELD(c)

    f.close();

    return {true, state};
}

// RESTful Randomized Quick Sort with a client-side comparator.
// All necessary state information is contained in the SortState
// input, and the updated sort state is reflected in the output.
// The input state should contain a comparator output value (unless
// it's the first iteration: top = UINT32_MAX && c = 0) and the output
// state should contain updated comparator inputs (unless sorting is
// complete: top = UINT32_MAX). Reports success status.
inline std::pair<bool, QuickSortState> restfulRandomizedQuickSort(const QuickSortState& currentState)
{
    QuickSortState state = currentState;

    // Reject invalid input states
    if (!validateState(state))
    {
        return {false, state};
    }
    if (state.top < std::numeric_limits<uint32_t>::max() && state.c == NOT_COMPARED)
    {
        return {false, state};
    }
    if (state.sorted == 1)
    {
        return {true, state};
    }

    // Randomized partition generator
    static auto randomizedPartition = [](const uint32_t& low, const uint32_t& high) {
        return low + rand() % (high - low);
    };

    // Element swapper
    static auto swap = [](std::vector<uint32_t>& arr, const uint32_t& i, const uint32_t& j) {
        uint32_t tmp = arr[i];
        arr[i]       = arr[j];
        arr[j]       = tmp;
    };

    // Partition reset from top of the stack
    static auto resetPartition = [](QuickSortState& s) {
        auto h = s.stack[s.top];
        auto l = s.stack[s.top - 1];

        auto l_swp = randomizedPartition(l, h);
        swap(s.arr, l, l_swp);

        s.p = l;
        s.i = l;
        s.j = h;

        s.l = LEFT_J;
        s.c = NOT_COMPARED;
    };

    // Algorithm initialization
    if (state.top == std::numeric_limits<uint32_t>::max() && state.c == NOT_COMPARED)
    {
        state.stack[++state.top] = 0;           // low
        state.stack[++state.top] = state.n - 1; // high
        resetPartition(state);
        return {true, state};
    }

    // Continue with partitioning
    if (state.l == LEFT_I)
    {
        if (state.c == LEFT_LESS && state.i < state.n - 1)
        {
            state.i++;
            state.c = NOT_COMPARED;
            return {true, state};
        }
        else
        {
            state.l = LEFT_J;
            state.j--;
            state.c = NOT_COMPARED;
            return {true, state};
        }
    }
    else
    {
        if (state.c == LEFT_GREATER)
        {
            state.j--;
            state.c = NOT_COMPARED;
            return {true, state};
        }
        else
        {
            if (state.i >= state.j)
            {
                auto p = state.j;
                auto h = state.stack[state.top--];
                auto l = state.stack[state.top--];
                if (p != 0 && p - 1 > l)
                {
                    state.stack[++state.top] = l;
                    state.stack[++state.top] = p - 1;
                }
                if (p + 1 < h)
                {
                    state.stack[++state.top] = p + 1;
                    state.stack[++state.top] = h;
                }
                if (state.top == std::numeric_limits<uint32_t>::max())
                {
                    state.sorted = 1; // sorting complete
                    return {true, state};
                }
                else
                {
                    resetPartition(state);
                    return {true, state};
                }
            }
            else
            {
                swap(state.arr, state.i, state.j);
                state.l = LEFT_I;
                if (state.i < state.n - 1)
                {
                    state.i++;
                }
                state.c = NOT_COMPARED;
                return {true, state};
            }
        }
    }
}

} // namespace sorting
