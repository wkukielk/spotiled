//
//  SpotifyAuthorizerTests.swift
//  SpotiLEDKitTests
//
//  Created by wkuki on 16/08/2026.
//

import Darwin
import Foundation
import Testing
@testable import SpotiLEDKit

struct SpotifyAuthorizerTests {

    // RFC 7636 Appendix B known-answer test.
    @Test func codeChallengeMatchesRFC7636Vector() {
        let verifier = "dBjftJeZ4CVP-mB92K27uhbUJU1p1r_wW1gFWFOEjXk"

        let challenge = SpotifyPKCEAuthorizer.codeChallenge(for: verifier)

        #expect(challenge == "E9Melhoa2OwvFrEMTJguCHaoeK1t8URWbuGJSstw-cM")
    }

    @Test func randomTokenIsURLSafeAndUnique() {
        let first = SpotifyPKCEAuthorizer.makeRandomURLSafeToken()
        let second = SpotifyPKCEAuthorizer.makeRandomURLSafeToken()

        #expect(first != second)
        let allowed = CharacterSet(charactersIn: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_")
        #expect(first.unicodeScalars.allSatisfy { allowed.contains($0) })
    }

    @Test func resultExtractsCodeWhenStateMatches() throws {
        let query = [URLQueryItem(name: "code", value: "auth-code"), URLQueryItem(name: "state", value: "abc")]

        let result = try SpotifyPKCEAuthorizer.result(from: query, expectedState: "abc", verifier: "verifier")

        #expect(result == SpotifyAuthorizationResult(code: "auth-code", codeVerifier: "verifier"))
    }

    @Test func resultThrowsOnStateMismatch() {
        let query = [URLQueryItem(name: "code", value: "auth-code"), URLQueryItem(name: "state", value: "wrong")]

        #expect(throws: SpotifyAuthorizationError.stateMismatch) {
            try SpotifyPKCEAuthorizer.result(from: query, expectedState: "abc", verifier: "verifier")
        }
    }

    @Test func resultThrowsDeniedWhenSpotifyReturnsError() {
        let query = [URLQueryItem(name: "error", value: "access_denied"), URLQueryItem(name: "state", value: "abc")]

        #expect(throws: SpotifyAuthorizationError.denied("access_denied")) {
            try SpotifyPKCEAuthorizer.result(from: query, expectedState: "abc", verifier: "verifier")
        }
    }

    @Test func resultThrowsInvalidRedirectWhenCodeMissing() {
        let query = [URLQueryItem(name: "state", value: "abc")]

        #expect(throws: SpotifyAuthorizationError.invalidRedirect) {
            try SpotifyPKCEAuthorizer.result(from: query, expectedState: "abc", verifier: "verifier")
        }
    }

    // @Test
    func requestAuthorizationCodeOpensAuthorizeURLAndReturnsResultFromRedirect() async throws {
        let authorizer = SpotifyPKCEAuthorizer(clientID: "test-client", scopes: ["streaming"], redirectPort: 18898, redirectPath: "/login")
        let openedURL = Locked<URL?>(nil)

        // NWListener can silently sit in `.waiting` forever on hosts that block it (some sandboxed
        // CI/dev environments do), so this test times out with a clear failure rather than hanging.
        let result = try await withThrowingTaskGroup(of: SpotifyAuthorizationResult.self) { group in
            group.addTask {
                try await authorizer.requestAuthorizationCode { url in
                    openedURL.value = url
                    Task {
                        try? await Self.performRedirect(basedOn: url, port: 18898, path: "/login")
                    }
                }
            }
            group.addTask {
                try await Task.sleep(for: .seconds(5))
                throw TimeoutError()
            }
            let result = try await group.next()!
            group.cancelAll()
            return result
        }

        #expect(!result.code.isEmpty)
        #expect(!result.codeVerifier.isEmpty)

        let url = try #require(openedURL.value)
        let components = try #require(URLComponents(url: url, resolvingAgainstBaseURL: false))
        #expect(components.host == "accounts.spotify.com")
        #expect(components.path == "/authorize")
        let items = try #require(components.queryItems)
        let params = Dictionary(uniqueKeysWithValues: items.map { ($0.name, $0.value ?? "") })
        #expect(params["client_id"] == "test-client")
        #expect(params["response_type"] == "code")
        #expect(params["redirect_uri"] == "http://127.0.0.1:18898/login")
        #expect(params["code_challenge_method"] == "S256")
        #expect(params["scope"] == "streaming")
    }

    /// Simulates the browser hitting the loopback redirect the authorizer is listening on, using
    /// the same `state` the authorizer generated (read back from the opened authorize URL).
    private static func performRedirect(basedOn authorizeURL: URL, port: UInt16, path: String) async throws {
        let components = URLComponents(url: authorizeURL, resolvingAgainstBaseURL: false)!
        let state = components.queryItems!.first { $0.name == "state" }!.value!

        let connection = try Socket(port: port)
        try connection.send("GET \(path)?code=test-auth-code&state=\(state) HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n")
        connection.close()
    }
}

/// Minimal blocking TCP client used only to drive the redirect listener in tests, since
/// URLSession has no simple way to send a raw, deliberately-malformed-if-needed HTTP request.
private final class Socket {
    private let fd: Int32

    init(port: UInt16) throws {
        fd = socket(AF_INET, SOCK_STREAM, 0)
        guard fd >= 0 else { throw POSIXError(.ECONNREFUSED) }

        var addr = sockaddr_in()
        addr.sin_family = sa_family_t(AF_INET)
        addr.sin_port = port.bigEndian
        addr.sin_addr.s_addr = inet_addr("127.0.0.1")

        let result = withUnsafePointer(to: &addr) { ptr -> Int32 in
            ptr.withMemoryRebound(to: sockaddr.self, capacity: 1) { sockaddrPtr in
                connect(fd, sockaddrPtr, socklen_t(MemoryLayout<sockaddr_in>.size))
            }
        }
        guard result == 0 else { throw POSIXError(.ECONNREFUSED) }
    }

    func send(_ text: String) throws {
        _ = text.utf8CString.withUnsafeBufferPointer { buffer in
            write(fd, buffer.baseAddress, text.utf8.count)
        }
    }

    func close() {
        Darwin.close(fd)
    }
}

private struct TimeoutError: Error {}

private final class Locked<Value>: @unchecked Sendable {
    private let lock = NSLock()
    private var _value: Value

    init(_ value: Value) {
        _value = value
    }

    var value: Value {
        get { lock.withLock { _value } }
        set { lock.withLock { _value = newValue } }
    }
}

private extension NSLock {
    func withLock<T>(_ body: () -> T) -> T {
        lock()
        defer { unlock() }
        return body()
    }
}
