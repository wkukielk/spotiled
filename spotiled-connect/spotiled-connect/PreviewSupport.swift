//
//  PreviewSupport.swift
//  spotiled-connect
//
//  Preview-only test doubles for ESP32SessionManaging and ControllerDiscovering.
//

#if DEBUG

import Network
import Observation
import SpotiLEDKit
import SpotiLEDProvisioning
import SwiftUI
import ESPProvision

@Observable
final class PreviewFailingConnection: ControllerSessionManaging {
    var controllerState: EndpointConnectionState = .disconnected
    var esp32State: ESP32ConnectionState = .disconnected
    private let failAfter: Duration

    init(failAfter: Duration = .seconds(0.6)) {
        self.failAfter = failAfter
    }

    func connect(to endpoint: NWEndpoint, name: String) {
        controllerState = .connecting(name: name)
        Task { @MainActor in
            try? await Task.sleep(for: failAfter)
            controllerState = .failed(name: name)
        }
    }

    func disconnect() {
        controllerState = .disconnected
    }

    func connectESP32(hostname: String) async throws {
        throw ESP32SessionError.rejected("preview failure")
    }

    func disconnectESP32() async throws {
        esp32State = .disconnected
    }

    func authorizeSpotify(code: String, codeVerifier: String) async throws {
        throw ESP32SessionError.rejected("preview failure")
    }

    func spotifyStatus() async throws -> String? {
        throw ESP32SessionError.rejected("preview failure")
    }
}

@Observable
final class PreviewControllerConnection: ControllerSessionManaging {
    private static let fallbackSpotifyUsername = "user"

    var controllerState: EndpointConnectionState
    var esp32State: ESP32ConnectionState = .disconnected
    var spotifyUsername: String?

    init(state: EndpointConnectionState, spotifyUsername: String? = nil) {
        self.controllerState = state
        self.spotifyUsername = spotifyUsername
    }

    func connect(to endpoint: NWEndpoint, name: String) {
        controllerState = .connecting(name: name)
    }

    func disconnect() {
        controllerState = .disconnected
    }

    func connectESP32(hostname: String) async throws {
        esp32State = .connected(host: hostname)
    }

    func disconnectESP32() async throws {
        esp32State = .disconnected
    }

    func authorizeSpotify(code: String, codeVerifier: String) async throws {
        spotifyUsername = spotifyUsername ?? Self.fallbackSpotifyUsername
    }

    func spotifyStatus() async throws -> String? {
        spotifyUsername
    }
}

final class PreviewControllerDiscovery: DeviceDiscovering {
    private let controllers: [DiscoveredDevice]
    private var handler: (([DiscoveredDevice]) -> Void)?

    init(controllers: [DiscoveredDevice]) {
        self.controllers = controllers
    }

    func start() {
        Task { @MainActor [weak self] in
            try? await Task.sleep(for: .seconds(0.6))
            guard let self else { return }
            self.handler?(self.controllers)
        }
    }

    func cancel() {}

    func onDevicesChanged(_ handler: @escaping ([DiscoveredDevice]) -> Void) {
        self.handler = handler
    }

    func onStateChanged(_ handler: @escaping (DeviceDiscoveryState) -> Void) {}
}

private final class NoOpESPBLEDeviceControlling: ESPBLEDeviceControlling {
    func connect(delegate: ESPDeviceConnectionDelegate?) async throws {}
    func provision(ssid: String, passPhrase: String) async throws {}
}

public extension BLEDevice {
    static func preview(id: String) -> BLEDevice {
        BLEDevice(id: id, controller: NoOpESPBLEDeviceControlling())
    }
}


@Observable
final class PreviewDeviceProvisioning: DeviceProvisioning {
    var state: ProvisioningState = .idle
    var devices: [BLEDevice] = []
    private let fakeDeviceNames: [String]
    private let shouldSucceed: Bool

    init(fakeDeviceNames: [String] = ["PROV_AABBCC"], shouldSucceed: Bool = true) {
        self.fakeDeviceNames = fakeDeviceNames
        self.shouldSucceed = shouldSucceed
    }

    func startScan() {
        state = .scanning
        Task { @MainActor in
            try? await Task.sleep(for: .seconds(0.6))
            devices = fakeDeviceNames.map { .preview(id: $0) }
            state = .idle
        }
    }

    func stopScan() {}

    func resetState() {
        state = .idle
    }

    func provision(device: BLEDevice, ssid: String, password: String) async throws {
        state = .connecting(device.id)
        try? await Task.sleep(for: .seconds(0.6))
        state = .provisioning(device.id)
        try? await Task.sleep(for: .seconds(0.6))
        if shouldSucceed {
            state = .success
        } else {
            state = .failed("preview failure")
            throw ProvisioningError.disconnected
        }
    }
}

extension PreviewDeviceProvisioning: DeviceScanning {}

struct PreviewSpotifyAuthorizer: SpotifyAuthorizing {
    private let result: Result<SpotifyAuthorizationResult, Error>

    static func success(code: String = "preview-code", codeVerifier: String = "preview-verifier") -> PreviewSpotifyAuthorizer {
        PreviewSpotifyAuthorizer(result: .success(SpotifyAuthorizationResult(code: code, codeVerifier: codeVerifier)))
    }

    static func failure(_ error: Error = SpotifyAuthorizationError.denied("preview failure")) -> PreviewSpotifyAuthorizer {
        PreviewSpotifyAuthorizer(result: .failure(error))
    }

    func requestAuthorizationCode(
        openURL: @escaping @Sendable (URL) -> Void
    ) async throws -> SpotifyAuthorizationResult {
        try? await Task.sleep(for: .seconds(0.6))
        return try result.get()
    }
}

struct SelectControllerDismissHarness: View {
    let controllers: [DiscoveredDevice]
    @State private var isSelectingController = true

    var body: some View {
        NavigationStack {
            Text("Root")
                .navigationDestination(isPresented: $isSelectingController) {
                    SelectControllerView(
                        browser: DeviceBrowser(discovery: PreviewControllerDiscovery(controllers: controllers))
                    )
                }
        }
    }
}

#endif
