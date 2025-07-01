# ✈️ ADS-B Display User Guide

## 🎯 What is this program?

**ADS-B Display** is a **program that displays real-time aircraft positions on a map**.
- 📍 Track aircraft around the world in real-time
- ✈️ Display detailed aircraft information (flight number, altitude, speed, etc.)
- 🗺️ Various map backgrounds available
- 📊 Aircraft movement path tracking and analysis
- 🎮 Playback functionality for historical data

---

## 📑 Table of Contents
1. [Quick Start Guide](#quick-start-guide)
2. [Main Menu](#main-menu)
3. [Aircraft Information Display](#aircraft-information-display)
4. [Right Control Panel](#right-control-panel)
5. [Bottom Legend Panel](#bottom-legend-panel)
6. [Map Navigation](#map-navigation)
7. [Keyboard Shortcuts](#keyboard-shortcuts)
8. [Mouse Controls](#mouse-controls)
9. [Playback Features](#playback-features)
10. [Filter Features](#filter-features)
11. [Area Management](#area-management)

---

## 🚀 Quick Start Guide

### Step 1: Connect to Data Source
```
🔗 Easiest Connection Method
1. Find "SBS Connect" button in the right panel
2. Enter "data.adsbhub.org" in IP address field (default)
3. Click "SBS Connect" button
4. When button changes to "SBS Disconnect", connection successful! ✅
```

### Step 2: View Aircraft
- After connection, aircraft will appear on the map within seconds 🛩️
- Each aircraft is displayed as colored dots with directional arrows

### Step 3: Check Aircraft Information
- **Right-click** on any aircraft on the map to start tracking
- Detailed aircraft information will appear in the right panel

---

## 📁 Main Menu

### **File Menu**
| Menu Item | Function | Description |
|-----------|----------|-------------|
| **ADS-B Hub** | Remote ADS-B server connection | Connect to external ADS-B data sources |
| **ADS-B Local** | Local ADS-B connection | Connect to local network ADS-B receiver |
| **Load ARTCC Boundaries** | Load control area boundaries | Load Air Route Traffic Control Center boundary files |
| **Exit** | Exit program | Complete application shutdown |

### **Help Menu**
| Menu Item | Function |
|-----------|----------|
| **Documentation** | Open user guide documentation |
| **About** | Display program information |

---

## ✈️ Aircraft Information Display

### **Tracked Aircraft Information (Right Panel)**

#### **Basic Identification Information**
| Label | Display Content | Example |
|-------|----------------|---------|
| **ICAO** | Aircraft unique identifier | "A1B2C3" |
| **FLIGHT #** | Flight number | "UAL123" or "N/A" |

#### **Position Information**
| Label | Display Content | Format |
|-------|----------------|--------|
| **Latitude** | Current latitude | Degrees-minutes-seconds format |
| **Longitude** | Current longitude | Degrees-minutes-seconds format |
| **Altitude** | Current altitude | Feet units |

#### **Flight Information**
| Label | Display Content | Unit |
|-------|----------------|------|
| **SPD** | Speed + Vertical Rate | "450.00 KTS VRATE:1200.00" |
| **HDG** | Heading | "090.00 DEG" |

#### **Aircraft Detailed Information**
| Item | Content |
|------|---------|
| **Serial Number** | Manufacturing serial number |
| **Manufacturer** | Manufacturer |
| **Model** | Aircraft model |
| **MFR Year** | Manufacturing year |
| **Certificated Info** | Certification information |
| **Expiration Data** | Expiration date |
| **Engine Type** | Engine type |
| **Air Type** | Aircraft classification |

#### **Flight Route Information**
| Section | Included Information |
|---------|---------------------|
| **Departure** | Airport name, ICAO code, location, country |
| **Destination** | Airport name, ICAO code, location, country |
| **Transit 1,2** | Airport name, ICAO code, location, country |

### **Real-time Map Display**
| Display Element | Description |
|----------------|-------------|
| **Aircraft Symbol** | Dots color-coded by type |
| **Direction Arrow** | Aircraft heading direction |
| **ICAO Code** | Text display next to aircraft |
| **Predicted Path** | Predicted movement path when Time-To-Go is active |
| **Flight Track** | Past flight trajectory of aircraft (gray line) |

---

## 🎛️ Right Control Panel

### **Connection Control**
| Button | Status Display | Function |
|--------|----------------|----------|
| **Raw Connect** | "Raw Connect" ↔ "Raw Disconnect" | Connect/disconnect Raw data |
| **SBS Connect** | "SBS Connect" ↔ "SBS Disconnect" | Connect/disconnect SBS data |
| **Raw Record** | Start/Stop Recording | Save Raw data to file |
| **Raw Playback** | Start/Stop Playback | Play Raw file |
| **SBS Record** | Start/Stop Recording | Save SBS data to file |
| **SBS Playback** | Start/Stop Playback | Play SBS file |

### **Map Settings**
| Control | Options | Description |
|---------|---------|-------------|
| **MapComboBox** | • Google Maps<br>• VFR<br>• IFS Low<br>• IFS High<br>• OpenStreetMap | Select map type |
| **Live Map** | Checkbox | Real-time map updates |
| **BigQuery Upload** | Checkbox | BigQuery data upload |
| **Display Airport** | Checkbox | Show/hide airport information |

### **Control Buttons**
| Button | Function |
|--------|----------|
| **Zoom In** | Zoom in map |
| **Zoom Out** | Zoom out map |
| **Purge** | Clean up old aircraft data |

### **Time-To-Go Settings**
- **Time-To-Go Checkbox**: Enable aircraft predicted path display
- **Time-To-Go Slider**: Set predicted time in 1~1800 second range

---

## 🎨 Bottom Legend Panel

### **Aircraft Type Color Coding**
| Color | Type | Description |
|-------|------|-------------|
| 🔴 **Red** | Helicopter | Rotorcraft |
| 🟢 **Green** | Military | Military aircraft |
| 🟠 **Wine** | Civilian | Civilian aircraft registered in database |
| 🔵 **Blue** | Unknown | Aircraft with no type information |
| 🟡 **Yellow** | Near Airport | Aircraft located within airport radius |
| ⭕ **Red Circle** | Selected | User-selected aircraft |
| 🔲 **White** | Flight Path | Past flight trajectory of aircraft |

### **Altitude Color Coding (Time-To-Go Lines)**
| Color | Altitude Range | Description |
|-------|----------------|-------------|
| 🟥 **Red** | Below 1,000ft | Lowest altitude |
| 🟧 **Orange** | 1,000-3,000ft | Low altitude |
| 🟨 **Yellow** | 3,000-8,000ft | Medium-low altitude |
| 🟩 **Green** | 8,000-15,000ft | Medium altitude |
| 🟦 **Blue** | 15,000-25,000ft | Medium-high altitude |
| 🟪 **Purple** | 25,000-35,000ft | High altitude |
| 🟪 **Pink** | Above 35,000ft | Highest altitude |

### **Filter Controls**
| Control | Function |
|---------|----------|
| **Aircraft Type Filter** | All / Helicopters / Military / Known Civilian / Unknown |
| **Altitude Filter** | Filter aircraft by altitude ranges |
| **Speed Filter** | Filter aircraft by speed ranges |

---

## 🗺️ Map Navigation

### **Basic Controls**
| Control Method | Function | Detailed Description |
|----------------|----------|---------------------|
| **Mouse Wheel** | Zoom In/Out | Zoom centered on mouse pointer position |
| **Left Click + Drag** | Pan Map | Move map in desired direction |
| **Right Click** | Track Aircraft | Click aircraft to activate tracking mode |
| **Ctrl + Right Click** | CPA Tracking | Track Closest Point of Approach (CPA) |
| **Middle Mouse Button** | Reset Coordinates | Reset map offset |

### **Advanced Map Features**
- **Cell Click Zoom**: Click specific map cells for automatic zoom-in to that area
- **Real-time Coordinate Display**: Real-time display of latitude/longitude at cursor position

---

## ⌨️ Keyboard Shortcuts

### **Arrow Key Controls**
| Key | Function | Description |
|-----|----------|-------------|
| **↑ (Up Arrow)** | Move Map Up | Linked with vertical scrollbar |
| **↓ (Down Arrow)** | Move Map Down | Linked with vertical scrollbar |
| **← (Left Arrow)** | Move Map Left | Linked with horizontal scrollbar |
| **→ (Right Arrow)** | Move Map Right | Linked with horizontal scrollbar |

### **Page Keys**
| Key | Function | Description |
|-----|----------|-------------|
| **Page Up** | Large Movement | Large movement based on last arrow direction |
| **Page Down** | Large Movement | Large movement based on last arrow direction |

### **Special Keys**
| Key | Function | Description |
|-----|----------|-------------|
| **Home** | Go to Pittsburgh | Coordinates: 40.4406°N, 79.9959°W |
| **End** | Disabled | Currently no function |
| **F1** | Debug Information Output | Output current map center and viewpoint info to console |

---

## 🖱️ Mouse Controls

### **Mouse Button Functions**

#### **Left Button**
- **Click**: Cell-based zoom-in or start pan
- **Drag**: Pan map
- **Ctrl + Click**: Reserved (currently unused)

#### **Right Button**
- **Normal Click**: Aircraft tracking mode
- **Ctrl + Click**: CPA (Closest Point of Approach) tracking
- **During Area Mode**: Add points to polygon area

#### **Middle Button (Wheel Click)**
- **Click**: Reset map offset

---

## ⏯️ Playback Features

### **Playback Controls (Displayed only during file playback)**

#### **Speed Control Panel**
- **Location**: Top-right of screen
- **Speed Range**: x1 ~ x10
- **Function**: Real-time playback speed adjustment

#### **Progress Control Panel**
- **Location**: Bottom-right of screen
- **Functions**:
  - Display current playback time
  - Display total playback time
  - Jump to desired position via progress bar
  - Time seeking via drag

### **Features Applied Only in Playback Mode**
- **PurgeOldHistory**: Automatic deletion of aircraft paths not updated for 30 seconds
- **Time-based Data Management**: Time-based data management unlike real-time mode

---

## 🔍 Filter Features

### **Aircraft Type Filter**
```
Dropdown Menu: "All" (default)
├── All: Display all aircraft
├── Helicopters: Display helicopters only
├── Military: Display military aircraft only
├── Known Civilian: Display registered civilian aircraft only
└── Unknown: Display aircraft with unknown type only
```

### **Altitude Filter (AltitudeFilterComboBox)**
- **Range**: User-defined altitude range filtering
- **Display**: Consolas font, bold display

### **Speed Filter (SpeedFilterComboBox)**
- **Range**: User-defined speed range filtering
- **Display**: Consolas font, bold display

### **Area Filter**
- **Multi-select**: Ctrl+click to select multiple areas simultaneously
- **Single-select**: Normal click to select only that area
- **Clear**: ESC key to clear all selections
- **Apply**: Display only aircraft within selected areas

---

## 📍 Area Management

### **Area Creation**
1. Click **Insert** button → Activate insertion mode
2. **Right-click** on map to add points
3. **Complete** button to finish area
4. **Cancel** button to cancel

### **Area List Management**
| Operation | Function |
|-----------|----------|
| **Single Click** | Select only that area |
| **Ctrl + Click** | Add/remove area selection |
| **ESC Key** | Clear all selections |
| **Delete Button** | Delete selected areas |

### **Area Display**
- **Boundary**: Display in area color
- **Interior**: Semi-transparent color fill
- **Selected State**: Emphasized with dashed boundary

---

## 📊 Real-time Status Information

### **Screen Display Information**
| Location | Display Content |
|----------|----------------|
| **Bottom-left** | Latitude/longitude at cursor position |
| **Top-right** | Number of currently displayed aircraft |
| **Connection Status** | Raw/SBS connection status display |

### **Performance Information**
- **Update Cycle**: 500ms (Timer1)
- **Data Cleanup**: Every 5 seconds (Timer2)
- **Aircraft Timeout**: 30 seconds (real-time mode)
- **Path Timeout**: 30 seconds (playback mode only)

---

## 🔧 Troubleshooting

### **Common Issues**
| Problem | Solution |
|---------|----------|
| **Aircraft not displayed** | Check SBS/Raw connection status |
| **Map not loading** | Check internet connection and map type |
| **Playback stopped** | Check file format and path |
| **Filter not working** | Recheck filter settings and refresh data |

---

*📅 Last Updated: July 2025* 