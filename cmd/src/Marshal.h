//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#pragma once

#include <utu/PartialData.h>
#include <loris/PartialList.h>

struct Marshal {
    static utu::PartialData from(const Loris::PartialList& p);
    static Loris::PartialList from(const utu::PartialData& p);
};