#pragma once

#include <deque>

class circularBuffer {
public:
    struct Glyph {
        int row;
        int col;
        char character;
    };

private:
    std::deque<Glyph> buffer;
    int count;

public:
    circularBuffer(int size) : count(size) {}

    void push(int row, int col, char character) {
        if (buffer.size() >= count)
            buffer.pop_front();

        buffer.push_back({row, col, character});
    }

    int size() const {
        return buffer.size();
    }

    Glyph& operator[](int index) {
        return buffer[index];
    }
};