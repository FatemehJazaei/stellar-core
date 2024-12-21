#pragma once

// Copyright 2018 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "overlay/Peer.h"
#include "xdr/Stellar-types.h"

namespace stellar
{
struct PeerSharedKeyId
{
    Dilithium2Public mECDHPublicKey;
    Peer::PeerRole mRole;

    friend bool operator==(PeerSharedKeyId const& x, PeerSharedKeyId const& y);
    friend bool operator!=(PeerSharedKeyId const& x, PeerSharedKeyId const& y);
};
}

namespace std
{
template <> struct hash<stellar::PeerSharedKeyId>
{
    size_t operator()(stellar::PeerSharedKeyId const& x) const noexcept;
};
}
