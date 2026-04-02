# Crow WebSocket Chat (Mustache + jQuery)

Simple real-time chat web app built with [Crow](https://github.com/CrowCpp/Crow) (C++) using **Mustache** templates for HTML rendering and **WebSockets** for instant message delivery.

## Task requirements

- **Framework**: Use Crow and Mustache templates.
- **UI screens**:
  - **Screen 1**: Enter your username
  - **Screen 2**: Main chat
- **Realtime**: Use **WebSockets** so messages are handled instantly.
- **Frontend**: Keep JavaScript minimal; **jQuery** is preferred over a larger framework.

## What’s implemented

- **Two-screen flow**
  - `GET /` renders the username entry screen (`templates/login.html`).
  - `POST /join` validates the username and redirects to `GET /chat?u=<username>`.
  - `GET /chat` renders the chat screen (`templates/chat.html`) and injects `{{username}}` via Mustache.
- **WebSocket chat**
  - WebSocket endpoint at `GET /ws?u=<username>`.
  - Each text message received from one client is broadcast to **all connected clients** instantly.
  - Messages are sent as JSON: `{"user":"<name>","text":"<message>"}`.
- **Input validation / constraints**
  - Username: **1–32 chars**, only `[A-Za-z0-9_]`.
  - Message: trimmed, **1–2000 chars**, **text frames only** (binary ignored).
  - Invalid/missing usernames are redirected back to `/`.
- **Minimal client-side JS**
  - Uses jQuery from CDN for DOM updates and form handling.
  - Uses the native `WebSocket` API; escapes user content before inserting into DOM.
- **Simple concurrency**
  - Server runs `multithreaded()`.
  - Connected websocket clients are tracked in-memory with a `std::mutex` for safety.

## Not implemented (by design / out of scope)

- **No persistence**: messages are not stored; refresh clears history.
- **No auth/session**: username travels via query string; this is intentional for simplicity.
- **No rooms**: single global chat.
- **No TLS**: SSL is disabled in `CMakeLists.txt` (`CROW_ENABLE_SSL OFF`).

## Project structure

- `src/main.cpp`: Crow app routes + websocket broadcast logic.
- `templates/login.html`: username entry page (Mustache template, static).
- `templates/chat.html`: chat page (Mustache template with `{{username}}`, minimal jQuery).
- `CMakeLists.txt`: uses CMake `FetchContent` to pull Crow and Asio; copies `templates/` to the build directory.

## Workflow (how to approach the task)

- **Start with routing + templates**
  - Render login via Mustache (`GET /`).
  - Validate username on the server (`POST /join`) and redirect into chat view.
- **Add WebSocket endpoint**
  - Reject connections without a valid username.
  - Store username in per-connection userdata.
  - Broadcast incoming messages to all connected clients.
- **Keep JS thin**
  - Connect to websocket, append incoming lines, send on submit.
  - Escape output to avoid HTML injection.

## Challenges / trade-offs encountered

- **Thread-safety**: `multithreaded()` means websocket callbacks can run concurrently, so the client set needs a mutex.
- **Lifecycle management**: connection userdata must be allocated on accept and freed on close to avoid leaks.
- **Validation duplication**: username is validated on `/join`, `/chat`, and `/ws` (each entry point must defend itself).
- **Simplicity vs. security**: username in query string and no auth is acceptable for this exercise but not for production.

## How to build and run (Ubuntu 22.04)

### 1) Install prerequisites

```bash
sudo apt update
sudo apt install -y git cmake g++ build-essential
```

This project fetches Crow + Asio during configure/build via CMake, so you don’t need to pre-install them.

### 2) Configure

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

### 3) Build

```bash
cmake --build build -j
```

### 4) Run

```bash
./build/chat_server
```

The server listens on **port 18080**.

Open in your browser:

- `http://localhost:18080/`

### 5) Use the app

- Enter a username (letters/digits/underscore).
- You’ll be redirected to the chat screen.
- Open multiple browser tabs (or multiple machines pointing to the same server) to see messages broadcast instantly.

## Notes / troubleshooting

- **Templates location**: `CMakeLists.txt` copies `templates/` into the build directory and the server is compiled with `CHAT_TEMPLATES_DIR` set to that location. If you run the binary outside `build/`, ensure the copied `build/templates/` still exists.
- **Firewall/remote access**: if testing from another machine, allow inbound TCP on port `18080` and browse to `http://<server-ip>:18080/`.

