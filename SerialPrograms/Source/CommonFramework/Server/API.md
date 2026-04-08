# SerialPrograms API Documentation

> [!WARNING]
> This API is still under development and not all features are fully implemented.

[<img src="https://canary.discordapp.com/api/guilds/695809740428673034/widget.png?style=banner2">](https://discord.gg/cQ4gWxN)

## Configuration


| Setting | Description                                         |
| :--- |:----------------------------------------------------|
| **Enable API** | Enable the API in the Settings (Default: Disabled)  |
| **HTTP Port** | Configurable (Default: `8080`, Range: `1025-65535`) |
| **WebSocket Port** | Configurable (Default: `8081`, Range: `1025-65535`) |

> [!IMPORTANT]
> The HTTP Port and WebSocket Port must be different.

---

## HTTP

> [!TIP]
> Sending a POST request to either the global settings or a program's settings will not update the setting if the relevant panel is currently open in the desktop application.

### Connection

- **URL:** `http://localhost:{HTTP Port}`

---

### Settings

#### `GET /settings`

Returns the current global settings.

**Response Body:**
```json
{
  "API": {
    "ENABLE_API": false,
    "HTTP_PORT": 8080,
    "WS_PORT": 8081
  }
}
```

---

#### `POST /settings`

Updates the global settings. Only the provided fields will be updated.

**Request Body:**
```json
{
  "API": {
    "ENABLE_API": true
  }
}
```

---

### Programs

#### `GET /programs`

Returns a list of all program categories.

**Response Body:**
```json
[
  {
    "display_name": "Nintendo Switch",
    "slug": "nintendo-switch"
  }
]
```

---

#### `GET /programs/{category_slug}`

Returns a list of all programs within a specific category.

**Response Body:**
```json
[
  {
    "description": "Endlessly mash A.",
    "display_name": "Turbo A",
    "slug": "turbo-a"
  }
]
```

---

#### `GET /programs/{category_slug}/{program_slug}`

Returns details for a specific program.

**Response Body:**
```json
{                                                                                                                                                
    "category": "Nintendo Switch",
    "description": "Endlessly mash A.",
    "display_name": "Turbo A",
    "slug": "turbo-a"
}
```

---

#### `GET /programs/{category_slug}/{program_slug}/options`

Returns the current configuration options for a specific program.

**Response Body:**
```json
{                                                                                                                                                
    "GO_HOME_WHEN_DONE": false,
    "START_LOCATION": "in-game",
    "SwitchSetup": {...},
    "TIME_LIMIT": "0 s"
}
```

---

#### `POST /programs/{category_slug}/{program_slug}/options`

Updates the configuration options for a specific program. Only the provided fields will be updated.

**Request Body:**
```json
{
  "GO_HOME_WHEN_DONE": true
}
```

---

## WebSocket

### Connection

- **URL:** `ws://localhost:{WebSocket Port}`

---

### Binary Messages

The WebSocket server broadcasts real-time video frames from the capture card. These are sent as binary messages containing a JPEG-encoded byte array.

### Text Messages

> [!WARNING] 
> Not yet implemented.
> Logs will be sent as text messages.
