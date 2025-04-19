#include "layout.h"
#include "window.h"
#include "nwm.h"
#include <X11/Xlib.h>

// Tile layout
void tileLayout(Monitor* m) {
    if (!m) return;
    
    unsigned int i, n;
    int mx, my, mw, mh;
    int sx, sy, sw, sh;
    float mfact;
    int nmaster;
    Client* c;
    
    // Count number of visible clients
    n = 0;
    for (i = 0; i < m->tags[m->selectedTag].clients.size(); i++) {
        c = m->tags[m->selectedTag].clients[i];
        if (!c->isfloating && !c->isfullscreen) {
            n++;
        }
    }
    
    if (n == 0) return;
    
    // Calculate master and stack areas
    mx = m->x;
    my = m->y;
    mw = m->width;
    mh = m->height;
    
    mfact = m->mfact;
    nmaster = m->nmaster;
    
    if (n > nmaster) {
        sw = mw * (1 - mfact);
        sx = mx + mw - sw;
        sh = mh;
        sy = my;
        
        mw = mw * mfact;
    }
    
    // Arrange clients
    int tx = mx, ty = my;
    int tw = mw, th = mh;
    
    for (i = 0, c = nullptr; i < m->tags[m->selectedTag].clients.size(); i++) {
        c = m->tags[m->selectedTag].clients[i];
        if (!c->isfloating && !c->isfullscreen) {
            if (i < (unsigned int)nmaster) {
                // Master area
                th = (mh - ty + my) / (MIN(n, nmaster) - i);
                resize(c, tx, ty, tw - 2 * c->bw, th - 2 * c->bw, false);
                ty += HEIGHT(c);
            } else {
                // Stack area
                sh = (mh - sy + my) / (n - i);
                resize(c, sx, sy, sw - 2 * c->bw, sh - 2 * c->bw, false);
                sy += HEIGHT(c);
            }
        }
    }
}

// Monocle layout
void monocleLayout(Monitor* m) {
    if (!m) return;
    
    unsigned int i;
    Client* c;
    
    for (i = 0; i < m->tags[m->selectedTag].clients.size(); i++) {
        c = m->tags[m->selectedTag].clients[i];
        if (!c->isfloating && !c->isfullscreen) {
            resize(c, m->x, m->y, m->width - 2 * c->bw, m->height - 2 * c->bw, false);
        }
    }
}

// Set layout
void setLayout(const LayoutType layout) {
    if (!g_windowManager) return;
    
    Monitor* m = g_windowManager->getCurrentMonitor();
    if (!m) return;
    
    m->previousLayout = m->currentLayout;
    m->currentLayout = layout;
    
    arrange(m);
}

// Toggle layout
void toggleLayout() {
    if (!g_windowManager) return;
    
    Monitor* m = g_windowManager->getCurrentMonitor();
    if (!m) return;
    
    LayoutType temp = m->currentLayout;
    m->currentLayout = m->previousLayout;
    m->previousLayout = temp;
    
    arrange(m);
}

// Arrange windows
void arrange(Monitor* m) {
    if (!g_windowManager) return;
    
    if (m) {
        showHide(m->tags[m->selectedTag].clients.size() > 0 ? 
                 m->tags[m->selectedTag].clients[0] : nullptr);
    } else {
        for (size_t i = 0; i < g_windowManager->monitors.size(); i++) {
            showHide(g_windowManager->monitors[i].tags[g_windowManager->monitors[i].selectedTag].clients.size() > 0 ? 
                     g_windowManager->monitors[i].tags[g_windowManager->monitors[i].selectedTag].clients[0] : nullptr);
        }
    }
    
    if (m) {
        arrangeMon(m);
        restack(m);
    } else {
        for (size_t i = 0; i < g_windowManager->monitors.size(); i++) {
            arrangeMon(&g_windowManager->monitors[i]);
            restack(&g_windowManager->monitors[i]);
        }
    }
}

// Arrange monitor
void arrangeMon(Monitor* m) {
    if (!m) return;
    
    switch (m->currentLayout) {
        case LayoutType::TILED:
            tileLayout(m);
            break;
        case LayoutType::FLOATING:
            // Do nothing for floating layout
            break;
        case LayoutType::MONOCLE:
            monocleLayout(m);
            break;
    }
}

// Restack windows
void restack(Monitor* m) {
    if (!m) return;
    
    // TODO: Implement window restacking
}

// Update bar position
void updateBarPos(Monitor* m) {
    if (!m) return;
    
    // TODO: Implement bar position update
}

// Update bars
void updateBars() {
    // TODO: Implement bars update
}

// Show/hide client
void showHide(Client* c) {
    if (!c) return;
    
    // TODO: Implement client show/hide
}
