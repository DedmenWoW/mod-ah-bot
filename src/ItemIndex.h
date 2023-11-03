/*
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ITEM_INDEX_H
#define ITEM_INDEX_H

#include "ObjectGuid.h"
#include "ItemTemplate.h"
#include "AuctionHouseBotConfig.h"
#include "DatabaseEnvFwd.h"
#include <vector>
#include <unordered_set>

class AuctionHouseIndex
{
public:
    AuctionHouseIndex() = default;
    ~AuctionHouseIndex() = default;

    static AuctionHouseIndex* instance();

    bool InitializeItemsToSell();

    const std::vector<uint32>& GetItemBin(uint32 quality) const
    {
        return _itemsBin[quality];
    }

private:


    std::array<std::vector<uint32>, AHB_MAX_QUALITY> _itemsBin{};

};


#define sAHIndex AuctionHouseIndex::instance()

#endif
