import WidgetKit
import SwiftUI
import Intents
import CoreLocation
import WeatherKit

// MARK: - Widget Configuration
struct WeatherComplicationWidget: Widget {
    let kind: String = "WeatherComplicationWidget"

    var body: some WidgetConfiguration {
        StaticConfiguration(kind: kind, provider: WeatherComplicationProvider()) { entry in
            WeatherComplicationEntryView(entry: entry)
                .containerBackground(.fill.tertiary, for: .widget)
        }
        .configurationDisplayName("Too Hot To Ride")
        .description("Shows riding conditions based on temperature, humidity, and wind.")
        .supportedFamilies([.accessoryCircular, .accessoryCorner, .accessoryInline])
    }
}

// MARK: - Timeline Provider
struct WeatherComplicationProvider: TimelineProvider {
    func placeholder(in context: Context) -> WeatherComplicationEntry {
        WeatherComplicationEntry(
            date: Date(),
            statusColor: .green,
            statusIcon: "checkmark",
            heatIndex: 125.0,
            statusText: "Good to ride!"
        )
    }

    func getSnapshot(in context: Context, completion: @escaping (WeatherComplicationEntry) -> ()) {
        let entry = WeatherComplicationEntry(
            date: Date(),
            statusColor: .green,
            statusIcon: "checkmark",
            heatIndex: 125.0,
            statusText: "Good to ride!"
        )
        completion(entry)
    }

    func getTimeline(in context: Context, completion: @escaping (Timeline<WeatherComplicationEntry>) -> ()) {
        Task {
            let entries = await generateEntries()
            let timeline = Timeline(entries: entries, policy: .atEnd)
            completion(timeline)
        }
    }
    
    private func generateEntries() async -> [WeatherComplicationEntry] {
        var entries: [WeatherComplicationEntry] = []
        let currentDate = Date()
        
        do {
            let location = try await getCurrentLocation()
            let weather = try await WeatherService.shared.weather(for: location)
            
            // Generate entries for the next 4 hours (updating every hour)
            for hourOffset in 0..<4 {
                let entryDate = Calendar.current.date(byAdding: .hour, value: hourOffset, to: currentDate)!
                let hourlyWeather = weather.hourlyForecast.first { forecast in
                    Calendar.current.isDate(forecast.date, equalTo: entryDate, toGranularity: .hour)
                } ?? weather.currentWeather
                
                let heatIndex = calculateHeatIndex(
                    temperature: hourlyWeather.temperature.value,
                    humidity: hourlyWeather.humidity,
                    windSpeed: hourlyWeather.wind.speed.value
                )
                
                let (color, icon, text) = getStatusInfo(for: heatIndex)
                
                let entry = WeatherComplicationEntry(
                    date: entryDate,
                    statusColor: color,
                    statusIcon: icon,
                    heatIndex: heatIndex,
                    statusText: text
                )
                entries.append(entry)
            }
        } catch {
            // Fallback entry if weather data is unavailable
            let entry = WeatherComplicationEntry(
                date: currentDate,
                statusColor: .gray,
                statusIcon: "exclamationmark.triangle",
                heatIndex: nil,
                statusText: "Weather unavailable"
            )
            entries.append(entry)
        }
        
        return entries
    }
    
    private func getCurrentLocation() async throws -> CLLocation {
        let locationManager = CLLocationManager()
        return try await withCheckedThrowingContinuation { continuation in
            let delegate = LocationDelegate { result in
                continuation.resume(with: result)
            }
            locationManager.delegate = delegate
            locationManager.requestWhenInUseAuthorization()
            locationManager.requestLocation()
        }
    }
    
    private func calculateHeatIndex(temperature: Double, humidity: Double, windSpeed: Double) -> Double {
        // Convert temperature from Celsius to Fahrenheit
        let temperatureFahrenheit = (temperature * 9/5) + 32
        
        // Convert humidity to percentage
        let humidityPercentage = humidity * 100
        
        // Convert wind speed from m/s to mph
        let windSpeedMph = windSpeed * 2.237
        
        // Apply the user's formula: temp + humidity - wind speed
        return temperatureFahrenheit + humidityPercentage - windSpeedMph
    }
    
    private func getStatusInfo(for heatIndex: Double) -> (Color, String, String) {
        if heatIndex < 130 {
            return (.green, "checkmark", "Good to ride!")
        } else if heatIndex < 170 {
            return (.yellow, "exclamationmark", "Caution advised")
        } else {
            return (.red, "xmark", "Too hot to ride!")
        }
    }
}

// MARK: - Location Delegate Helper
private class LocationDelegate: NSObject, CLLocationManagerDelegate {
    private let completion: (Result<CLLocation, Error>) -> Void
    
    init(completion: @escaping (Result<CLLocation, Error>) -> Void) {
        self.completion = completion
        super.init()
    }
    
    func locationManager(_ manager: CLLocationManager, didUpdateLocations locations: [CLLocation]) {
        guard let location = locations.first else {
            completion(.failure(LocationError.noLocation))
            return
        }
        completion(.success(location))
    }
    
    func locationManager(_ manager: CLLocationManager, didFailWithError error: Error) {
        completion(.failure(error))
    }
}

private enum LocationError: Error {
    case noLocation
}

// MARK: - Widget Bundle
@main
struct WeatherComplicationBundle: WidgetBundle {
    var body: some Widget {
        WeatherComplicationWidget()
    }
}