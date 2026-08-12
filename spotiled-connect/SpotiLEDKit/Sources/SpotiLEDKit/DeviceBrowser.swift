//
//  ControllerBrowser.swift
//  SpotiLEDKit
//
//  Created by wkuki on 19/07/2026.
//

import Observation
import Network
import Mockable

public struct DiscoveredDevice: Equatable, Identifiable, Sendable {
    public var id: String { name }
    public let name: String
    public let endpoint: NWEndpoint

    public init(name: String, endpoint: NWEndpoint) {
        self.name = name
        self.endpoint = endpoint
    }
}

@Mockable
public protocol DeviceDiscovering {
    func start()
    func cancel()
    func onDevicesChanged(_ handler: @escaping ([DiscoveredDevice]) -> Void)
}


public final class BonjourServiceDiscovery: DeviceDiscovering {
    private var browser: NWBrowser?
    private var servicesChangedHandler: (([DiscoveredDevice]) -> Void)?
    private var serviceType: String;

    public init(serviceType: String) {
        self.serviceType = serviceType
    }


    public func start() {
        guard let _ = servicesChangedHandler else {
            assertionFailure("you must set servicesChangedHandler before start()")
            return
        }
        let parameters = NWParameters()
        parameters.includePeerToPeer = true
        browser = NWBrowser(for: .bonjour(type: serviceType, domain: nil), using: parameters)
        browser?.browseResultsChangedHandler = { [weak self] results, _ in
            self?.servicesChangedHandler?(Self.devices(from: results))
        }
        browser?.start(queue: .main)
    }

    public func cancel() {
        browser?.cancel()
    }

    public func onDevicesChanged(_ handler: @escaping ([DiscoveredDevice]) -> Void) {
        servicesChangedHandler = handler
    }

    static func devices(from results: some Sequence<NWBrowser.Result>) -> [DiscoveredDevice] {
        results.compactMap { result in
            guard case let .service(name, _, _, _) = result.endpoint else { return nil }
            return DiscoveredDevice(name: name, endpoint: result.endpoint)
        }.sorted { $0.name < $1.name }
    }
}

@Observable
public final class DeviceBrowser {
    public private(set) var discoveredDevices: [DiscoveredDevice] = []
    public var discoveredNames: [String] { discoveredDevices.map(\.name) }
    private let discovery: DeviceDiscovering

    public init(discovery: DeviceDiscovering = BonjourServiceDiscovery(serviceType: "_s-led-controler._tcp")) {
        self.discovery = discovery
        discovery.onDevicesChanged { [weak self] devices in
            self?.discoveredDevices = devices
        }
    }

    public func start() {
        discovery.start()
    }

    public func stop() {
        discovery.cancel()
    }
}
