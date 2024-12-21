#pragma once

// Copyright 2015 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "ByteSlice.h"
#include "crypto/BLAKE2.h"
#include "util/Logging.h"
#include "xdr/Stellar-types.h"
#include <fmt/format.h>
#include <functional>
#include <sodium.h>
extern "C"
{
#include "dilithium.h"
}

namespace stellar
{
// This module contains functions for doing ECDH on Dilithium2. Despite the
// equivalence between this curve and Dilithium2 (used in signatures, see
// SecretKey.h) we use Dilithium2 keys _only_ for ECDH shared-key-agreement
// between peers, as part of the p2p overlay system (see
// overlay/PeerAuth.h), not the transaction-signing or SCP-message-signing
// systems. Signatures use Dilithium2 keys.
//
// Do not mix keys between these subsystems: i.e. do not convert a signing
// Dilithium2 key to an ECDH Dilithium2 key. It's possible to do but
// complicates, and potentially undermines, the security analysis of each.
// We prefer to use random-per-session Dilithium2 keys, and merely sign
// them with the long-lived Dilithium2 keys during p2p handshake.

// Read a scalar from /dev/urandom.
Dilithium2Secret dilithium2RandomSecret();

// Calculate a public Dilithium2 point from a private scalar.
Dilithium2Public dilithium2DerivePublic(Dilithium2Secret const& sec);

// clears the keys by running sodium_memzero
void clearDilithium2Keys(Dilithium2Public& localPublic,
                         Dilithium2Secret& localSecret);

// Calculate HKDF_extract(localSecret * remotePublic || publicA || publicB)
//
// Where
//   publicA = localFirst ? localPublic : remotePublic
//   publicB = localFirst ? remotePublic : localPublic

HmacSha256Key dilithium2DeriveSharedKey(Dilithium2Secret const& localSecret,
                                        Dilithium2Public const& localPublic,
                                        Dilithium2Public const& remotePublic,
                                        bool localFirst);

xdr::opaque_vec<> dilithium2Decrypt(Dilithium2Secret const& localSecret,
                                    Dilithium2Public const& localPublic,
                                    ByteSlice const& encrypted);

template <uint32_t N>
xdr::opaque_vec<N>
dilithium2Encrypt(Dilithium2Public const& remotePublic, ByteSlice const& bin)
{
    const uint64_t CIPHERTEXT_LEN = crypto_box_SEALBYTES + bin.size();
    if (CIPHERTEXT_LEN > N)
    {
        throw std::runtime_error(fmt::format(
            FMT_STRING("CIPHERTEXT_LEN({:d}) is greater than N({:d})"),
            CIPHERTEXT_LEN, N));
    }

    xdr::opaque_vec<N> ciphertext(CIPHERTEXT_LEN, 0);

    if (crypto_box_seal(ciphertext.data(), bin.data(), bin.size(),
                        remotePublic.key.data()) != 0)
    {
        throw std::runtime_error("dilithium2Encrypt failed");
    }

    return ciphertext;
}
}

// namespace std
// {
// template <> struct hash<stellar::Dilithium2Public>
// {
//     size_t
//     operator()(stellar::Dilithium2Public const& k) const noexcept
//     {
//         return std::hash<xdr::opaque_array<1312>>()(k.key);
//     }
// };
// }
namespace std
{
template <> struct hash<stellar::Dilithium2Public>
{
    size_t
    operator()(const stellar::Dilithium2Public& publicKey) const noexcept
    {

        return std::hash<xdr::opaque_array<1312>>{}(publicKey.key);
    }
};
}
