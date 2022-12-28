#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <utility>

namespace sorting
{

// State required for sporadic, RESTful client-server-type sorting.
struct QuickSortState
{
    // Number of elements in the sortable array
    uint32_t n;
    // Sortable array
    std::vector<uint32_t> arr;
    // Auxiliary stack for iterative quick sort
    std::vector<uint32_t> stack;
    // Index of the top of the stack
    uint32_t top;
    // Current parition pivot element
    uint32_t p;
    // Current partition leftmost element
    uint32_t i;
    // Current partition rightmost element
    uint32_t j;
    // Current left input to client comparator
    uint32_t l;
    // Current right input to client comparator
    uint32_t r;
    // Current output of the client comparator given (l, r)
    // 0: NOT COMPARED; 1: l > r; 2: r > l; 3: r = l
    uint32_t c;
};

inline bool validateState(const QuickSortState& state)
{
    if (state.n == 0 || state.arr.size() != state.n | state.stack.size() != state.n)
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
    f.write((char*)&state.r, sizeof(uint32_t));
    f.write((char*)&state.c, sizeof(uint32_t));

    f.close();

    return true;
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
    f.read((char*)&read, sizeof(uint32_t));
    if (!f.fail())
    {
        state.n = read;
    }
    else
    {
        return {false, state};
    }
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
    f.read((char*)&read, sizeof(uint32_t));
    if (!f.fail())
    {
        state.top = read;
    }
    else
    {
        return {false, state};
    }
    f.read((char*)&read, sizeof(uint32_t));
    if (!f.fail())
    {
        state.p = read;
    }
    else
    {
        return {false, state};
    }
    f.read((char*)&read, sizeof(uint32_t));
    if (!f.fail())
    {
        state.i = read;
    }
    else
    {
        return {false, state};
    }
    f.read((char*)&read, sizeof(uint32_t));
    if (!f.fail())
    {
        state.j = read;
    }
    else
    {
        return {false, state};
    }
    f.read((char*)&read, sizeof(uint32_t));
    if (!f.fail())
    {
        state.l = read;
    }
    else
    {
        return {false, state};
    }
    f.read((char*)&read, sizeof(uint32_t));
    if (!f.fail())
    {
        state.r = read;
    }
    else
    {
        return {false, state};
    }
    f.read((char*)&read, sizeof(uint32_t));
    if (!f.fail())
    {
        state.c = read;
    }
    else
    {
        return {false, state};
    }

    f.close();

    return {true, state};
}

// RESTful Randomized Quick Sort with a client-side comparator.
// All necessary state information is contained in the SortState
// input, and the updated sort state is reflected in the output.
// The input state should contain a comparator output value (unless
// it's the first iteration: top = 0) and the output state should
// contain updated comparator inputs (unless sorting is complete:
// top = 0). Reports success status.
inline std::pair<bool, QuickSortState> restfulRandomizedQuickSort(const QuickSortState& currentState)
{
    QuickSortState state = currentState;
    if (!validateState(state))
    {
        return {false, state};
    }
    if (state.top > 0 && state.c == 0)
    {
        return {false, state};
    }

    // TODO

    return {true, state};
}

} // namespace sorting
