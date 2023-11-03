#include "ItemIndex.h"

#include <numeric>

#include "Config.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SmartEnum.h"

AuctionHouseIndex* AuctionHouseIndex::instance()
{
    static AuctionHouseIndex instance;
    return &instance;
}

struct ItemFilter
{
    bool SellMethod{ false };

    bool Vendor_Items{ false };
    bool Loot_Items{ false };
    bool Other_Items{ false };
    bool Vendor_TGs{ false };
    bool Loot_TGs{ false };
    bool Other_TGs{ false };

    bool No_Bind{ false };
    bool Bind_When_Picked_Up{ false };
    bool Bind_When_Equipped{ false };
    bool Bind_When_Use{ false };
    bool Bind_Quest_Item{ false };

    bool DisablePermEnchant{ false };
    bool DisableConjured{ false };
    bool DisableGems{ false };
    bool DisableMoney{ false };
    bool DisableMoneyLoot{ false };
    bool DisableLootable{ false };;
    bool DisableKeys{ false };
    bool DisableDuration{ false };
    bool DisableBOP_Or_Quest_NoReqLevel{ false };

    std::bitset<32> DisableClassItemsMask;

    uint32 DisableItemsBelowLevel{ 0 };
    uint32 DisableItemsAboveLevel{ 0 };
    uint32 DisableTGsBelowLevel{ 0 };
    uint32 DisableTGsAboveLevel{ 0 };
    uint32 DisableItemsBelowGUID{ 0 };
    uint32 DisableItemsAboveGUID{ 0 };
    uint32 DisableTGsBelowGUID{ 0 };
    uint32 DisableTGsAboveGUID{ 0 };
    uint32 DisableItemsBelowReqLevel{ 0 };
    uint32 DisableItemsAboveReqLevel{ 0 };
    uint32 DisableTGsBelowReqLevel{ 0 };
    uint32 DisableTGsAboveReqLevel{ 0 };
    uint32 DisableItemsBelowReqSkillRank{ 0 };
    uint32 DisableItemsAboveReqSkillRank{ 0 };
    uint32 DisableTGsBelowReqSkillRank{ 0 };
    uint32 DisableTGsAboveReqSkillRank{ 0 };

    std::unordered_set<uint32> disabledItems{};
    std::unordered_set<uint32> npcItems{};
    std::unordered_set<uint32> lootItems{};

    ItemFilter()
    {
        QueryResult results = WorldDatabase.Query("SELECT item FROM mod_auctionhousebot_disabled_items");

        if (results)
        {
            do
            {
                const Field* fields = results->Fetch();
                disabledItems.emplace(fields[0].Get<uint32>());
            } while (results->NextRow());
        }

        std::string npcQuery = "SELECT distinct item FROM npc_vendor";
        results = WorldDatabase.Query(npcQuery);
        if (results)
        {
            do
            {
                const Field* fields = results->Fetch();
                npcItems.emplace(fields[0].Get<int32>());
            } while (results->NextRow());
        }
        else
            LOG_ERROR("module.ahbot", "AuctionHouseBot: \"{}\" failed", npcQuery);

        std::string lootQuery = "SELECT item FROM creature_loot_template UNION "
            "SELECT item FROM reference_loot_template UNION "
            "SELECT item FROM disenchant_loot_template UNION "
            "SELECT item FROM fishing_loot_template UNION "
            "SELECT item FROM gameobject_loot_template UNION "
            "SELECT item FROM item_loot_template UNION "
            "SELECT item FROM milling_loot_template UNION "
            "SELECT item FROM pickpocketing_loot_template UNION "
            "SELECT item FROM prospecting_loot_template UNION "
            "SELECT item FROM skinning_loot_template";

        results = WorldDatabase.Query(lootQuery);
        if (results)
        {
            do
            {
                const Field* fields = results->Fetch();
                lootItems.emplace(fields[0].Get<uint32>());
            } while (results->NextRow());
        }
        else
            LOG_ERROR("module.ahbot", "AuctionHouseBot: \"{}\" failed", lootQuery);



        SellMethod = sConfigMgr->GetOption<bool>("AuctionHouseBot.UseBuyPriceForSeller", false);

        // Begin Filters

        Vendor_Items = sConfigMgr->GetOption<bool>("AuctionHouseBot.VendorItems", false);
        Loot_Items = sConfigMgr->GetOption<bool>("AuctionHouseBot.LootItems", true);
        Other_Items = sConfigMgr->GetOption<bool>("AuctionHouseBot.OtherItems", false);
        Vendor_TGs = sConfigMgr->GetOption<bool>("AuctionHouseBot.VendorTradeGoods", false);
        Loot_TGs = sConfigMgr->GetOption<bool>("AuctionHouseBot.LootTradeGoods", true);
        Other_TGs = sConfigMgr->GetOption<bool>("AuctionHouseBot.OtherTradeGoods", false);

        No_Bind = sConfigMgr->GetOption<bool>("AuctionHouseBot.No_Bind", true);
        Bind_When_Picked_Up = sConfigMgr->GetOption<bool>("AuctionHouseBot.Bind_When_Picked_Up", false);
        Bind_When_Equipped = sConfigMgr->GetOption<bool>("AuctionHouseBot.Bind_When_Equipped", true);
        Bind_When_Use = sConfigMgr->GetOption<bool>("AuctionHouseBot.Bind_When_Use", true);
        Bind_Quest_Item = sConfigMgr->GetOption<bool>("AuctionHouseBot.Bind_Quest_Item", false);

        DisablePermEnchant = sConfigMgr->GetOption<bool>("AuctionHouseBot.DisablePermEnchant", false);
        DisableConjured = sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableConjured", false);
        DisableGems = sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableGems", false);
        DisableMoney = sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableMoney", false);
        DisableMoneyLoot = sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableMoneyLoot", false);
        DisableLootable = sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableLootable", false);
        DisableKeys = sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableKeys", false);
        DisableDuration = sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableDuration", false);
        DisableBOP_Or_Quest_NoReqLevel = sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableBOP_Or_Quest_NoReqLevel", false);

        // Classes are 1 based index, to get their flag bit we need zero based, so -1
        DisableClassItemsMask.set(Classes::CLASS_WARRIOR - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableWarriorItems", false));
        DisableClassItemsMask.set(Classes::CLASS_PALADIN - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisablePaladinItems", false));
        DisableClassItemsMask.set(Classes::CLASS_HUNTER - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableHunterItems", false));
        DisableClassItemsMask.set(Classes::CLASS_ROGUE - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableRogueItems", false));
        DisableClassItemsMask.set(Classes::CLASS_PRIEST - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisablePriestItems", false));
        DisableClassItemsMask.set(Classes::CLASS_DEATH_KNIGHT - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableDKItems", false));
        DisableClassItemsMask.set(Classes::CLASS_SHAMAN - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableShamanItems", false));
        DisableClassItemsMask.set(Classes::CLASS_MAGE - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableMageItems", false));
        DisableClassItemsMask.set(Classes::CLASS_WARLOCK - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableWarlockItems", false));
        DisableClassItemsMask.set(10 - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableUnusedClassItems", false));
        DisableClassItemsMask.set(Classes::CLASS_DRUID - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableDruidItems", false));
        DisableClassItemsMask.set(Classes::CLASS_WARRIOR - 1, sConfigMgr->GetOption<bool>("AuctionHouseBot.DisableWarriorItems", false));

        DisableItemsBelowLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsBelowLevel", 0);
        DisableItemsAboveLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsAboveLevel", 0);
        DisableTGsBelowLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsBelowLevel", 0);
        DisableTGsAboveLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsAboveLevel", 0);
        DisableItemsBelowGUID = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsBelowGUID", 0);
        DisableItemsAboveGUID = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsAboveGUID", 0);
        DisableTGsBelowGUID = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsBelowGUID", 0);
        DisableTGsAboveGUID = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsAboveGUID", 0);
        DisableItemsBelowReqLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsBelowReqLevel", 0);
        DisableItemsAboveReqLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsAboveReqLevel", 0);
        DisableTGsBelowReqLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsBelowReqLevel", 0);
        DisableTGsAboveReqLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsAboveReqLevel", 0);
        DisableItemsBelowReqSkillRank = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsBelowReqSkillRank", 0);
        DisableItemsAboveReqSkillRank = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsAboveReqSkillRank", 0);
        DisableTGsBelowReqSkillRank = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsBelowReqSkillRank", 0);
        DisableTGsAboveReqSkillRank = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsAboveReqSkillRank", 0);
    }

    bool IsAccepted(const ItemTemplate& itemTemplate) const
    {
        switch (itemTemplate.Bonding)
        {
        case NO_BIND:
            if (!No_Bind)
                return false;
            break;
        case BIND_WHEN_PICKED_UP:
            if (!Bind_When_Picked_Up)
                return false;
            break;
        case BIND_WHEN_EQUIPED:
            if (!Bind_When_Equipped)
                return false;
            break;
        case BIND_WHEN_USE:
            if (!Bind_When_Use)
                return false;
            break;
        case BIND_QUEST_ITEM:
            if (!Bind_Quest_Item)
                return false;
            break;
        default:
            return false;
        }

        if (SellMethod)
        {
            if (!itemTemplate.BuyPrice)
                return false;
        }
        else
        {
            if (!itemTemplate.SellPrice)
                return false;
        }

        if (itemTemplate.Quality > ITEM_QUALITY_ARTIFACT)
            return false;

        auto isVendorItem = [this](const ItemTemplate& itemTemplate)
            {
                return npcItems.contains(itemTemplate.ItemId);
            };

        auto isLootItem = [this](const ItemTemplate& itemTemplate)
            {
                return lootItems.contains(itemTemplate.ItemId);
            };

        if (itemTemplate.Class != ITEM_CLASS_TRADE_GOODS)
        {
            // Item checks

            if (!Vendor_Items)
            {
                if (isVendorItem(itemTemplate))
                    return false;
            }

            if (!Loot_Items)
            {
                if (isLootItem(itemTemplate))
                    return false;
            }

            if (!Other_Items)
            {
                if (!isLootItem(itemTemplate) && !isVendorItem(itemTemplate))
                    return false;
            }
        }
        else if (itemTemplate.Class == ITEM_CLASS_TRADE_GOODS)
        {
            // Tradegood checks

            if (!Vendor_TGs)
            {
                if (isVendorItem(itemTemplate))
                    return false;
            }

            if (!Loot_TGs)
            {
                if (isLootItem(itemTemplate))
                    return false;
            }

            if (!Other_TGs)
            {
                if (!isLootItem(itemTemplate) && !isVendorItem(itemTemplate))
                    return false;
            }
        }


        // Disable items by Id
        if (disabledItems.contains(itemTemplate.ItemId))
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (PTR/Beta/Unused Item)", itemTemplate.ItemId);
            return false;
        }

        // Disable permanent enchants items
        if (DisablePermEnchant && itemTemplate.Class == ITEM_CLASS_PERMANENT)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Permanent Enchant Item)", itemTemplate.ItemId);
            return false;
        }

        // Disable conjured items
        if (DisableConjured && itemTemplate.IsConjuredConsumable())
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Conjured Consumable)", itemTemplate.ItemId);
            return false;
        }

        // Disable gems
        if (DisableGems && itemTemplate.Class == ITEM_CLASS_GEM)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Gem)", itemTemplate.ItemId);
            return false;
        }

        // Disable money
        if (DisableMoney && itemTemplate.Class == ITEM_CLASS_MONEY)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Money)", itemTemplate.ItemId);
            return false;
        }

        // Disable moneyloot
        if (DisableMoneyLoot && itemTemplate.MinMoneyLoot)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (MoneyLoot)", itemTemplate.ItemId);
            return false;
        }

        // Disable lootable items
        if (DisableLootable && itemTemplate.Flags & 4)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Lootable Item)", itemTemplate.ItemId);
            return false;
        }

        // Disable Keys
        if (DisableKeys && itemTemplate.Class == ITEM_CLASS_KEY)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Quest Item)", itemTemplate.ItemId);
            return false;
        }

        // Disable items with duration
        if (DisableDuration && itemTemplate.Duration)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Has a Duration)", itemTemplate.ItemId);
            return false;
        }

        // Disable items which are BOP or Quest Items and have a required level lower than the item level
        if (DisableBOP_Or_Quest_NoReqLevel && ((itemTemplate.Bonding == BIND_WHEN_PICKED_UP || itemTemplate.Bonding == BIND_QUEST_ITEM) && (itemTemplate.RequiredLevel < itemTemplate.ItemLevel)))
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (BOP or BQI and Required Level is less than Item Level)", itemTemplate.ItemId);
            return false;
        }


        // We have disabled some class-specific items, lets see if this is one of them
        if (DisableClassItemsMask.any())
        {
            const std::bitset<32> allowableClass(itemTemplate.AllowableClass);

            // If this item is specific for just one class (as opposed to multiple classes), then check if its a class that we have disabled
            if (allowableClass.count() == 1)
            {
                if ((DisableClassItemsMask & allowableClass).any())
                {
                    // slightly ugly, find index of first bit that is set, to find out which class this is

                    Classes itemAllowClass = Classes::CLASS_NONE;
                    for (uint8 classBit = 0; classBit < MAX_CLASSES; ++classBit)
                        if (allowableClass.test(classBit))
                            itemAllowClass = static_cast<Classes>(classBit + 1); // Zero based index back to 1 based

                    LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled ({} Item)", itemTemplate.ItemId, Acore::Impl::EnumUtilsImpl::EnumUtils<Classes>::ToString(itemAllowClass).Title);
                    return false;
                }
            }
        }

        if (itemTemplate.Class != ITEM_CLASS_TRADE_GOODS)
        {
            // Item filters

            // Disable Items below level X
            if (DisableItemsBelowLevel && itemTemplate.ItemLevel < DisableItemsBelowLevel)
            {
                LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Item Level = {})", itemTemplate.ItemId, itemTemplate.ItemLevel);
                return false;
            }

            // Disable Items above level X
            if (DisableItemsAboveLevel && itemTemplate.ItemLevel > DisableItemsAboveLevel)
            {
                LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Item Level = {})", itemTemplate.ItemId, itemTemplate.ItemLevel);
                return false;
            }

            // Disable Items below GUID X
            if (DisableItemsBelowGUID && itemTemplate.ItemId < DisableItemsBelowGUID)
            {
                LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Item Level = {})", itemTemplate.ItemId, itemTemplate.ItemLevel);
                return false;
            }

            // Disable Items above GUID X
            if (DisableItemsAboveGUID && itemTemplate.ItemId > DisableItemsAboveGUID)
            {
                LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Item Level = {})", itemTemplate.ItemId, itemTemplate.ItemLevel);
                return false;
            }
        }
        else
        {
            // TradeGood filters

            // Disable Trade Goods below level X
            if (DisableTGsBelowLevel && itemTemplate.ItemLevel < DisableTGsBelowLevel)
            {
                LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Trade Good {} disabled (Trade Good Level = {})", itemTemplate.ItemId, itemTemplate.ItemLevel);
                return false;
            }

            // Disable Trade Goods above level X
            if (DisableTGsAboveLevel && itemTemplate.ItemLevel > DisableTGsAboveLevel)
            {
                LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Trade Good {} disabled (Trade Good Level = {})", itemTemplate.ItemId, itemTemplate.ItemLevel);
                return false;
            }

            // Disable Trade Goods below GUID X
            if (DisableTGsBelowGUID && itemTemplate.ItemId < DisableTGsBelowGUID)
            {
                LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Trade Good Level = {})", itemTemplate.ItemId, itemTemplate.ItemLevel);
                return false;
            }

            // Disable Trade Goods above GUID X
            if (DisableTGsAboveGUID && itemTemplate.ItemId > DisableTGsAboveGUID)
            {
                LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (Trade Good Level = {})", itemTemplate.ItemId, itemTemplate.ItemLevel);
                return false;
            }
        }

        // Disable Items for level lower than X
        if (DisableItemsBelowReqLevel && itemTemplate.RequiredLevel < DisableItemsBelowReqLevel)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (RequiredLevel = {})", itemTemplate.ItemId, itemTemplate.RequiredLevel);
            return false;
        }

        // Disable Items for level higher than X
        if (DisableItemsAboveReqLevel && itemTemplate.RequiredLevel > DisableItemsAboveReqLevel)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (RequiredLevel = {})", itemTemplate.ItemId, itemTemplate.RequiredLevel);
            return false;
        }

        // Disable Trade Goods for level lower than X
        if (DisableTGsBelowReqLevel && itemTemplate.RequiredLevel < DisableTGsBelowReqLevel)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Trade Good {} disabled (RequiredLevel = {})", itemTemplate.ItemId, itemTemplate.RequiredLevel);
            return false;
        }

        // Disable Trade Goods for level higher than X
        if (DisableTGsAboveReqLevel && itemTemplate.RequiredLevel > DisableTGsAboveReqLevel)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Trade Good {} disabled (RequiredLevel = {})", itemTemplate.ItemId, itemTemplate.RequiredLevel);
            return false;
        }

        // Disable Items that require skill lower than X
        if (DisableItemsBelowReqSkillRank && itemTemplate.RequiredSkillRank < DisableItemsBelowReqSkillRank)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (RequiredSkillRank = {})", itemTemplate.ItemId, itemTemplate.RequiredSkillRank);
            return false;
        }

        // Disable Items that require skill higher than X
        if (DisableItemsAboveReqSkillRank && itemTemplate.RequiredSkillRank > DisableItemsAboveReqSkillRank)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (RequiredSkillRank = {})", itemTemplate.ItemId, itemTemplate.RequiredSkillRank);
            return false;
        }

        // Disable Trade Goods that require skill lower than X
        if (DisableTGsBelowReqSkillRank && itemTemplate.RequiredSkillRank < DisableTGsBelowReqSkillRank)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (RequiredSkillRank = {})", itemTemplate.ItemId, itemTemplate.RequiredSkillRank);
            return false;
        }

        // Disable Trade Goods that require skill higher than X
        if (DisableTGsAboveReqSkillRank && itemTemplate.RequiredSkillRank > DisableTGsAboveReqSkillRank)
        {
            LOG_DEBUG("module.ahbot.filters", "AuctionHouseBot: Item {} disabled (RequiredSkillRank = {})", itemTemplate.ItemId, itemTemplate.RequiredSkillRank);
            return false;
        }

        return true;
    }
};



bool AuctionHouseIndex::InitializeItemsToSell()
{
    const ItemFilter filter;


    for (auto const& [itemID, itemTemplate] : *sObjectMgr->GetItemTemplateStore())
    {
        WPAssert(itemTemplate.ItemId, "ItemID cannot be zero");

        if (!filter.IsAccepted(itemTemplate))
            continue;

        const uint32 itemQualityIndexStart = itemTemplate.Class == ITEM_CLASS_TRADE_GOODS ? 0 : AHB_DEFAULT_QUALITY_SIZE;
        _itemsBin[itemQualityIndexStart + itemTemplate.Quality].emplace_back(itemTemplate.ItemId);
    }

    std::size_t totalItems = std::accumulate(_itemsBin.begin(), _itemsBin.end(), 0u, [](const std::size_t c, const std::vector<uint32>& v) {return c + v.size(); });

    if (!totalItems)
    {
        LOG_ERROR("module.ahbot", "AuctionHouseBot: No items");
        return false;
    }

    LOG_INFO("module.ahbot", "AuctionHouseBot:");
    LOG_INFO("module.ahbot", "{} disabled items", filter.disabledItems.size());
    LOG_INFO("module.ahbot", "Loaded {} grey trade goods", _itemsBin[ITEM_QUALITY_POOR].size());
    LOG_INFO("module.ahbot", "Loaded {} white trade goods", _itemsBin[ITEM_QUALITY_NORMAL].size());
    LOG_INFO("module.ahbot", "Loaded {} green trade goods", _itemsBin[ITEM_QUALITY_UNCOMMON].size());
    LOG_INFO("module.ahbot", "Loaded {} blue trade goods", _itemsBin[ITEM_QUALITY_RARE].size());
    LOG_INFO("module.ahbot", "Loaded {} purple trade goods", _itemsBin[ITEM_QUALITY_EPIC].size());
    LOG_INFO("module.ahbot", "Loaded {} orange trade goods", _itemsBin[ITEM_QUALITY_LEGENDARY].size());
    LOG_INFO("module.ahbot", "Loaded {} yellow trade goods", _itemsBin[ITEM_QUALITY_ARTIFACT].size());
    LOG_INFO("module.ahbot", "Loaded {} grey items", _itemsBin[AHB_ITEM_QUALITY_POOR].size());
    LOG_INFO("module.ahbot", "Loaded {} white items", _itemsBin[AHB_ITEM_QUALITY_NORMAL].size());
    LOG_INFO("module.ahbot", "Loaded {} green items", _itemsBin[AHB_ITEM_QUALITY_UNCOMMON].size());
    LOG_INFO("module.ahbot", "Loaded {} blue items", _itemsBin[AHB_ITEM_QUALITY_RARE].size());
    LOG_INFO("module.ahbot", "Loaded {} purple items", _itemsBin[AHB_ITEM_QUALITY_EPIC].size());
    LOG_INFO("module.ahbot", "Loaded {} orange items", _itemsBin[AHB_ITEM_QUALITY_LEGENDARY].size());
    LOG_INFO("module.ahbot", "Loaded {} yellow items", _itemsBin[AHB_ITEM_QUALITY_ARTIFACT].size());

    return true;
    /*
     AuctionHouseBot:
    5957 disabled items
    loaded 5 grey trade goods
    loaded 471 white trade goods
    loaded 58 green trade goods
    loaded 26 blue trade goods
    loaded 3 purple trade goods
    loaded 1 orange trade goods
    loaded 0 yellow trade goods
    loaded 1633 grey items
    loaded 2771 white items
    loaded 5520 green items
    loaded 1645 blue items
    loaded 809 purple items
    loaded 0 orange items
    loaded 1 yellow items
    */

}

