//
//  ControllerBrowser.swift
//  SpotiLEDKit
//
//  Created by wkuki on 19/07/2026.
//

import Observation
import Network
import Mockable

public struct DiscoveredController: Equatable, Identifiable, Sendable {
    public var id: String { name }
    public let name: String
    public let endpoint: NWEndpoint

    public init(name: String, endpoint: NWEndpoint) {
        self.name = name
        self.endpoint = endpoint
    }
}

@Mockable
public protocol ControllerDiscovering {
    func start()
    func cancel()
    func onControllersChanged(_ handler: @escaping ([DiscoveredController]) -> Void)
}

public final class BonjourControllerDiscovery: ControllerDiscovering {
    private var browser: NWBrowser?
    private var controllersChangedHandler: (([DiscoveredController]) -> Void)?
    private var serviceType: String;

    public init(serviceType: String) {
        self.serviceType = serviceType
    }


    public func start() {
        let parameters = NWParameters()
        parameters.includePeerToPeer = true
        browser = NWBrowser(for: .bonjour(type: serviceType, domain: nil), using: parameters)
        browser?.browseResultsChangedHandler = { [weak self] results, _ in
            self?.controllersChangedHandler?(Self.controllers(from: results))
        }
        browser?.start(queue: .main)
    }

    public func cancel() {
        browser?.cancel()
    }

    public func onControllersChanged(_ handler: @escaping ([DiscoveredController]) -> Void) {
        controllersChangedHandler = handler
    }

    static func controllers(from results: some Sequence<NWBrowser.Result>) -> [DiscoveredController] {
        results.compactMap { result in
            guard case let .service(name, _, _, _) = result.endpoint else { return nil }
            return DiscoveredController(name: name, endpoint: result.endpoint)
        }.sorted { $0.name < $1.name }
    }
}

@Observable
public final class ControllerBrowser {
    public private(set) var discoveredControllers: [DiscoveredController] = []
    public var discoveredNames: [String] { discoveredControllers.map(\.name) }
    private let discovery: ControllerDiscovering

    public init(discovery: ControllerDiscovering = BonjourControllerDiscovery(serviceType: "_s-led-controler._tcp")) {
        self.discovery = discovery
        discovery.onControllersChanged { [weak self] controllers in
            self?.discoveredControllers = controllers
        }
    }

    public func start() {
        discovery.start()
    }

    public func stop() {
        discovery.cancel()
    }
}
