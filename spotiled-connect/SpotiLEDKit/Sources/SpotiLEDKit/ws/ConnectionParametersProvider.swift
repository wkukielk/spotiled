//
//  File.swift
//  SpotiLEDKit
//
//  Created by wkuki on 13/09/2026.
//

import Foundation
import Network
import CryptoKit

protocol ConnectionParametersProvider {
    func getParameters() -> NWParameters
}

public enum ControllerTLSPolicy {
    case insecure
    case pairing(onFingerprint: (Data) -> Void)
    case pinned(fingerprint: Data)
}

struct InsecureConnectionParametersProvider : ConnectionParametersProvider {
    func getParameters() -> NWParameters {
        .tcp
    }
}

struct PinnedConnectionParametersProvider : ConnectionParametersProvider {
    let pinned: Data
    let onMismatch: () -> Void
    
    init(pinned: Data, onMismatch: @escaping () -> Void) {
        self.pinned = pinned
        self.onMismatch = onMismatch
    }
    
    func getParameters() -> NWParameters {
        let tlsOptions = NWProtocolTLS.Options()
        configureVerifyBlock(tlsOptions, onMismatch)
        return NWParameters(tls: tlsOptions, tcp: .init())
    }
    
    private func configureVerifyBlock(_ tlsOptions: NWProtocolTLS.Options, _ onMismatch: @escaping () -> Void) {
        let secOptions = tlsOptions.securityProtocolOptions
        sec_protocol_options_set_verify_block(secOptions, { _, secTrustRef, complete in
            guard let fingerprint = get_cert_fingerprint(secTrustRef) else {
                complete(false);
                return
            }
            let matches = fingerprint == self.pinned
            if !matches { onMismatch() }
            complete(matches)
        }, DispatchQueue.main)
    }
}

struct PairingConnectionParametersProvider: ConnectionParametersProvider {
    let onFingerprint: (Data) -> Void

    init(onFingerprint: @escaping (Data) -> Void) {
        self.onFingerprint = onFingerprint
    }
    
    func getParameters() -> NWParameters {
        let tlsOptions = NWProtocolTLS.Options()
        self.configureVerifyBlock(tlsOptions)
        return NWParameters(tls: tlsOptions, tcp: .init())
    }
    
    private func configureVerifyBlock(_ tlsOptions: NWProtocolTLS.Options) {
        let secOptions = tlsOptions.securityProtocolOptions
        sec_protocol_options_set_verify_block(secOptions, { _, secTrustRef, complete in
            guard let fingerprint = get_cert_fingerprint(secTrustRef) else {
                complete(false);
                return
            }
            self.onFingerprint(fingerprint)
            complete(true)
        }, DispatchQueue.main)
    }
}

func get_cert_fingerprint(_ secTrustRef: sec_trust_t) -> Data? {
    let trust = sec_trust_copy_ref(secTrustRef).takeRetainedValue()
    guard let chain = SecTrustCopyCertificateChain(trust) as? [SecCertificate],
          let leaf = chain.first else {
        return nil
    }
    let der = SecCertificateCopyData(leaf) as Data
    return Data(SHA256.hash(data: der))
}

func addWSParams(_ parameters: NWParameters) -> NWParameters {
    let webSocketOptions = NWProtocolWebSocket.Options()
    webSocketOptions.autoReplyPing = true
    parameters.defaultProtocolStack.applicationProtocols.insert(webSocketOptions, at: 0)
    return parameters
}
