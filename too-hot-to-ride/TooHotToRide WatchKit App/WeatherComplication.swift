import SwiftUI
import WidgetKit
import ClockKit
import CoreLocation
import WeatherKit

// MARK: - Weather Manager
@MainActor
class WeatherManager: NSObject, ObservableObject {
    @Published var currentScore: Double?
    @Published var statusColor: Color = .gray
    @Published var statusText: String = "Loading..."
    @Published var statusIcon: String = "questionmark"
    
    private let locationManager = CLLocationManager()
    private var currentLocation: CLLocation?
    
    override init() {
        super.init()
        locationManager.delegate = self
        locationManager.desiredAccuracy = kCLLocationAccuracyKilometer
    }
    
    func requestLocation() {
        locationManager.requestWhenInUseAuthorization()
        locationManager.requestLocation()
    }
    
    private func fetchWeather(for location: CLLocation) async {
        do {
            let weather = try await WeatherService.shared.weather(for: location)
            let current = weather.currentWeather
            
            // Calculate heat index using the formula: temperature + humidity - wind speed
            let temperature = current.temperature.value // In Celsius
            let humidity = current.humidity * 100 // Convert to percentage
            let windSpeed = current.wind.speed.value // In m/s, convert to more readable unit
            
            // Convert temperature to Fahrenheit for the calculation
            let temperatureFahrenheit = (temperature * 9/5) + 32
            
            // Apply the user's formula: temp + humidity - wind speed
            // Convert wind speed from m/s to mph for the calculation
            let windSpeedMph = windSpeed * 2.237
            let heatIndex = temperatureFahrenheit + humidity - windSpeedMph
            
            await MainActor.run {
                self.currentScore = heatIndex
                self.updateStatus(heatIndex: heatIndex)
            }
        } catch {
            await MainActor.run {
                self.statusText = "Weather unavailable"
                self.statusColor = .gray
                self.statusIcon = "exclamationmark.triangle"
            }
        }
    }
    
    private func updateStatus(heatIndex: Double) {
        if heatIndex < 130 {
            statusColor = .green
            statusText = "Good to ride!"
            statusIcon = "checkmark"
        } else if heatIndex < 170 {
            statusColor = .yellow
            statusText = "Caution advised"
            statusIcon = "exclamationmark"
        } else {
            statusColor = .red
            statusText = "Too hot to ride!"
            statusIcon = "xmark"
        }
    }
}

// MARK: - Location Manager Delegate
extension WeatherManager: CLLocationManagerDelegate {
    func locationManager(_ manager: CLLocationManager, didUpdateLocations locations: [CLLocation]) {
        guard let location = locations.first else { return }
        currentLocation = location
        
        Task {
            await fetchWeather(for: location)
        }
    }
    
    func locationManager(_ manager: CLLocationManager, didFailWithError error: Error) {
        DispatchQueue.main.async {
            self.statusText = "Location unavailable"
            self.statusColor = .gray
            self.statusIcon = "location.slash"
        }
    }
}

// MARK: - Complication Views
struct WeatherComplicationEntryView: View {
    let entry: WeatherComplicationEntry
    
    var body: some View {
        ZStack {
            Circle()
                .fill(entry.statusColor)
                .frame(width: 40, height: 40)
            
            Image(systemName: entry.statusIcon)
                .foregroundColor(.white)
                .font(.title3)
        }
    }
}

struct WeatherComplicationEntry: TimelineEntry {
    let date: Date
    let statusColor: Color
    let statusIcon: String
    let heatIndex: Double?
    let statusText: String
}