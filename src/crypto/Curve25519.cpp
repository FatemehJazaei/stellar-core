// Copyright 2015 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0
#include "crypto/Curve25519.h"
#include "crypto/BLAKE2.h"
#include "crypto/CryptoError.h"
#include "crypto/SHA.h"
#include "util/HashOfHash.h"
#include <Tracy.hpp>
#include <functional>
#include <sodium.h>

#ifdef MSAN_ENABLED
#include <sanitizer/msan_interface.h>
#endif
extern "C"
{
#include "dilithium.h"
}

namespace stellar
{

Dilithium2Secret
dilithium2RandomSecret()
{
    Dilithium2Secret out;
    randombytes_buf(out.key.data(), out.key.size());
#ifdef MSAN_ENABLED
    __msan_unpoison(out.key.data(), out.key.size());
#endif
    return out;
}

Dilithium2Public
dilithium2DerivePublic(Dilithium2Secret const& sec)
{
    ZoneScoped;
    Dilithium2Public out;
    uint8_t secretKey[pqcrystals_dilithium2_ref_SECRETKEYBYTES];
    if (pqcrystals_dilithium2_ref_seed(out.key.data(), secretKey,
                                       sec.key.data()) != 0)
    {
        throw CryptoError("Could not derive key (mult_base)");
    }
    return out;
}

void
clearDilithium2Keys(Dilithium2Public& localPublic,
                    Dilithium2Secret& localSecret)
{
    sodium_memzero(localPublic.key.data(), localPublic.key.size());
    sodium_memzero(localSecret.key.data(), localSecret.key.size());
}

HmacSha256Key
dilithium2DeriveSharedKey(Dilithium2Secret const& localSecret,
                          Dilithium2Public const& localPublic,
                          Dilithium2Public const& remotePublic, bool localFirst)
{
    ZoneScoped;
    auto const& publicA = localFirst ? localPublic : remotePublic;
    auto const& publicB = localFirst ? remotePublic : localPublic;

    unsigned char q[crypto_scalarmult_BYTES];
    if (crypto_scalarmult(q, localSecret.key.data(), remotePublic.key.data()) !=
        0)
    {
        throw CryptoError("Could not derive shared key (mult)");
    }
#ifdef MSAN_ENABLED
    __msan_unpoison(q, crypto_scalarmult_BYTES);
#endif
    std::vector<uint8_t> buf;
    buf.reserve(crypto_scalarmult_BYTES + publicA.key.size() +
                publicB.key.size());
    buf.insert(buf.end(), q, q + crypto_scalarmult_BYTES);
    buf.insert(buf.end(), publicA.key.begin(), publicA.key.end());
    buf.insert(buf.end(), publicB.key.begin(), publicB.key.end());
    return hkdfExtract(buf);
}

xdr::opaque_vec<>
dilithium2Decrypt(Dilithium2Secret const& localSecret,
                  Dilithium2Public const& localPublic,
                  ByteSlice const& encrypted)
{
    ZoneScoped;
    if (encrypted.size() < crypto_box_SEALBYTES)
    {
        throw CryptoError(
            "encrypted.size() is less than crypto_box_SEALBYTES!");
    }

    const uint64_t MESSAGE_LEN = encrypted.size() - crypto_box_SEALBYTES;
    xdr::opaque_vec<> decrypted(MESSAGE_LEN, 0);

    if (crypto_box_seal_open(decrypted.data(), encrypted.data(),
                             encrypted.size(), localPublic.key.data(),
                             localSecret.key.data()) != 0)
    {
        throw CryptoError("dilithium2Decrypt failed");
    }

    return decrypted;
}
}
