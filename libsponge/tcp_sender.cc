#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _rto(retx_timeout)
    , _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    size_t sending_space = _abs_ackno + (_receiver_window_size != 0 ? _receiver_window_size : 1) - _next_seqno;
    while (sending_space > 0 && !_fin_sent) {
        TCPSegment seg;
        seg.header().seqno = next_seqno();
        if (_next_seqno == 0) {
            seg.header().syn = true;
            sending_space--;
        }
        size_t read_size = min(sending_space, TCPConfig::MAX_PAYLOAD_SIZE);
        seg.payload() = stream_in().read(read_size);
        sending_space -= seg.payload().size();
        if (stream_in().eof() && sending_space > 0) {
            seg.header().fin = true;
            _fin_sent = true;
            sending_space--;
        }
        if (seg.length_in_sequence_space() == 0) {
            return;
        }
        segments_out().emplace(seg);
        if (!_time_running) {
            _time_running = true;
            _time_elapsed = 0;
        }
        _segments_outstanding.push(seg);
        _next_seqno += seg.length_in_sequence_space();
        _bytes_in_flight += seg.length_in_sequence_space();
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _abs_ackno = unwrap(ackno, _isn, _next_seqno);
    if (_abs_ackno > _next_seqno) {
        return;
    }
    _receiver_window_size = window_size;
    bool new_ack = false;
    while (!_segments_outstanding.empty()) {
        TCPSegment seg = _segments_outstanding.front();
        size_t len = seg.length_in_sequence_space();
        uint64_t seqno = unwrap(seg.header().seqno, _isn, _next_seqno);
        if (seqno + len > _abs_ackno) {
            break;
        }
        _segments_outstanding.pop();
        _bytes_in_flight -= len;
        new_ack = true;
    }
    if (new_ack) {
        _rto = _initial_retransmission_timeout;
        _time_elapsed = 0;
        _time_running = !_segments_outstanding.empty();
        _consecutive_retransmissions = 0;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_time_running) {
        return;
    }
    _time_elapsed += ms_since_last_tick;
    if (_time_elapsed >= _rto) {
        _segments_out.push(_segments_outstanding.front());
        if (_receiver_window_size > 0) {
            _consecutive_retransmissions++;
            _rto <<= 1;
        }
        _time_elapsed = 0;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    _segments_out.push(seg);
}
