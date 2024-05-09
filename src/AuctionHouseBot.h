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

#ifndef AUCTION_HOUSE_BOT_H
#define AUCTION_HOUSE_BOT_H

#include "ObjectGuid.h"
#include "ItemTemplate.h"
#include "AuctionHouseBotConfig.h"
#include "DatabaseEnvFwd.h"
#include <vector>
#include <unordered_set>

struct AuctionEntry;
class Player;
class WorldSession;


enum class AHBotCommand : uint32
{
    ahexpire,
    ahexpireclass,
    minitems,
    maxitems,
    percentages,
    minprice,
    maxprice,
    minbidprice,
    maxbidprice,
    maxstack,
    buyerprice,
    bidinterval,
    bidsperinterval
};

class AuctionHouseBot
{
public:
    AuctionHouseBot();
    ~AuctionHouseBot() = default;

    static AuctionHouseBot* instance();

    void Update();
    void Initialize();
    void InitializeConfiguration();
    void LoadValues(AHBConfig*);
    void DecrementItemCounts(AuctionEntry* ah, uint32 itemEntry);
    void IncrementItemCounts(AuctionEntry* ah);
    void Commands(AHBotCommand, uint32, uint32, char*);
    ObjectGuid::LowType GetAHBplayerGUID() { return AHBplayerGUID; };

private:
    bool AHBSeller{ false };
    bool AHBBuyer{ false };
    bool BuyMethod{ false };
    bool SellMethod{ false };

    uint32 AHBplayerAccount;
    ObjectGuid::LowType AHBplayerGUID;
    uint32 ItemsPerCycle;

    AHBConfig AllianceConfig;
    AHBConfig HordeConfig;
    AHBConfig NeutralConfig;

    Seconds _lastUpdateAlliance{ 0s };
    Seconds _lastUpdateHorde{ 0s };
    Seconds _lastUpdateNeutral{ 0s };

    inline uint32 minValue(uint32 a, uint32 b) { return a <= b ? a : b; };
    void AddNewAuctions(Player* AHBplayer, AHBConfig* config);
    void AddNewAuctionBuyerBotBid(std::shared_ptr<Player> player, std::shared_ptr<WorldSession> session, AHBConfig* config);
    void AddNewAuctionBuyerBotBidCallback(std::shared_ptr<Player> player, std::shared_ptr<WorldSession> session, std::shared_ptr<AHBConfig> config, QueryResult result);

    void ProcessQueryCallbacks();

    QueryCallbackProcessor _queryProcessor;
};

#define sAHBot AuctionHouseBot::instance()

#endif
