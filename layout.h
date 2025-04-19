#pragma once

#include "nwm.h"

// Layout functions
void tileLayout(Monitor* m);
void monocleLayout(Monitor* m);

// Layout management functions
void setLayout(const LayoutType layout);
void toggleLayout();
void arrange(Monitor* m = nullptr);
void arrangeMon(Monitor* m);
void restack(Monitor* m);
void updateBarPos(Monitor* m);
void updateBars();
void showHide(Client* c);
