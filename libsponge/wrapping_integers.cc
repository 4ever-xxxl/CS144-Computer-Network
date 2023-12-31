#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) { return isn + uint32_t(n); }

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t res = uint64_t(n - isn);
    uint64_t RING = 1ul << 32;
    if (res >= checkpoint) {
        uint64_t k = (res - checkpoint) / RING;
        uint64_t Mod = (res - checkpoint) % RING;
        if (2ul * Mod > RING && checkpoint >= Mod) {
            res -= (k + 1ul) * RING;
        } else {
            res -= k * RING;
        }
    } else {
        uint64_t k = (checkpoint - res) / RING;
        uint64_t Mod = (checkpoint - res) % RING;
        if (2ul * Mod > RING) {
            res += (k + 1ul) * RING;
        } else {
            res += k * RING;
        }
    }
    return res;
}
