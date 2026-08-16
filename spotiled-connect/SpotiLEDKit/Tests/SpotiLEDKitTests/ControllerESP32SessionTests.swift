//
//  ControllerESP32SessionTests.swift
//  SpotiLEDKitTests
//
//  Created by wkuki on 10/08/2026.
//

import Testing
import Mockable
@testable import SpotiLEDKit

struct ControllerESP32SessionTests {

    @Test func connectDelegatesToConnection() {
        let connection = MockEndpointConnecting(policy: .relaxed)
        let session = ControllerESP32Session(connection: connection)

        session.connect(to: .hostPort(host: "192.168.1.10", port: 80), name: "Controller")

        verify(connection).connect(to: .value(.hostPort(host: "192.168.1.10", port: 80)), name: .value("Controller"), path: .value("/")).called(.once)
    }

    @Test func disconnectDelegatesToConnectionAndResetsESP32State() async throws {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).send(.value("connect esp.local")).willReturn(#"{"ack":"ok"}"#)
        let session = ControllerESP32Session(connection: connection)
        try await session.connectESP32(hostname: "esp.local")
        #expect(session.esp32State == .connected(host: "esp.local"))

        session.disconnect()

        verify(connection).disconnect().called(.once)
        #expect(session.esp32State == .disconnected)
    }

    @Test func controllerStateForwardsUnderlyingConnection() {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).state.willReturn(.connected(name: "Living Room"))
        let session = ControllerESP32Session(connection: connection)

        #expect(session.controllerState == .connected(name: "Living Room"))
    }

    @Test func connectESP32SetsConnectingStateBeforeAwaitingReply() async throws {
        let connection = MockEndpointConnecting(policy: .relaxed)
        let session = ControllerESP32Session(connection: connection)
        var stateWhileSending: ESP32ConnectionState?
        given(connection).send(.any).willProduce { _ in
            stateWhileSending = session.esp32State
            return #"{"ack":"ok"}"#
        }

        try await session.connectESP32(hostname: "esp.local")

        #expect(stateWhileSending == .connecting(host: "esp.local"))
    }

    @Test func connectESP32SendsConnectCommandAndUpdatesStateOnAck() async throws {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).send(.any).willReturn(#"{"ack":"ok"}"#)
        let session = ControllerESP32Session(connection: connection)

        try await session.connectESP32(hostname: "esp.local")

        verify(connection).send(.value("connect esp.local")).called(.once)
        #expect(session.esp32State == .connected(host: "esp.local"))
    }

    @Test func connectESP32ThrowsAndSetsFailedStateOnControllerRejection() async {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).send(.any).willReturn(#"{"error":"busy"}"#)
        let session = ControllerESP32Session(connection: connection)

        await #expect(throws: ESP32SessionError.rejected("busy")) {
            try await session.connectESP32(hostname: "esp.local")
        }
        #expect(session.esp32State == .failed(host: "esp.local", message: "busy"))
    }

    @Test func connectESP32ThrowsAndSetsFailedStateOnTransportFailure() async {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).send(.any).willThrow(EndpointConnectionError.notConnected)
        let session = ControllerESP32Session(connection: connection)

        await #expect(throws: EndpointConnectionError.notConnected) {
            try await session.connectESP32(hostname: "esp.local")
        }
        #expect(session.esp32State == .failed(host: "esp.local", message: "notConnected"))
    }

    @Test func disconnectESP32SendsDisconnectCommandAndClearsStateOnAck() async throws {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).send(.value("connect esp.local")).willReturn(#"{"ack":"ok"}"#)
        given(connection).send(.value("disconnect")).willReturn(#"{"ack":"ok"}"#)
        let session = ControllerESP32Session(connection: connection)
        try await session.connectESP32(hostname: "esp.local")

        try await session.disconnectESP32()

        verify(connection).send(.value("disconnect")).called(.once)
        #expect(session.esp32State == .disconnected)
    }

    @Test func disconnectESP32ThrowsAndLeavesStateUnchangedOnControllerRejection() async throws {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).send(.value("connect esp.local")).willReturn(#"{"ack":"ok"}"#)
        given(connection).send(.value("disconnect")).willReturn(#"{"error":"no active session"}"#)
        let session = ControllerESP32Session(connection: connection)
        try await session.connectESP32(hostname: "esp.local")

        await #expect(throws: ESP32SessionError.rejected("no active session")) {
            try await session.disconnectESP32()
        }
        #expect(session.esp32State == .connected(host: "esp.local"))
    }

    @Test func authorizeSpotifySendsAuthorizeCommandWithCodeAndVerifier() async throws {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).send(.any).willReturn(#"{"ack":"ok"}"#)
        let session = ControllerESP32Session(connection: connection)

        try await session.authorizeSpotify(code: "auth-code", codeVerifier: "verifier")

        verify(connection).send(.value("authorize auth-code verifier")).called(.once)
    }

    @Test func authorizeSpotifyThrowsOnControllerRejection() async {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).send(.any).willReturn(#"{"error":"exchange failed"}"#)
        let session = ControllerESP32Session(connection: connection)

        await #expect(throws: ESP32SessionError.rejected("exchange failed")) {
            try await session.authorizeSpotify(code: "auth-code", codeVerifier: "verifier")
        }
    }
}
