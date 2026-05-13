// _OVPNBridge.h — unified umbrella for all ObjC types used by TunnelKitOpenVPN Swift code.
// Merges CTunnelKitOpenVPNCore and CTunnelKitOpenVPNProtocol into one self-contained
// module that has zero dependencies beyond Foundation (no openssl-apple, no cross-module refs).
// The #ifndef guards in DataPath.h/XOR.h prevent duplicate enum definitions.

// ── CTunnelKitOpenVPNCore types (must be first to set the #ifndef guards) ──
#import "CompressionAlgorithmNative.h"
#import "CompressionFramingNative.h"
#import "XORMethodNative.h"
#import "Errors.h"

// ── CTunnelKitOpenVPNProtocol types ──
#import "CryptoMacros.h"
#import "PacketMacros.h"
#import "ReplayProtector.h"
#import "DataPath.h"
#import "DataPathCrypto.h"
#import "TLSBox.h"
#import "Crypto.h"
#import "CryptoAEAD.h"
#import "CryptoCBC.h"
#import "CryptoCTR.h"
#import "CryptoBox.h"
#import "MSS.h"
#import "PacketStream.h"
#import "ControlPacket.h"
#import "XOR.h"

