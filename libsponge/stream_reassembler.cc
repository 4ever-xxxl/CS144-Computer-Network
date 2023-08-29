#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _buffer(capacity, '\0'), _bitmap(capacity, false), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (eof && _first_unassembled_idx + _capacity - _output.buffer_size() >= index + data.length()) {
        _eof = true;
    }
    size_t front_boundary = std::max(index, _first_unassembled_idx);
    size_t back_boundary = std::min(index + data.length(), _first_unassembled_idx + _capacity - _output.buffer_size());
    for (size_t i = front_boundary; i < back_boundary; i++) {
        if (_bitmap[i - _first_unassembled_idx]) {
            continue;
        }
        _buffer[i - _first_unassembled_idx] = data[i - index];
        _bitmap[i - _first_unassembled_idx] = true;
        _unassembled_bytes_num++;
    }
    std::string _str = "";
    while (_bitmap.front()) {
        _str += _buffer.front();
        _buffer.pop_front();
        _bitmap.pop_front();
        _buffer.push_back('\0');
        _bitmap.push_back(false);
    }
    _output.write(_str);
    _first_unassembled_idx += _str.length();
    _unassembled_bytes_num -= _str.length();
    if (_eof && empty()) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes_num; }

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
