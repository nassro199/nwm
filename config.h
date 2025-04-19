#pragma once

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>

// Appearance
constexpr int BORDER_PX = 1;        // Border pixel of windows
constexpr int SNAP_PX = 32;         // Snap pixel
constexpr bool SHOW_BAR = true;     // Show status bar
constexpr bool TOP_BAR = true;      // Status bar at top
constexpr const char* FONT = "monospace:size=10";
constexpr int BAR_HEIGHT = 20;      // Status bar height

// Colors
constexpr const char* COLOR_BORDER_NORMAL = "#444444";
constexpr const char* COLOR_BORDER_SELECTED = "#005577";
constexpr const char* COLOR_BG_NORMAL = "#222222";
constexpr const char* COLOR_BG_SELECTED = "#005577";
constexpr const char* COLOR_FG_NORMAL = "#bbbbbb";
constexpr const char* COLOR_FG_SELECTED = "#eeeeee";

// Tags/Workspaces
constexpr const char* TAGS[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
constexpr int NUM_TAGS = sizeof(TAGS) / sizeof(TAGS[0]);

// Layout
constexpr float MASTER_FACTOR = 0.55;  // Factor of master area size [0.05..0.95]
constexpr int NUM_MASTER = 1;          // Number of clients in master area
constexpr bool RESPECT_SIZE_HINTS = true; // Respect size hints in tiled resizals

// Applications
constexpr const char* TERMINAL[] = { "x-terminal-emulator", nullptr };
constexpr const char* MENU_PROGRAM[] = { "dmenu_run", nullptr };

// Key definitions
#define MODKEY Mod1Mask  // Alt key
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      [](void* arg) { g_windowManager->viewTag(TAG); }, nullptr }, \
    { MODKEY|ControlMask,           KEY,      [](void* arg) { g_windowManager->toggleTag(TAG); }, nullptr }, \
    { MODKEY|ShiftMask,             KEY,      [](void* arg) { g_windowManager->tagWindow(g_windowManager->getFocusedWindow(), TAG); }, nullptr }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      [](void* arg) { g_windowManager->toggleWindowTag(g_windowManager->getFocusedWindow(), TAG); }, nullptr }

// Helper for spawning shell commands
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, nullptr } }

// Key binding structure
struct KeyBinding {
    unsigned int mod;
    KeySym keysym;
    void (*func)(void*);
    void* arg;
};

// Key bindings will be defined in nwm.cpp