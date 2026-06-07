# ESP32 Smart Auxiliary Sighting System

## Project Overview

This is an intelligent auxiliary sighting system based on the ESP32-C3 Super Mini development board, designed specifically for shooting enthusiasts. The system integrates an OLED display, MPU6050 6-axis sensor, laser rangefinder module, and wind compensation functionality, providing real-time environmental monitoring, ballistic calculation, and auxiliary aiming capabilities.

## Hardware Configuration

### Core Hardware
- **Main Controller**: ESP32-C3 Super Mini
- **Display**: SSD1306 OLED (128×64 pixels, I2C interface)
- **Motion Sensor**: MPU6050 6-axis gyroscope/accelerometer
- **Rangefinder Module**: TOF laser rangefinder (50m or 600m version selectable)

### Pin Connections
| Function | Pin | Description |
|----------|-----|-------------|
| **OLED SDA** | GPIO 8 | Shared with MPU6050 |
| **OLED SCL** | GPIO 9 | Shared with MPU6050 |
| **TOF RX** | GPIO 20 | 50m module receive |
| **TOF TX** | GPIO 21 | 50m module transmit |
| **UP Button** | GPIO 0 | Menu navigation/reload |
| **DOWN Button** | GPIO 1 | Menu navigation |
| **LEFT Button** | GPIO 2 | Decrease value/back |
| **RIGHT Button** | GPIO 3 | Increase value/confirm |
| **ENTER Button** | GPIO 4 | Boot mode selection/confirm |

> **Note**: Buttons use internal pull-up resistors, with the other end connected to ground

## System Features

### Main Feature Modules
1. **Wind Compensation** - Wind speed (0-20m/s) and direction (0-360°) settings
2. **Bullet Velocity** - Bullet velocity settings (1-500m/s)
3. **Crosshair Settings** - Four crosshair style selections
4. **Arc Points Settings** - Ballistic drop point display modes (Solid/Blinking/Off)
5. **Sensor Calibration** - MPU6050 sensor calibration
6. **Ammo Count** - Ammunition counting and automatic shot detection
7. **Preset Manager** - Management of 3 independent configuration presets
8. **Range Finder** - Laser ranging (single/continuous mode)
9. **Ballistic Calib** - Ballistic calibration (5m/10m/15m three-point calibration)
10. **Mount Angle** - Sight mounting angle offset adjustment

### Technical Features
- **Ballistic Calculation**: Supports both physical drag model and polynomial fitting algorithms
- **Real-time Compensation**: Automatically calculates wind drift, gravity drop, and tilt correction
- **Stability Detection**: IMU-based device stability assessment
- **Non-volatile Storage**: All configurations saved in EEPROM, retained after power loss
- **Multiple Presets**: 3 independent configuration presets for different scenarios
- **Intelligent Filtering**: Multi-level data filtering ensures measurement accuracy and display stability

## Operation Guide

### Boot Modes
- **Main Display Mode**: Power on without pressing Enter button, directly enter aiming interface
- **Menu Mode**: Hold Enter button during power-on to enter configuration menu

### Button Functions
| Button | Menu Mode | Main Display Mode |
|--------|-----------|-------------------|
| **UP** | Move cursor up | Reload ammunition to full capacity |
| **DOWN** | Move cursor down | - |
| **LEFT** | Decrease value/return to parent | - |
| **RIGHT** | Increase value/enter submenu | - |
| **ENTER** | Confirm selection | - |

### Main Display Interface
```
┌────────────────────────────────┐
│ 5.0m/s →                     │ ← Wind speed + direction arrow
│                                │
│                                │
│              •                │ ← Crosshair style
│                                │
│                                │
│ 15/30        S               │ ← Ammo count/stability
└────────────────────────────────┘
```

- **Top Left**: Wind speed value and direction indicator arrow
- **Screen Center**: Selected crosshair style
- **Middle Right**: Laser ranging distance (if enabled)
- **Ballistic Drop Point**: 3×3 pixel square indicating predicted impact position
- **Bottom Left**: Ammo count (current/total capacity)
- **Bottom Right**: Stability status indicator (S/M/U)

## Menu Structure

```
Main Menu
├── Wind Compensation          # Wind compensation settings
├── Bullet Velocity            # Bullet velocity settings
├── Crosshair Settings         # Crosshair style selection
├── Arc Points Settings        # Ballistic drop point display mode
├── Sensor Calibration         # Sensor calibration
├── Ammo Count                 # Ammunition counting management
├── Preset Manager             # Preset configuration management
├── Range Finder               # Laser ranging mode
├── Balistic Calib             # Ballistic three-point calibration
├── Mount Angle                # Sight mounting angle offset
└── Exit                       # Exit to main display
    ├── NO                     # Return to main menu
    └── YES                    # Confirm exit
```

## Ballistic Calibration Process

### Calibration Preparation
1. Select appropriate bullet velocity setting
2. Perform calibration in actual shooting environment
3. Ensure device stability and avoid vibration interference

### Three-Point Calibration Steps
1. Enter **Balistic Calib** menu
2. Select **Calibration** field
3. Use UP/DOWN to select calibration distance points (5m/10m/15m)
4. Aim at target and maintain stability, press RIGHT to record current IMU angle
5. Repeat steps 3-4 to complete all three calibration points
6. Navigate to **Save Settings** to save calibration data

### Model Selection
- **Physics Model**: Physics-based model using air resistance coefficient inversion
- **Polynomial Model**: Quadratic polynomial fitting based on three-point data

## Compilation and Deployment

### Dependencies
```ini
lib_deps = 
    adafruit/Adafruit SSD1306 @ ^2.5.0
    adafruit/Adafruit GFX Library @ ^1.11.0
    adafruit/Adafruit MPU6050 @ ^2.2.9
    adafruit/Adafruit Unified Sensor @ ^1.1.15
```

### Build Commands
```bash
# Compile project
pio run

# Upload firmware to development board
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200
```

### Development Environment Requirements
- **IDE**: VSCode + PlatformIO plugin
- **Platform**: PlatformIO Core ≥ 5.0
- **Framework**: Arduino for ESP32

## Configuration Parameters

All system parameters are centrally defined in `src/config.h`:

### Display Configuration
- `SCREEN_WIDTH/HEIGHT`: 128×64 pixels
- `DROP_POINT_PX_PER_RADIAN`: 12.0f (radian to pixel conversion coefficient)

### Sensor Configuration
- `MPU6050_SDA/SCL`: GPIO 8/9 (shared with OLED)
- `STABILITY_WINDOW_SIZE`: 8 (stability calculation window size)

### Ranging Configuration
- `TOF_ACTIVE`: 50 (currently using 50m module)
- `TOF_MIN/MAX_DISTANCE`: 0.1-50.0 meters

### Ballistic Parameters
- `BULLET_VELOCITY_DEFAULT`: 50 m/s (default velocity)
- `SIGHT_HEIGHT_MM`: 50 mm (sight height above bore)

## Troubleshooting

### Common Issues
1. **Display Not Lighting Up**
   - Check I2C wiring connections
   - Verify power supply voltage (3.3V or 5V)
   - Confirm I2C address is 0x3C

2. **Buttons Not Responding**
   - Check button wiring (one end to GPIO, one end to GND)
   - Confirm internal pull-up resistors are enabled
   - Test if buttons conduct properly

3. **Abnormal Ranging Data**
   - Ensure TOF module has sufficient power
   - Check UART wiring (RX/TX cross-connected)
   - Verify baud rate settings match

4. **IMU Data Drift**
   - Perform sensor calibration procedure
   - Ensure device is completely stationary during calibration
   - Check for strong magnetic field interference

### Debugging Tips
- Enable serial monitor to view detailed logs
- Use `Serial.println()` to output key variables
- Check if EEPROM data is saved correctly

## Project Architecture

### Code Structure
```
src/
├── main.cpp                   # Program entry point and main loop
├── system_init.cpp/h          # System initialization
├── menu_system.cpp/h          # Core menu engine
├── main_display.cpp/h         # Main display logic
├── button_handler.cpp/h       # Button debouncing handler
│
├── config.h                   # Global configuration parameters
│
├── ballistic_calc.cpp/h       # Ballistic calculation core
├── ballistic_calibration.cpp/h # Ballistic calibration module
├── wind_compensation.cpp/h    # Wind compensation module
├── range_finder.cpp/h         # Laser ranging module
├── sensor_calibration.cpp/h   # Sensor calibration module
├── preset_manager.cpp/h       # Preset management module
└── eeprom_manager.cpp/h       # EEPROM storage management
```

### Design Patterns
- **State Pattern**: Manages different operation modes (MENU/DISPLAY/CALIBRATION)
- **Composite Pattern**: Menu items and submenus form a tree structure
- **Callback Mechanism**: Decouples menu item behaviors
- **Singleton Concept**: Global state management

## License

This project is licensed under the MIT License, allowing free use, modification, and distribution.

## Documentation Reference

For detailed operation instructions, please refer to the [`Menu_Operation_Guide.md`](Menu_Operation_Guide.md) file.