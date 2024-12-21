// Copyright 2016 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "KeyUtils.h"

#include "crypto/StrKey.h"
extern "C"
{
#include "dilithium.h"
}
namespace stellar
{

size_t
KeyUtils::getKeyVersionSize(strKey::StrKeyVersionByte keyVersion)
{
    switch (keyVersion)
    {
    case strKey::STRKEY_PUBKEY_DILITHIUM2:
    case strKey::STRKEY_SEED_DILITHIUM2:
        return pqcrystals_dilithium2_ref_PUBLICKEYBYTES;
    case strKey::STRKEY_PRE_AUTH_TX:
    case strKey::STRKEY_HASH_X:
        return 32U;
    case strKey::STRKEY_SIGNED_PAYLOAD_DILITHIUM2:
        return 1376U; // 1312 bytes for the key and 64 bytes for the payload
    default:
        throw std::invalid_argument("invalid key version: " +
                                    std::to_string(keyVersion));
    }
}
}
