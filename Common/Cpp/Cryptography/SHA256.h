/* SHA256
 *
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_Cryptography_SHA256_H
#define PokemonAutomation_Cryptography_SHA256_H

#include <stdint.h>
#include <string>

namespace PokemonAutomation{



class SHA256{
    uint32_t m_hash[8];
    uint64_t m_bytes_loaded;
    uint32_t m_bytes_in_buffer;
    union{
        uint8_t m_buffer[64];
        uint32_t m_words[16];
    };

public:
    SHA256(){ reset(); }

    const void* get_hash_raw() const;
    void get_hash_raw(void* raw_hash) const;
    std::string get_hash_hex() const;

    void reset();
    void push(const void* data, size_t bytes);
    void finish();

private:
    static const uint32_t ROUND_CONSTANTS[64];
    void push_block();
};



}
#endif
