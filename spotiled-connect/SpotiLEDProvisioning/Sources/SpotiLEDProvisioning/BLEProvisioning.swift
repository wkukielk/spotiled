import ESPProvision
import Mockable
import Observation

@Mockable
public protocol ESPBLEDeviceControlling: AnyObject {
    func connect(delegate: ESPDeviceConnectionDelegate?) async throws
    func provision(ssid: String, passPhrase: String) async throws
}

extension ESPDevice: ESPBLEDeviceControlling {
    public func connect(delegate: ESPDeviceConnectionDelegate?) async throws {
        try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<Void, Error>) in
            connect(delegate: delegate) { status in
                switch status {
                case .connected:
                    continuation.resume()
                case .failedToConnect(let error):
                    continuation.resume(throwing: error)
                case .disconnected:
                    continuation.resume(throwing: ProvisioningError.disconnected)
                }
            }
        }
    }

    public func provision(ssid: String, passPhrase: String) async throws {
        try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<Void, Error>) in
            var didComplete = false
            provision(ssid: ssid, passPhrase: passPhrase, threadOperationalDataset: nil) { status in
                switch status {
                case .configApplied:
                    break
                case .success:
                    guard !didComplete else { return }
                    didComplete = true
                    continuation.resume()
                case .failure(let error):
                    guard !didComplete else { return }
                    didComplete = true
                    continuation.resume(throwing: error)
                }
            }
        }
    }
}

public struct BLEDevice: Identifiable, Equatable {
    public let id: String
    let controller: any ESPBLEDeviceControlling

    public init(id: String, controller: any ESPBLEDeviceControlling) {
        self.id = id
        self.controller = controller
    }

    public static func == (lhs: BLEDevice, rhs: BLEDevice) -> Bool {
        lhs.id == rhs.id
    }
}

public enum ProvisioningState: Equatable, Sendable {
    case idle
    case scanning
    case connecting(String)
    case provisioning(String)
    case success
    case failed(String)
}

public enum ProvisioningError: Error, Equatable {
    case disconnected
}

final class EmptyProofOfPossessionDelegate: ESPDeviceConnectionDelegate {
    func getProofOfPossesion(forDevice: ESPDevice, completionHandler: @escaping (String) -> Void) {
        completionHandler("")
    }

    func getUsername(forDevice: ESPDevice, completionHandler: @escaping (String?) -> Void) {
        completionHandler(nil)
    }
}

@Mockable
public protocol DeviceProvisioning: AnyObject, Observable {
    var state: ProvisioningState { get }
    var devices: [BLEDevice] { get }
    func startScan()
    func stopScan()
    func provision(device: BLEDevice, ssid: String, password: String) async throws
}

@Observable
@MainActor
public final class ESPBLEProvisioning: DeviceProvisioning {
    private static let devicePrefix = "PROV_"

    public private(set) var state: ProvisioningState = .idle
    public private(set) var devices: [BLEDevice] = []

    public init() {}

    public func startScan() {
        state = .scanning
        devices = []
        ESPProvisionManager.shared.searchESPDevices(
            devicePrefix: Self.devicePrefix,
            transport: .ble,
            security: .secure
        ) { [weak self] espDevices, error in
            Task { @MainActor in
                guard let self else { return }
                if let error {
                    self.state = .failed("\(error)")
                    return
                }
                self.devices = (espDevices ?? []).map { BLEDevice(id: $0.name, controller: $0) }
                self.state = .idle
            }
        }
    }

    public func stopScan() {
        ESPProvisionManager.shared.stopScan()
    }

    public func provision(device: BLEDevice, ssid: String, password: String) async throws {
        state = .connecting(device.id)
        do {
            try await device.controller.connect(delegate: EmptyProofOfPossessionDelegate())
            state = .provisioning(device.id)
            try await device.controller.provision(ssid: ssid, passPhrase: password)
            state = .success
        } catch {
            state = .failed("\(error)")
            throw error
        }
    }
}
