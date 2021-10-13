// HARFANG(R) Copyright (C) 2021 Emmanuel Julien, NWNC HARFANG. Released under GPL/LGPL/Commercial Licence, see licence.txt for details.

#pragma once

#include "platform/window_system.h"

struct ANativeWindow;

namespace gs {

struct AndroidWindow : Window {
    ANativeWindow *handle = nullptr;
};

} // gs
