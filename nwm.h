#pragma once

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <X11/cursorfont.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "config.h"

// Forward declarations
class Client;
class Layout;

// Layout types
enum class LayoutType {
    TILED,
    FLOATING,
    MONOCLE
};

// Tag structure
struct Tag {
    std::string name;
    std::vector<Client*> clients;
    bool visible;
};

// Monitor structure
struct Monitor {
    int x, y, width, height;  // Monitor geometry
    std::vector<Tag> tags;
    int selectedTag;
    int previousTag;
    LayoutType currentLayout;
    LayoutType previousLayout;
    Window barwin;  // Status bar window
    float mfact;    // Master area factor
    int nmaster;    // Number of windows in master area
};

// Client (window) class
class Client {
public:
    Client(Window win);
    ~Client();

    Window window;
    std::string name;
    int x, y, width, height;
    int oldx, oldy, oldwidth, oldheight;
    int basew, baseh, incw, inch, maxw, maxh, minw, minh;
    int bw;  // Border width
    unsigned int tags;
    bool isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
    Monitor* mon;
};

// Layout class
class Layout {
public:
    Layout(const std::string& symbol, void (*arrange)(Monitor*));
    ~Layout();

    std::string symbol;
    void (*arrange)(Monitor*);
};

// WindowManager class
class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    // Initialization
    bool initialize();
    void run();
    void cleanup();

    // Window management
    void manageClient(Window win, XWindowAttributes* wa);
    void unmanageClient(Client* c, bool destroyed = false);
    void focusClient(Client* c);
    void unfocusClient(Client* c, bool setfocus = true);
    void killClient(Client* c);
    void toggleFloating(Client* c);
    void moveClient(Client* c, int x, int y);
    void resizeClient(Client* c, int width, int height);
    void toggleFullscreen(Client* c);

    // Tag management
    void viewTag(int tag);
    void toggleTag(int tag);
    void tagClient(Client* c, int tag);
    void toggleClientTag(Client* c, int tag);

    // Layout management
    void setLayout(LayoutType layout);
    void toggleLayout();
    void arrange(Monitor* m = nullptr);
    void increaseMasterCount();
    void decreaseMasterCount();
    void increaseMasterSize();
    void decreaseMasterSize();

    // Status bar
    void updateStatusBar();
    void toggleStatusBar();

    // Utility functions
    Client* getClientByWindow(Window win);
    Client* getFocusedClient();
    Monitor* getCurrentMonitor();
    Monitor* getMonitorByCoord(int x, int y);

    // Event handlers
    void handleEvent(XEvent* ev);

private:
    // X11 related
    Display* display;
    Window root;
    int screen;
    int screenWidth, screenHeight;
    Colormap cmap;
    XftColor colors[2][3];  // [SchemeNorm/SchemeSel][fg/bg/border]
    Cursor cursors[3];      // Normal, resize, move
    Atom wmatom[4];         // WM_PROTOCOLS, WM_DELETE_WINDOW, WM_STATE, WM_TAKE_FOCUS
    Atom netatom[10];       // _NET atoms

    // Window management
    std::vector<Monitor> monitors;
    int currentMonitor;
    std::unordered_map<Window, std::unique_ptr<Client>> clients;
    Client* focusedClient;
    std::vector<Layout> layouts;
    bool running;

    // Event handlers
    void handleButtonPress(XEvent* ev);
    void handleClientMessage(XEvent* ev);
    void handleConfigureRequest(XEvent* ev);
    void handleConfigureNotify(XEvent* ev);
    void handleDestroyNotify(XEvent* ev);
    void handleEnterNotify(XEvent* ev);
    void handleExpose(XEvent* ev);
    void handleFocusIn(XEvent* ev);
    void handleKeyPress(XEvent* ev);
    void handleMappingNotify(XEvent* ev);
    void handleMapRequest(XEvent* ev);
    void handleMotionNotify(XEvent* ev);
    void handlePropertyNotify(XEvent* ev);
    void handleUnmapNotify(XEvent* ev);
};

// Global instance
extern WindowManager* g_windowManager;

// Layout functions
void tileLayout(Monitor* m);
void monocleLayout(Monitor* m);

// Utility functions
void spawn(const char** cmd);
void quit(void* arg);
void grabKeys();
void grabButtons();
void updateNumlockMask();
void updateStatus();
void drawBar(Monitor* m);
void drawBars();