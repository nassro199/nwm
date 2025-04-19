#include "nwm.h"
#include "window.h"
#include "layout.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// Global instance
WindowManager* g_windowManager = nullptr;

// Event handler function pointers
typedef void (WindowManager::*EventHandler)(XEvent*);
static EventHandler eventHandlers[LASTEvent] = {
    [ButtonPress] = &WindowManager::handleButtonPress,
    [ClientMessage] = &WindowManager::handleClientMessage,
    [ConfigureRequest] = &WindowManager::handleConfigureRequest,
    [ConfigureNotify] = &WindowManager::handleConfigureNotify,
    [DestroyNotify] = &WindowManager::handleDestroyNotify,
    [EnterNotify] = &WindowManager::handleEnterNotify,
    [Expose] = &WindowManager::handleExpose,
    [FocusIn] = &WindowManager::handleFocusIn,
    [KeyPress] = &WindowManager::handleKeyPress,
    [MappingNotify] = &WindowManager::handleMappingNotify,
    [MapRequest] = &WindowManager::handleMapRequest,
    [MotionNotify] = &WindowManager::handleMotionNotify,
    [PropertyNotify] = &WindowManager::handlePropertyNotify,
    [UnmapNotify] = &WindowManager::handleUnmapNotify
};

// Key bindings
static KeyBinding keys[] = {
    { MODKEY, XK_p, [](void* arg) { spawn(MENU_PROGRAM); }, nullptr },
    { MODKEY|ShiftMask, XK_Return, [](void* arg) { spawn(TERMINAL); }, nullptr },
    { MODKEY, XK_b, [](void* arg) { g_windowManager->toggleStatusBar(); }, nullptr },
    { MODKEY, XK_j, [](void* arg) { /* Focus next window */ }, nullptr },
    { MODKEY, XK_k, [](void* arg) { /* Focus previous window */ }, nullptr },
    { MODKEY, XK_i, [](void* arg) { g_windowManager->increaseMasterCount(); }, nullptr },
    { MODKEY, XK_d, [](void* arg) { g_windowManager->decreaseMasterCount(); }, nullptr },
    { MODKEY, XK_h, [](void* arg) { g_windowManager->decreaseMasterSize(); }, nullptr },
    { MODKEY, XK_l, [](void* arg) { g_windowManager->increaseMasterSize(); }, nullptr },
    { MODKEY, XK_Return, [](void* arg) { /* Zoom */ }, nullptr },
    { MODKEY, XK_Tab, [](void* arg) { /* View previous tag */ }, nullptr },
    { MODKEY|ShiftMask, XK_c, [](void* arg) { /* Kill client */ }, nullptr },
    { MODKEY, XK_t, [](void* arg) { g_windowManager->setLayout(LayoutType::TILED); }, nullptr },
    { MODKEY, XK_f, [](void* arg) { g_windowManager->setLayout(LayoutType::FLOATING); }, nullptr },
    { MODKEY, XK_m, [](void* arg) { g_windowManager->setLayout(LayoutType::MONOCLE); }, nullptr },
    { MODKEY, XK_space, [](void* arg) { g_windowManager->toggleLayout(); }, nullptr },
    { MODKEY|ShiftMask, XK_space, [](void* arg) { /* Toggle floating */ }, nullptr },
    { MODKEY, XK_0, [](void* arg) { /* View all tags */ }, nullptr },
    { MODKEY|ShiftMask, XK_0, [](void* arg) { /* Tag with all tags */ }, nullptr },
    { MODKEY, XK_comma, [](void* arg) { /* Focus previous monitor */ }, nullptr },
    { MODKEY, XK_period, [](void* arg) { /* Focus next monitor */ }, nullptr },
    { MODKEY|ShiftMask, XK_comma, [](void* arg) { /* Send to previous monitor */ }, nullptr },
    { MODKEY|ShiftMask, XK_period, [](void* arg) { /* Send to next monitor */ }, nullptr },
    TAGKEYS(XK_1, 0),
    TAGKEYS(XK_2, 1),
    TAGKEYS(XK_3, 2),
    TAGKEYS(XK_4, 3),
    TAGKEYS(XK_5, 4),
    TAGKEYS(XK_6, 5),
    TAGKEYS(XK_7, 6),
    TAGKEYS(XK_8, 7),
    TAGKEYS(XK_9, 8),
    { MODKEY|ShiftMask, XK_q, quit, nullptr }
};

// Constructor
WindowManager::WindowManager() 
    : display(nullptr), root(0), screen(0), screenWidth(0), screenHeight(0),
      currentMonitor(0), focusedClient(nullptr), running(false) {
}

// Destructor
WindowManager::~WindowManager() {
    cleanup();
}

// Initialize the window manager
bool WindowManager::initialize() {
    // Open display
    display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "nwm: cannot open display" << std::endl;
        return false;
    }

    // Get screen and root window
    screen = DefaultScreen(display);
    root = RootWindow(display, screen);
    screenWidth = DisplayWidth(display, screen);
    screenHeight = DisplayHeight(display, screen);
    
    // Check if another window manager is running
    XSetErrorHandler([](Display*, XErrorEvent*) -> int { 
        std::cerr << "nwm: another window manager is already running" << std::endl;
        exit(1);
        return 0;
    });
    
    // Try to select SubstructureRedirectMask on root window
    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(display, False);
    
    // Set normal error handler
    XSetErrorHandler([](Display*, XErrorEvent*) -> int { return 0; });
    XSync(display, False);
    
    // Initialize atoms
    // TODO: Initialize atoms
    
    // Initialize cursors
    cursors[0] = XCreateFontCursor(display, XC_left_ptr);
    cursors[1] = XCreateFontCursor(display, XC_sizing);
    cursors[2] = XCreateFontCursor(display, XC_fleur);
    
    // Initialize colors
    // TODO: Initialize colors
    
    // Initialize layouts
    layouts.push_back(Layout("[]=" , tileLayout));
    layouts.push_back(Layout("><>", nullptr));
    layouts.push_back(Layout("[M]", monocleLayout));
    
    // Initialize monitors
    // TODO: Initialize monitors with Xinerama if available
    Monitor m;
    m.x = 0;
    m.y = 0;
    m.width = screenWidth;
    m.height = screenHeight;
    m.selectedTag = 0;
    m.previousTag = 0;
    m.currentLayout = LayoutType::TILED;
    m.previousLayout = LayoutType::TILED;
    m.mfact = MASTER_FACTOR;
    m.nmaster = NUM_MASTER;
    
    // Initialize tags
    for (int i = 0; i < NUM_TAGS; i++) {
        Tag tag;
        tag.name = TAGS[i];
        tag.visible = (i == 0);
        m.tags.push_back(tag);
    }
    
    monitors.push_back(m);
    
    // Create status bar
    // TODO: Create status bar
    
    // Grab keys
    grabKeys();
    
    // Grab buttons
    grabButtons();
    
    // Scan for existing windows
    XGrabServer(display);
    Window root_return, parent_return;
    Window* children;
    unsigned int nchildren;
    
    if (XQueryTree(display, root, &root_return, &parent_return, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; i++) {
            XWindowAttributes wa;
            if (XGetWindowAttributes(display, children[i], &wa) && 
                wa.override_redirect == False && wa.map_state == IsViewable) {
                manageClient(children[i], &wa);
            }
        }
        if (children) {
            XFree(children);
        }
    }
    
    XUngrabServer(display);
    
    return true;
}

// Main event loop
void WindowManager::run() {
    XEvent ev;
    running = true;
    
    while (running && !XNextEvent(display, &ev)) {
        if (eventHandlers[ev.type]) {
            (this->*eventHandlers[ev.type])(&ev);
        }
    }
}

// Clean up resources
void WindowManager::cleanup() {
    // TODO: Implement cleanup
    if (display) {
        XCloseDisplay(display);
        display = nullptr;
    }
}

// Manage a new client window
void WindowManager::manageClient(Window win, XWindowAttributes* wa) {
    // TODO: Implement client management
}

// Unmanage a client window
void WindowManager::unmanageClient(Client* c, bool destroyed) {
    // TODO: Implement client unmanagement
}

// Focus a client
void WindowManager::focusClient(Client* c) {
    // TODO: Implement client focusing
}

// Unfocus a client
void WindowManager::unfocusClient(Client* c, bool setfocus) {
    // TODO: Implement client unfocusing
}

// Kill a client
void WindowManager::killClient(Client* c) {
    // TODO: Implement client killing
}

// Toggle floating state of a client
void WindowManager::toggleFloating(Client* c) {
    // TODO: Implement floating toggle
}

// Move a client
void WindowManager::moveClient(Client* c, int x, int y) {
    // TODO: Implement client moving
}

// Resize a client
void WindowManager::resizeClient(Client* c, int width, int height) {
    // TODO: Implement client resizing
}

// Toggle fullscreen state of a client
void WindowManager::toggleFullscreen(Client* c) {
    // TODO: Implement fullscreen toggle
}

// View a tag
void WindowManager::viewTag(int tag) {
    // TODO: Implement tag viewing
}

// Toggle a tag
void WindowManager::toggleTag(int tag) {
    // TODO: Implement tag toggling
}

// Tag a client
void WindowManager::tagClient(Client* c, int tag) {
    // TODO: Implement client tagging
}

// Toggle a client's tag
void WindowManager::toggleClientTag(Client* c, int tag) {
    // TODO: Implement client tag toggling
}

// Set the layout
void WindowManager::setLayout(LayoutType layout) {
    // TODO: Implement layout setting
}

// Toggle between layouts
void WindowManager::toggleLayout() {
    // TODO: Implement layout toggling
}

// Arrange windows
void WindowManager::arrange(Monitor* m) {
    // TODO: Implement window arrangement
}

// Increase master count
void WindowManager::increaseMasterCount() {
    // TODO: Implement master count increase
}

// Decrease master count
void WindowManager::decreaseMasterCount() {
    // TODO: Implement master count decrease
}

// Increase master size
void WindowManager::increaseMasterSize() {
    // TODO: Implement master size increase
}

// Decrease master size
void WindowManager::decreaseMasterSize() {
    // TODO: Implement master size decrease
}

// Update status bar
void WindowManager::updateStatusBar() {
    // TODO: Implement status bar update
}

// Toggle status bar
void WindowManager::toggleStatusBar() {
    // TODO: Implement status bar toggle
}

// Get client by window
Client* WindowManager::getClientByWindow(Window win) {
    auto it = clients.find(win);
    if (it != clients.end()) {
        return it->second.get();
    }
    return nullptr;
}

// Get focused client
Client* WindowManager::getFocusedClient() {
    return focusedClient;
}

// Get current monitor
Monitor* WindowManager::getCurrentMonitor() {
    if (currentMonitor >= 0 && currentMonitor < static_cast<int>(monitors.size())) {
        return &monitors[currentMonitor];
    }
    return nullptr;
}

// Get monitor by coordinates
Monitor* WindowManager::getMonitorByCoord(int x, int y) {
    // TODO: Implement monitor finding by coordinates
    return &monitors[0];
}

// Handle events
void WindowManager::handleEvent(XEvent* ev) {
    if (eventHandlers[ev->type]) {
        (this->*eventHandlers[ev->type])(ev);
    }
}

// Event handlers
void WindowManager::handleButtonPress(XEvent* ev) {
    // TODO: Implement button press handling
}

void WindowManager::handleClientMessage(XEvent* ev) {
    // TODO: Implement client message handling
}

void WindowManager::handleConfigureRequest(XEvent* ev) {
    // TODO: Implement configure request handling
}

void WindowManager::handleConfigureNotify(XEvent* ev) {
    // TODO: Implement configure notify handling
}

void WindowManager::handleDestroyNotify(XEvent* ev) {
    // TODO: Implement destroy notify handling
}

void WindowManager::handleEnterNotify(XEvent* ev) {
    // TODO: Implement enter notify handling
}

void WindowManager::handleExpose(XEvent* ev) {
    // TODO: Implement expose handling
}

void WindowManager::handleFocusIn(XEvent* ev) {
    // TODO: Implement focus in handling
}

void WindowManager::handleKeyPress(XEvent* ev) {
    // TODO: Implement key press handling
}

void WindowManager::handleMappingNotify(XEvent* ev) {
    // TODO: Implement mapping notify handling
}

void WindowManager::handleMapRequest(XEvent* ev) {
    // TODO: Implement map request handling
}

void WindowManager::handleMotionNotify(XEvent* ev) {
    // TODO: Implement motion notify handling
}

void WindowManager::handlePropertyNotify(XEvent* ev) {
    // TODO: Implement property notify handling
}

void WindowManager::handleUnmapNotify(XEvent* ev) {
    // TODO: Implement unmap notify handling
}

// Utility functions
void spawn(const char** cmd) {
    if (fork() == 0) {
        if (g_windowManager && g_windowManager->display) {
            close(ConnectionNumber(g_windowManager->display));
        }
        
        setsid();
        
        execvp(cmd[0], const_cast<char* const*>(cmd));
        fprintf(stderr, "nwm: execvp %s failed: %s\n", cmd[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void quit(void* arg) {
    if (g_windowManager) {
        g_windowManager->running = false;
    }
}

void grabKeys() {
    // TODO: Implement key grabbing
}

void grabButtons() {
    // TODO: Implement button grabbing
}

void updateNumlockMask() {
    // TODO: Implement numlock mask update
}

void updateStatus() {
    // TODO: Implement status update
}

void drawBar(Monitor* m) {
    // TODO: Implement bar drawing
}

void drawBars() {
    // TODO: Implement bars drawing
}

// Layout functions
void tileLayout(Monitor* m) {
    // TODO: Implement tile layout
}

void monocleLayout(Monitor* m) {
    // TODO: Implement monocle layout
}

// Main function
int main(int argc, char* argv[]) {
    if (argc == 2 && !strcmp("-v", argv[1])) {
        std::cout << "nwm-1.0" << std::endl;
        return EXIT_SUCCESS;
    } else if (argc != 1) {
        std::cerr << "usage: nwm [-v]" << std::endl;
        return EXIT_FAILURE;
    }
    
    if (!setlocale(LC_CTYPE, "")) {
        std::cerr << "warning: no locale support" << std::endl;
    }
    
    if (!(g_windowManager = new WindowManager())) {
        std::cerr << "nwm: failed to create window manager" << std::endl;
        return EXIT_FAILURE;
    }
    
    if (!g_windowManager->initialize()) {
        delete g_windowManager;
        return EXIT_FAILURE;
    }
    
    g_windowManager->run();
    
    delete g_windowManager;
    
    return EXIT_SUCCESS;
}

