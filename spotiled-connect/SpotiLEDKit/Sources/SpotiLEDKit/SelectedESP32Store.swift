//
//  SelectedESP32Store.swift
//  SpotiLEDKit
//
//  Created by wkuki on 26/07/2026.
//

import Observation

@Observable
public final class SelectedESP32Store {
    public var device: DiscoveredDevice?

    public init(device: DiscoveredDevice? = nil) {
        self.device = device
    }
}
