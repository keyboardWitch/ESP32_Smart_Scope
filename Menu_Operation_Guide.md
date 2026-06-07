# ESP32 Smart Auxiliary Sighting System - Menu Operation Guide

## System Overview

This system is based on the ESP32-C3 development board and integrates an OLED display, IMU sensor, and laser rangefinder module to provide shooting enthusiasts with real-time environmental compensation, ballistic calculation, and auxiliary aiming capabilities. The system employs a multi-level menu architecture supporting configuration and management of 9 main functional modules.

## Startup and Mode Switching

### Boot Mode Selection
- **Main Display Mode**: Do **not press** the Enter button during startup to directly enter the aiming main interface
- **Menu Mode**: **Hold** the Enter button during startup to enter the main menu for configuration

### Button Operation Guide
| Button | Function |
|--------|----------|
| **UP** | Move cursor up in menus / Reload ammunition in main display |
| **DOWN** | Move cursor down in menus |
| **LEFT** | Decrease values / Return to parent menu |
| **RIGHT** | Increase values / Enter submenu or confirm selection |
| **ENTER** | Select mode during boot / Confirm selection in menus |

## Main Menu Interface
```
┌────────────────────────────────┐
│           MENU               │  ← Title bar (highlighted white)
├────────────────────────────────┤
│ Wind Compensation            │
│ Bullet Velocity              │
│ Crosshair Settings           │
│ Arc Points Settings          │
│ Sensor Calibration           │
│ Ammo Count                   │
│ Preset Manager               │
│ Range Finder                 │
│ Balistic Calib               │
│ Exit                         │  ← Currently selected item (highlighted background)
└────────────────────────────────┘
```
*Figure 1: Main menu displaying all 9 functional options and Exit option*

---

## Detailed Functional Module Descriptions

### 1. Wind Compensation
**Function**: Set current environmental wind speed and direction parameters for ballistic wind drift calculation

**Parameter Range**:
- Wind Speed: 0.0 - 20.0 m/s
- Wind Direction: 0° - 360° (0° indicates north wind, increasing clockwise)

**Operation Procedure**:
1. In the Wind Speed field, use LEFT/RIGHT to adjust wind speed value
2. Navigate to Wind Direction field, use LEFT/RIGHT to adjust wind direction angle  
3. Navigate to Save Settings, press RIGHT to save configuration to current preset

**Interface Wireframe**:
```
┌────────────────────────────────┐
│        WIND COMP             │
├────────────────────────────────┤
│ * Wind Speed: 5.0 m/s        │  ← Currently selected (with asterisk)
│   Wind Direction: 90°        │
│   Save Settings              │
└────────────────────────────────┘
```
*Figure 2: Wind compensation settings interface, currently selected field displays an asterisk*

**Display Effect**: Wind speed value and direction arrow displayed in top-left corner of main interface

---

### 2. Bullet Velocity
**Function**: Set the muzzle velocity of ammunition used, affecting ballistic drop calculation

**Parameter Range**:
- Velocity: 1 - 999 m/s (Typical values: BB pellets 150-300 m/s, steel BBs 300-400 m/s)

**Operation Procedure**:
1. In the Velocity field, use LEFT/RIGHT to adjust velocity value
2. Navigate to Save Settings, press RIGHT to save configuration

**Interface Wireframe**:
```
┌────────────────────────────────┐
│       BULLET VELOCITY        │
├────────────────────────────────┤
│ * Velocity: 250 m/s          │  ← Currently selected
│   Save Settings              │
└────────────────────────────────┘
```
*Figure 3: Bullet velocity settings interface*

**Note**: Velocity setting directly affects ballistic calculation accuracy; please set according to actual ammunition type

---

### 3. Crosshair Settings
**Function**: Select different aiming reticle styles

**Available Styles**:
- **Simple Crosshair**: Simple crosshair (default)
- **Dot**: Center dot
- **Dual Circle**: Dual concentric circles
- **Ring Dot**: Ring with center dot

**Operation Procedure**:
1. Use UP/DOWN to switch between four reticle styles
2. Navigate to Save Settings, press RIGHT to save selection

**Interface Wireframe**:
```
┌────────────────────────────────┐
│      CROSSHAIR SETTINGS      │
├────────────────────────────────┤
│   Simple Crosshair           │
│ * Dot                        │  ← Currently selected
│   Dual Circle                │
│   Ring Dot                   │
│   Save Settings              │
└────────────────────────────────┘
```
*Figure 4: Crosshair settings interface, currently selected style displays an asterisk*

**Display Effect**: Selected reticle style displayed at center of main interface

---

### 4. Arc Points Settings
**Function**: Control the display method of ballistic drop points

**Display Modes**:
- **Solid**: Constant display (default)
- **Blinking**: Blinking display (500ms interval)
- **Off**: Disable drop point display

**Operation Procedure**:
1. Use UP/DOWN to switch between three modes
2. Navigate to Save Settings, press RIGHT to save settings

**Interface Wireframe**:
```
┌────────────────────────────────┐
│     ARC POINTS SETTINGS      │
├────────────────────────────────┤
│   Solid                      │
│ * Blinking                   │  ← Currently selected
│   Off                        │
│   Save Settings              │
└────────────────────────────────┘
```
*Figure 5: Arc points settings interface*

**Application Scenarios**:
- Solid mode: Suitable for static targets
- Blinking mode: Improves visual recognition for dynamic targets
- Off mode: Reduces screen interference, focuses on basic aiming

---

### 5. Sensor Calibration
**Function**: Calibrate MPU6050 gyroscope and accelerometer to improve angle measurement accuracy

**Calibration Procedure**:
1. Select Gyroscope Calibration option
2. Press RIGHT to start calibration (device must remain completely stationary for approximately 2 seconds)
3. Automatically return to menu after calibration completes
4. Navigate to Save Settings to save calibration data to EEPROM

**Interface Wireframe**:
```
┌────────────────────────────────┐
│    SENSOR CALIBRATION        │
├────────────────────────────────┤
│ * Gyroscope Calibration      │  ← Currently selected
│   Save Settings              │
└────────────────────────────────┘
```
*Figure 6: Sensor calibration interface, displays progress during calibration*

**Important Notes**:
- Device must be placed on a completely level, vibration-free surface during calibration
- Calibration data is saved to the currently active preset
- To recalibrate, repeat the above steps to overwrite old data

---

### 6. Ammo Count
**Function**: Manage magazine capacity and enable shot detection functionality

**Configuration Options**:
- **Capacity**: Set magazine capacity (0-999 rounds)
- **Shot Detection**: Enable/disable automatic shot detection

**Shot Detection Principle**:
- Detects shooting vibration based on IMU accelerometer
- Automatically reduces ammunition count by 1 after detecting a shot
- Supports reloading: Press UP button in main interface to restore full ammunition

**Operation Procedure**:
1. Set magazine capacity in Capacity field
2. Toggle ON/OFF state in Shot Detection field
3. Navigate to Save Settings to save configuration

**Interface Wireframe**:
```
┌────────────────────────────────┐
│        AMMO COUNT            │
├────────────────────────────────┤
│ * Capacity: 30               │  ← Currently selected
│   Shot Detection: ON         │
│   Save Settings              │
└────────────────────────────────┘
```
*Figure 7: Ammunition count settings interface*

---

### 7. Preset Manager
**Function**: Manage 3 independent configuration presets for quick switching between different scenario settings

**Preset Features**:
- Each preset contains complete system configuration (wind speed, velocity, reticle, etc.)
- Preset data saved in EEPROM, retained after power loss
- Currently active preset takes effect in main interface

**Operation Procedure**:
1. Use UP/DOWN to select Preset 1/2/3
2. Press RIGHT to activate selected preset (system automatically loads all configurations for that preset)
3. Navigate to Save Settings to save current preset selection state

**Interface Wireframe**:
```
┌────────────────────────────────┐
│      PRESET MANAGER          │
├────────────────────────────────┤
│   Preset 1                   │
│ * Preset 2                   │  ← Currently active (with asterisk)
│   Preset 3                   │
│   Save Settings              │
└────────────────────────────────┘
```
*Figure 8: Preset manager interface, currently active preset displays an asterisk*

**Usage Recommendations**:
- Preset 1: Indoor short-range configuration
- Preset 2: Outdoor medium-range configuration  
- Preset 3: Special ammunition or environmental configuration

---

### 8. Range Finder
**Function**: Use TOF laser module to measure target distance

**Operating Modes**:
- **Single Mode**: Manually trigger single measurement (press GPIO 7 button)
- **Continuous Mode**: Automatic continuous measurement (updates every 500ms)

**Technical Specifications**:
- Measurement range: 0.1 - 20.0 meters
- Accuracy: ±1cm (short range)
- Filtering: 5-sample median filter + dynamic low-pass filter

**Operation Procedure**:
1. Select Single or Continuous mode
2. Navigate to Save Settings to save ranging mode
3. **Single mode**: Press GPIO 7 button in main interface to trigger measurement
4. **Continuous mode**: Real-time distance automatically displayed in main interface

**Interface Wireframe**:
```
┌────────────────────────────────┐
│       RANGE FINDER           │
├────────────────────────────────┤
│ * mode: single               │  ← Currently selected mode (with asterisk)
│   mode: continuous           │
│   save settings              │
└────────────────────────────────┘
```
*Figure 9: Ranging settings interface, currently selected mode displays an asterisk*

**Display Effect**: Distance value displayed in middle-right of main interface (e.g., "15.3m")

---

### 9. Ballistic Calib
**Function**: Calibrate ballistic model using real measurement data to improve long-range hit accuracy

**Calibration Points**:
- **5m point**: Record actual impact point angle at 5-meter distance
- **10m point**: Record actual impact point angle at 10-meter distance  
- **15m point**: Record actual impact point angle at 15-meter distance

**Calibration Procedure**:
1. Select Calibration option
2. Use UP/DOWN to select distance point to calibrate (5m/10m/15m)
3. Aim at target and ensure stability, press RIGHT to record current IMU angle
4. Repeat steps 2-3 to complete all required calibration points (minimum 3 points)
5. Navigate to Shoot Adjust for fine-tuning (optional)
6. Navigate to Save Settings to save calibration data

**Interface Wireframe**:
```
┌────────────────────────────────┐
│     BALLISTIC CALIB          │
├────────────────────────────────┤
│ * Calibration: 5m            │  ← Currently selected calibration point
│   Shoot Adjust               │
│   Save Settings              │
└────────────────────────────────┘
```
*Figure 10: Ballistic calibration interface, displays calibration status for each distance point*

**Calibration Requirements**:
- Must be performed in actual shooting environment
- Each distance point requires multiple verifications to ensure accuracy
- Calibration data bound to current preset and bullet velocity setting

---

### 10. Exit
**Function**: Exit menu system and return to main interface or confirm exit

**Operation Procedure**:
1. Select Exit option
2. System displays confirmation dialog (NO/YES)
3. Select NO to return to main menu, select YES to exit to main interface

**Interface Wireframe**:
```
┌────────────────────────────────┐
│           EXIT?              │
├────────────────────────────────┤
│ * NO                         │  ← Currently selected
│   YES                        │
└────────────────────────────────┘
```
*Figure 11: Exit confirmation dialog*

## Main Display Functions

### Main Display Layout
```
┌────────────────────────────────┐
│ 5.0m/s →                     │  ← Top-left: Wind speed + direction arrow
│                                │
│                                │
│              •                │  ← Center: Reticle style
│                                │
│                                │
│ 15/30        S               │  ← Bottom-left: Ammo count, bottom-right: Stability indicator
└────────────────────────────────┘
```
*Figure 12: Complete main display layout schematic (• represents reticle, S represents stability status)*

### Real-time Display Elements
- **Top-left corner**: Wind speed value + direction arrow
- **Screen center**: Selected reticle style
- **Middle-right**: Laser ranging distance (if enabled)
- **Ballistic drop point**: 3x3 pixel square showing predicted impact position
- **Bottom-left corner**: Ammunition count (current/total capacity)
- **Bottom-right corner**: Stability waveform indicator (S/M/U status)

### Main Display Interaction
- **UP button**: Reload ammunition to full capacity
- **GPIO 7 button**: Trigger laser ranging in Single mode
- **Automatic functions**: Continuous ranging, shot detection, stability calculation

## Optimization Suggestions

Based on the current menu structure, the following are potential optimization directions:

### User Experience Optimization
1. **Hotkey Support**: Add quick preset switching hotkeys to main interface
2. **Batch Save**: Add "Apply to All Presets" option in each functional module
3. **Default Reset**: Add factory reset functionality

### Feature Enhancement
1. **Multi-target Ranging**: Support storing multiple distance points for complex scenarios
2. **Environmental Temperature Compensation**: Add temperature sensor support for ballistic correction
3. **Battery Level Display**: Integrate battery monitoring functionality

### Interface Improvement
1. **Customizable Layout**: Allow users to show/hide specific information elements
2. **Color Themes**: Support inverted display (white background, black text) for different lighting conditions
3. **Font Size Adjustment**: Automatically adjust critical information display size based on distance

### Performance Optimization
1. **Intelligent Refresh**: Dynamically adjust UI refresh frequency based on stability
2. **Low Power Mode**: Reduce screen brightness or turn off backlight after extended inactivity
3. **Data Compression**: Optimize EEPROM storage format to extend flash memory lifespan

---
*Document Version: 1.0 | Last Updated: 2026-05-14*