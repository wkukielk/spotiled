//
//  PreviewSupport.swift
//  spotiled-connect
//
//  Preview-only test doubles for ControllerConnecting and ControllerDiscovering.
//

#if DEBUG

import Network
import Observation
import SpotiLEDKit
import SwiftUI

@Observable
final class PreviewFailingConnection: ControllerConnecting {
    var state: ControllerConnectionState = .disconnected
    private let failAfter: Duration

    init(failAfter: Duration = .seconds(0.6)) {
        self.failAfter = failAfter
    }

    func connect(to endpoint: NWEndpoint, name: String) {
        state = .connecting(name: name)
        Task { @MainActor in
            try? await Task.sleep(for: failAfter)
            state = .failed(name: name)
        }
    }

    func disconnect() {
        state = .disconnected
    }

    func send(_ message: String) async throws -> String {
        throw ControllerConnectionError.notConnected
    }
}

@Observable
final class PreviewControllerConnection: ControllerConnecting {
    var state: ControllerConnectionState

    init(state: ControllerConnectionState) {
        self.state = state
    }

    func connect(to endpoint: NWEndpoint, name: String) {
        state = .connecting(name: name)
    }

    func disconnect() {
        state = .disconnected
    }

    func send(_ message: String) async throws -> String {
        throw ControllerConnectionError.notConnected
    }
}

final class PreviewControllerDiscovery: ControllerDiscovering {
    private let controllers: [DiscoveredController]
    private var handler: (([DiscoveredController]) -> Void)?

    init(controllers: [DiscoveredController]) {
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

    func onControllersChanged(_ handler: @escaping ([DiscoveredController]) -> Void) {
        self.handler = handler
    }
}

struct SelectControllerDismissHarness: View {
    let controllers: [DiscoveredController]
    @State private var isSelectingController = true

    var body: some View {
        NavigationStack {
            Text("Root")
                .navigationDestination(isPresented: $isSelectingController) {
                    SelectControllerView(
                        browser: ControllerBrowser(discovery: PreviewControllerDiscovery(controllers: controllers))
                    )
                }
        }
    }
}

#endif
