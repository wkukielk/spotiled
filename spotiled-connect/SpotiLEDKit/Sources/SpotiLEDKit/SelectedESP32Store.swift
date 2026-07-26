//
//  SelectedESP32Store.swift
//  SpotiLEDKit
//
//  Created by wkuki on 26/07/2026.
//

import Observation

@Observable
public final class SelectedESP32Store {
    public var device: DiscoveredController?

    public init(device: DiscoveredController? = nil) {
        self.device = device
    }
}
