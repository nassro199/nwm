#pragma once

#include "nwm.h"

// Additional window management functions
void attachClient(Client* c);
void detachClient(Client* c);
void attachStack(Client* c);
void detachStack(Client* c);
void applyRules(Client* c);
void applySizeHints(Client* c, int* x, int* y, int* w, int* h, bool interact);
void updateClientList();
void updateTitle(Client* c);
void updateWindowType(Client* c);
void updateWMHints(Client* c);
void updateSizeHints(Client* c);
void setClientState(Client* c, long state);
void setFullscreen(Client* c, bool fullscreen);
int sendEvent(Client* c, Atom proto);
void resize(Client* c, int x, int y, int w, int h, bool interact);
void resizeClient(Client* c, int x, int y, int w, int h);
void resizemouse(Client* c);
void movemouse(Client* c);
void zoom(Client* c);
void pop(Client* c);
Client* nexttiled(Client* c);
