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
import SwiftUI

@Observable
final class PreviewFailingConnection: ESP32SessionManaging {
    var controllerState: ControllerConnectionState = .disconnected
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
}

@Observable
final class PreviewControllerConnection: ESP32SessionManaging {
    var controllerState: ControllerConnectionState
    var esp32State: ESP32ConnectionState = .disconnected

    init(state: ControllerConnectionState) {
        self.controllerState = state
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
