/* SHA256
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "SHA256.h"

namespace PokemonAutomation{


uint32_t byte_swap32(uint32_t x){
    x = ((x & 0x0000ffff) <<  16) | ((x & 0xffff0000) >> 16);
    x = ((x & 0x00ff00ff) <<   8) | ((x & 0xff00ff00) >>  8);
    return x;
}
uint32_t rotate_right32(uint32_t x, int bits){
    return (x >> bits) | (x << (32 - bits));
}


const uint32_t SHA256::ROUND_CONSTANTS[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};
void SHA256::reset(){
    m_bytes_loaded = 0;
    m_bytes_in_buffer = 0;
    m_hash[0] = 0x6a09e667;
    m_hash[1] = 0xbb67ae85;
    m_hash[2] = 0x3c6ef372;
    m_hash[3] = 0xa54ff53a;
    m_hash[4] = 0x510e527f;
    m_hash[5] = 0x9b05688c;
    m_hash[6] = 0x1f83d9ab;
    m_hash[7] = 0x5be0cd19;
}
void SHA256::push(const void* data, size_t bytes){
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
    for (; bytes > 0; bytes--){
        m_bytes_loaded++;
        m_buffer[m_bytes_in_buffer++] = *ptr++;
        if (m_bytes_in_buffer == 64){
            push_block();
            m_bytes_in_buffer = 0;
        }
    }
}
void SHA256::push_block(){
    uint32_t w[64];
    for (int i = 0; i < 16; i++){
        w[i] = byte_swap32(m_words[i]);
    }

    for (int i = 16; i < 64; i++){
        uint32_t s0 = rotate_right32(w[i - 15],  7) ^ rotate_right32(w[i - 15], 18) ^ (w[i - 15] >>  3);
        uint32_t s1 = rotate_right32(w[i -  2], 17) ^ rotate_right32(w[i -  2], 19) ^ (w[i -  2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a = m_hash[0];
    uint32_t b = m_hash[1];
    uint32_t c = m_hash[2];
    uint32_t d = m_hash[3];
    uint32_t e = m_hash[4];
    uint32_t f = m_hash[5];
    uint32_t g = m_hash[6];
    uint32_t h = m_hash[7];
    for (int i = 0; i < 64; i++){
        uint32_t s0 = rotate_right32(e,  6) ^ rotate_right32(e, 11) ^ rotate_right32(e, 25);
        uint32_t x0 = (e & f) ^ (~e & g);
        uint32_t t0 = h + s0 + x0 + ROUND_CONSTANTS[i] + w[i];
        uint32_t s1 = rotate_right32(a,  2) ^ rotate_right32(a, 13) ^ rotate_right32(a, 22);
        uint32_t x1 = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t1 = s1 + x1;
        h = g;
        g = f;
        f = e;
        e = d + t0;
        d = c;
        c = b;
        b = a;
        a = t0 + t1;
    }

    m_hash[0] += a;
    m_hash[1] += b;
    m_hash[2] += c;
    m_hash[3] += d;
    m_hash[4] += e;
    m_hash[5] += f;
    m_hash[6] += g;
    m_hash[7] += h;
}
void SHA256::finish(){
    m_buffer[m_bytes_in_buffer++] = 0x80;
    memset(m_buffer + m_bytes_in_buffer, 0, 64 - m_bytes_in_buffer);

    if (m_bytes_in_buffer > 56){
        push_block();
        memset(m_buffer, 0, 64);
    }

    uint64_t bits = m_bytes_loaded * 8;
    m_words[15] = byte_swap32((uint32_t)bits);
    m_words[14] = byte_swap32((uint32_t)(bits >> 32));
    push_block();
}


const void* SHA256::get_hash_raw() const{
    return m_hash;
}
void SHA256::get_hash_raw(void* raw_hash) const{
    memcpy(raw_hash, m_hash, sizeof(m_hash));
}
std::string SHA256::get_hash_hex() const{
    const char HEX_DIGITS[] = "0123456789abcdef";

    std::string out;
    for (int c = 0; c < 8; c++){
        char buf[8];
        uint32_t word = m_hash[c];
        buf[7] = HEX_DIGITS[(word >>  0) & 0xf];
        buf[6] = HEX_DIGITS[(word >>  4) & 0xf];
        buf[5] = HEX_DIGITS[(word >>  8) & 0xf];
        buf[4] = HEX_DIGITS[(word >> 12) & 0xf];
        buf[3] = HEX_DIGITS[(word >> 16) & 0xf];
        buf[2] = HEX_DIGITS[(word >> 20) & 0xf];
        buf[1] = HEX_DIGITS[(word >> 24) & 0xf];
        buf[0] = HEX_DIGITS[(word >> 28) & 0xf];
        out.append(buf, 8);
    }
    return out;
}





}
