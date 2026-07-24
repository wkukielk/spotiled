//
//  ControllerBrowser.swift
//  SpotiLEDKit
//
//  Created by wkuki on 19/07/2026.
//

import Observation
import Network
import Mockable

@Mockable
public protocol ControllerDiscovering {
    func start()
    func cancel()
    func onNamesChanged(_ handler: @escaping ([String]) -> Void)
}

public final class BonjourControllerDiscovery: ControllerDiscovering {
    private var browser: NWBrowser?
    private var namesChangedHandler: (([String]) -> Void)?
    private var serviceType: String;

    public init(serviceType: String) {
        self.serviceType = serviceType
    }
    

    public func start() {
        let parameters = NWParameters()
        parameters.includePeerToPeer = true
        browser = NWBrowser(for: .bonjour(type: serviceType, domain: nil), using: parameters)
        browser?.browseResultsChangedHandler = { [weak self] results, _ in
            self?.namesChangedHandler?(Self.names(from: results.map(\.endpoint)))
        }
        browser?.start(queue: .main)
    }

    public func cancel() {
        browser?.cancel()
    }

    public func onNamesChanged(_ handler: @escaping ([String]) -> Void) {
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
public final class ControllerBrowser {
    public private(set) var discoveredNames: [String] = []
    private let discovery: ControllerDiscovering

    public init(discovery: ControllerDiscovering = BonjourControllerDiscovery(serviceType: "_ws._tcp")) {
        self.discovery = discovery
        discovery.onNamesChanged { [weak self] names in
            self?.discoveredNames = names
        }
    }

    public func start() {
        discovery.start()
    }

    public func stop() {
        discovery.cancel()
    }
}
