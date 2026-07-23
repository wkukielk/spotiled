//
//  SelectControllerView.swift
//  spotiled-connect
//
//  Created by wkuki on 19/07/2026.
//

import SwiftUI
import Network
import Mockable

@Mockable
protocol ControllerDiscovering {
    func start()
    func cancel()
    func onNamesChanged(_ handler: @escaping ([String]) -> Void)
}

final class BonjourControllerDiscovery: ControllerDiscovering {
    private let browser: NWBrowser
    private var namesChangedHandler: (([String]) -> Void)?

    init(serviceType: String) {
        let parameters = NWParameters()
        parameters.includePeerToPeer = true
        browser = NWBrowser(for: .bonjour(type: serviceType, domain: nil), using: parameters)
        browser.browseResultsChangedHandler = { [weak self] results, _ in
            self?.namesChangedHandler?(Self.names(from: results.map(\.endpoint)))
        }
    }

    func start() {
        browser.start(queue: .main)
    }

    func cancel() {
        browser.cancel()
    }

    func onNamesChanged(_ handler: @escaping ([String]) -> Void) {
        namesChangedHandler = handler
    }

    static func names(from endpoints: some Sequence<NWEndpoint>) -> [String] {
        endpoints.compactMap { endpoint in
            if case let .service(name, _, _, _) = endpoint {
                return name
            }
            return nil
        }.sorted()
    }
}

@Observable
final class ControllerBrowser {
    private(set) var discoveredNames: [String] = []
    private let discovery: ControllerDiscovering

    init(discovery: ControllerDiscovering = BonjourControllerDiscovery(serviceType: "_ws._tcp")) {
        self.discovery = discovery
        discovery.onNamesChanged { [weak self] names in
            self?.discoveredNames = names
        }
    }

    func start() {
        discovery.start()
    }

    func stop() {
        discovery.cancel()
    }
}

struct SelectControllerView: View {
    @State private var browser = ControllerBrowser()

    var body: some View {
        List(browser.discoveredNames, id: \.self) { name in
            Text(name)
        }
        .overlay {
            if browser.discoveredNames.isEmpty {
                ContentUnavailableView("Searching for controllers…", systemImage: "wifi")
            }
        }
        .navigationTitle("Select controller")
        .task {
            browser.start()
        }
        .onDisappear {
            browser.stop()
        }
    }
}

#Preview {
    NavigationStack {
        SelectControllerView()
    }
}
