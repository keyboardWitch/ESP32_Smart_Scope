#ifndef CONFIG_H
#define CONFIG_H

// ==================== OLED Display Configuration ====================
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_SDA        8
#define OLED_SCL        9
#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C

// ==================== MPU6050 Sensor Configuration ====================
#define MPU6050_SDA     8    // Shared SDA pin with OLED
#define MPU6050_SCL     9    // Shared SCL pin with OLED
#define MPU6050_ADDR    0x68 // Default I2C address for MPU6050

// ==================== ACTIVE TOF Module Configuration ====================
#define TOF_ACTIVE 600 // Currently used TOF module (50 or 600)
#if TOF_ACTIVE == 50
#define UART_RX         20   // ESP32C3 GPIO20 (RX)
#define UART_TX         21   // ESP32C3 GPIO21 (TX)
#define UART_BAUDRATE   115200 // Baud rate
#define TOF_MIN_DISTANCE 0.1f  // Minimum valid distance 0.1 meters
#define TOF_MAX_DISTANCE 50.0f // Maximum valid distance 50 meters
#define TOF_RAW_DIVISOR  10000.0f // Raw data divisor (e.g., 570.00 / 10000 = 0.057m)
#elif TOF_ACTIVE == 600
#define UART_RX         5   // ESP32C3 GPIO20 (RX)
#define UART_TX         6   // ESP32C3 GPIO21 (TX)
// #define TOF_POWER_ON    7   // ESP32C3 GPIO10 (Power On)
#define UART_BAUDRATE   9600 // Baud rate
#define TOF_MIN_DISTANCE 0.1f  // Minimum valid distance 0.1 meters
#define TOF_MAX_DISTANCE 600.0f // Maximum valid distance 600 meters

#endif
// ==================== 5-Way Button Pin Configuration ====================
#define BTN_UP          0
#define BTN_DOWN        1  
#define BTN_LEFT        2
#define BTN_RIGHT       3
#define BTN_ENTER       4

// ==================== Menu System Configuration ====================
#define MAX_MENU_ITEMS      10
#define MAX_SUBMENU_DEPTH   3
#define ITEMS_PER_PAGE      4

// ==================== Button Debounce Configuration ====================
#define DEBOUNCE_DELAY      50  // Debounce delay 50ms

// ==================== Ballistic Calibration Configuration ====================
#define MOUNT_ANGLE_OFFSET  0  // Sight mounting angle offset (in pixels), default is 0

// ==================== Main Display Configuration ====================
#define SIGHT_HEIGHT_MM           50     // Sight height above bore (millimeters)
#define DROP_POINT_PX_PER_MOA     0.25f  // Pixel/MOA conversion factor
#define DROP_POINT_PX_PER_RADIAN  12.0f  // Radian to pixel conversion factor (reduced from 100.0f to 12.0f to fit 64-pixel high screen)
#define DROP_POINT_BLINK_INTERVAL 500    // Drop point blink interval (ms)

// ==================== Filtering Coefficient Configuration ====================
#define DISTANCE_LPF_ALPHA        0.2f   // Distance filtering coefficient (reduced to 0.2 to match new smoothing strategy)
// #define COORD_LPF_ALPHA           0.2f   // Coordinate filtering coefficient (removed, using moving average filter instead)
#define IMU_LPF_ALPHA             0.1f   // IMU angle filtering coefficient (retained but no longer used)

// ==================== IMU Stability Calculation Configuration ====================
#define STABILITY_WINDOW_SIZE     8      // Stability calculation sliding window size
#define STABILITY_UPDATE_INTERVAL 200    // Stability update interval (ms)

// ==================== Shot Detection Configuration ====================
#define SHOT_DETECTION_COOLDOWN        2000  // Shot detection cooldown time (ms)
#define CONSECUTIVE_DETECTIONS_REQUIRED 3    // Required consecutive detections  
#define SHOT_DETECTION_WINDOW          500   // Shot detection window time (ms)
#define SHOT_DETECTION_THRESHOLD       4.0f  // Vibration threshold (m/s²)

// ==================== Update Interval Configuration ====================
#define MAIN_DISPLAY_UPDATE_INTERVAL   10    // Main display update interval (ms)
#define MEASUREMENT_INTERVAL          100   // Measurement interval (ms)

// ==================== Tilt Ballistic Correction Configuration ====================
#define TILT_CORRECTION_SCALE         1.0f  // Tilt correction magnitude scale factor, default 100% (1.0f = 100%)

// ==================== Bullet Velocity Configuration ====================
#define BULLET_VELOCITY_DEFAULT       50    // Default bullet velocity (m/s)
#define BULLET_VELOCITY_MIN           1     // Minimum bullet velocity (m/s)
#define BULLET_VELOCITY_MAX           500   // Maximum bullet velocity (m/s)

// ==================== MPU6050 Calibration Configuration ====================
#define MPU6050_CALIBRATION_SAMPLES   200   // Collect 200 samples
#define MPU6050_CALIBRATION_DELAY   10      // 10ms interval between each sample

// ==================== Default Wind Speed and Direction Configuration ====================
#define DEFAULT_WIND_SPEED   0.0f  // Default wind speed (m/s)
#define DEFAULT_WIND_DIRECTION 0.0f // Default wind direction (radians)

#endif // CONFIG_H