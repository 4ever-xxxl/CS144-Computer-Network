#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (!seg.header().syn && !_syn_flag) {
        return;
    }
    _syn_flag |= seg.header().syn;
    _fin_flag |= seg.header().fin;
    if (seg.header().syn) {
        _isn = seg.header().seqno;
    }
    _seqno = seg.header().seqno + seg.header().syn;
    _abs_seqno = unwrap(_seqno, _isn, _reassembler.ack_idx());
    _reassembler.push_substring(seg.payload().copy(), _abs_seqno - 1, seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    size_t _abs_ackno = _reassembler.ack_idx() + _syn_flag + _reassembler.input_ended();
    if (_abs_ackno > 0) {
        return wrap(_abs_ackno, _isn);
    } else {
        return std::nullopt;
    }
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
