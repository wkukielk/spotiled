//
//  SpotifyAuthorizer.swift
//  SpotiLEDKit
//
//  Created by wkuki on 16/08/2026.
//

import Foundation
import Network
import CryptoKit
import Mockable

public struct SpotifyAuthorizationResult: Sendable, Equatable {
    public let code: String
    public let codeVerifier: String

    public init(code: String, codeVerifier: String) {
        self.code = code
        self.codeVerifier = codeVerifier
    }
}

public enum SpotifyAuthorizationError: Error, Equatable {
    case listenerUnavailable
    case invalidRedirect
    case stateMismatch
    case denied(String)
}

@Mockable
public protocol SpotifyAuthorizing: Sendable {
    func requestAuthorizationCode(
        openURL: @escaping @Sendable (URL) -> Void
    ) async throws -> SpotifyAuthorizationResult
}

public struct SpotifyPKCEAuthorizer: SpotifyAuthorizing {
    public static let defaultClientID = "65b708073fc0480ea92a077233ca87bd"
    private static let authorizationEndpoint = "https://accounts.spotify.com/authorize"

    private let clientID: String
    private let scopes: [String]
    private let redirectPort: NWEndpoint.Port
    private let redirectPath: String

    public init(
        clientID: String = Self.defaultClientID,
        scopes: [String] = ["streaming"],
        redirectPort: UInt16 = 8898,
        redirectPath: String = "/login"
    ) {
        self.clientID = clientID
        self.scopes = scopes
        self.redirectPort = NWEndpoint.Port(rawValue: redirectPort) ?? 8898
        self.redirectPath = redirectPath
    }

    public func requestAuthorizationCode(
        openURL: @escaping @Sendable (URL) -> Void
    ) async throws -> SpotifyAuthorizationResult {
        let verifier = Self.makeRandomURLSafeToken()
        let challenge = Self.codeChallenge(for: verifier)
        let state = Self.makeRandomURLSafeToken()

        let listener = try SpotifyRedirectListener(port: redirectPort)
        listener.start(path: redirectPath)

        openURL(authorizationURL(challenge: challenge, state: state))

        let query = try await listener.awaitRedirect()
        return try Self.result(from: query, expectedState: state, verifier: verifier)
    }

    private var redirectURIString: String {
        "http://127.0.0.1:\(redirectPort.rawValue)\(redirectPath)"
    }

    private func authorizationURL(challenge: String, state: String) -> URL {
        var components = URLComponents(string: Self.authorizationEndpoint)!
        components.queryItems = [
            URLQueryItem(name: "client_id", value: clientID),
            URLQueryItem(name: "response_type", value: "code"),
            URLQueryItem(name: "redirect_uri", value: redirectURIString),
            URLQueryItem(name: "code_challenge_method", value: "S256"),
            URLQueryItem(name: "code_challenge", value: challenge),
            URLQueryItem(name: "state", value: state),
            URLQueryItem(name: "scope", value: scopes.joined(separator: " ")),
        ]
        guard let url = components.url else {
            preconditionFailure("hardcoded authorization endpoint with well-formed query items is always a valid URL")
        }
        return url
    }

    static func result(
        from query: [URLQueryItem],
        expectedState: String,
        verifier: String
    ) throws -> SpotifyAuthorizationResult {
        let params = Dictionary(query.map { ($0.name, $0.value ?? "") }, uniquingKeysWith: { first, _ in first })
        guard params["state"] == expectedState else {
            throw SpotifyAuthorizationError.stateMismatch
        }
        if let error = params["error"] {
            throw SpotifyAuthorizationError.denied(error)
        }
        guard let code = params["code"] else {
            throw SpotifyAuthorizationError.invalidRedirect
        }
        return SpotifyAuthorizationResult(code: code, codeVerifier: verifier)
    }

    static func makeRandomURLSafeToken(byteCount: Int = 32) -> String {
        var bytes = [UInt8](repeating: 0, count: byteCount)
        let status = SecRandomCopyBytes(kSecRandomDefault, bytes.count, &bytes)
        precondition(status == errSecSuccess, "SecRandomCopyBytes failed")
        return base64URLEncode(Data(bytes))
    }

    static func codeChallenge(for verifier: String) -> String {
        let digest = SHA256.hash(data: Data(verifier.utf8))
        return base64URLEncode(Data(digest))
    }

    private static func base64URLEncode(_ data: Data) -> String {
        data.base64EncodedString()
            .replacingOccurrences(of: "+", with: "-")
            .replacingOccurrences(of: "/", with: "_")
            .replacingOccurrences(of: "=", with: "")
    }
}

final class SpotifyRedirectListener: @unchecked Sendable {
    private static let queue = DispatchQueue(label: "SpotifyRedirectListener")

    private let listener: NWListener
    private let redirectResult = OneShotContinuation<[URLQueryItem]>()

    init(port: NWEndpoint.Port) throws {
        let parameters = NWParameters.tcp
        parameters.allowLocalEndpointReuse = true
        do {
            listener = try NWListener(using: parameters, on: port)
        } catch {
            throw SpotifyAuthorizationError.listenerUnavailable
        }
    }

    func start(path: String) {
        listener.stateUpdateHandler = { [weak self] state in
            if case .failed = state {
                self?.finish(.failure(SpotifyAuthorizationError.listenerUnavailable))
            }
        }
        listener.newConnectionHandler = { [weak self] connection in
            guard Self.isLoopback(connection.endpoint) else {
                connection.cancel()
                return
            }
            connection.start(queue: Self.queue)
            self?.handle(connection, expectedPath: path)
        }
        listener.start(queue: Self.queue)
    }

    private static func isLoopback(_ endpoint: NWEndpoint) -> Bool {
        guard case .hostPort(let host, _) = endpoint else { return false }
        switch host {
        case .ipv4(let address):
            return address == .loopback
        case .ipv6(let address):
            return address == .loopback
        case .name(let name, _):
            return name == "localhost" || name == "127.0.0.1"
        @unknown default:
            return false
        }
    }

    func awaitRedirect() async throws -> [URLQueryItem] {
        try await withTaskCancellationHandler {
            try await redirectResult.wait()
        } onCancel: {
            finish(.failure(CancellationError()))
        }
    }

    private func handle(_ connection: NWConnection, expectedPath: String) {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 8192) { [weak self] data, _, _, error in
            guard let self else { return }
            if let error {
                self.finish(.failure(error))
                return
            }
            guard let data, let text = String(data: data, encoding: .utf8) else { return }
            guard let requestLine = text.split(separator: "\r\n", maxSplits: 1).first else {
                self.finish(.failure(SpotifyAuthorizationError.invalidRedirect))
                return
            }
            Self.respond(on: connection)
            self.finish(Result { try Self.queryItems(fromRequestLine: String(requestLine), expectedPath: expectedPath) })
        }
    }

    private func finish(_ result: Result<[URLQueryItem], Error>) {
        guard redirectResult.finish(result) else { return }
        listener.newConnectionHandler = nil
        listener.stateUpdateHandler = nil
        listener.cancel()
    }

    private static func queryItems(fromRequestLine requestLine: String, expectedPath: String) throws -> [URLQueryItem] {
        let parts = requestLine.split(separator: " ")
        guard parts.count >= 2 else {
            throw SpotifyAuthorizationError.invalidRedirect
        }
        let url = String(parts[1])
        let components = URLComponents(string: url)
        guard let components, components.path == expectedPath else {
            throw SpotifyAuthorizationError.invalidRedirect
        }
        return components.queryItems ?? []
    }

    private static func respond(on connection: NWConnection) {
        let body = "<html><body><p>You can close this tab and return to SpotiLED Connect.</p></body></html>"
        let response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: \(body.utf8.count)\r\nConnection: close\r\n\r\n\(body)"
        connection.send(content: Data(response.utf8), completion: .contentProcessed { _ in
            connection.cancel()
        })
    }
}
