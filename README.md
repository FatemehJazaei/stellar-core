# Stellar Core — Dilithium2 Prototype

⚠️ **Disclaimer:** This is a research prototype.  
It modifies **Stellar Core** to use **CRYSTALS-Dilithium2** (post-quantum) digital signatures instead of the native Ed25519 scheme.  
**Not compatible with the public Stellar network. Do not use in production.**

---

## Overview
This fork adapts **Stellar Core**’s consensus and transaction validation to support **Dilithium2** signatures.  
Key changes include:
- New signature type `SIGNATURE_TYPE_DILITHIUM2` in XDR and Core logic.
- Transaction envelopes updated to handle larger signature blobs.
- Signing and verification paths replaced with calls to a Dilithium2 library.
- Adjustments in consensus validation to accept Dilithium2-based signatures.

---

## Key & Signature Sizes (Dilithium2 — approximate)
- **Public key:** ~1,312 bytes  
- **Private key:** ~2,528 bytes  
- **Signature:** ~2,420 bytes  

> By comparison, Ed25519 uses 32-byte public keys and 64-byte signatures.  
> This size difference affects **ledger capacity, bandwidth, storage, and performance**.
