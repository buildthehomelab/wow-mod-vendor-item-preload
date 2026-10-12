/*
 * mod-vendor-item-preload
 *
 * Makes vendor windows show every item name the first time they open.
 *
 * A vendor's list carries each item's icon and price, but not its name: that comes from the
 * client's item cache. For an item the cache doesn't have, a 3.3.5a client asks the server, and
 * the answer arrives after the window is drawn. Nothing draws the list again, so the row stays
 * nameless until the window is opened a second time. A player notices it on every vendor after
 * their Cache folder was emptied (a new install, or a launcher that clears it when the server's
 * data changes).
 *
 * Before a vendor's list goes out, this module sends the details of everything the vendor stocks,
 * and of the items its prices are paid in, the way the server answers the client's own request for
 * them. Each item is sent once per character login: after that the client has it.
 *
 * Released under the MIT License.
 */

#include "Config.h"
#include "Creature.h"
#include "DBCStores.h"
#include "DataMap.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <unordered_set>

namespace
{
    bool enabled = true;

    // What this character's client was already sent. Lives on the Player object, so it goes away
    // at logout, and only the thread handling that player's packets touches it.
    class SentItems : public DataMap::Base
    {
    public:
        std::unordered_set<uint32> items;
    };

    constexpr char const* SENT_ITEMS_KEY = "mod-vendor-item-preload";
}

class VendorItemPreloadPlayerScript : public PlayerScript
{
public:
    VendorItemPreloadPlayerScript() : PlayerScript("VendorItemPreloadPlayerScript", { PLAYERHOOK_ON_SEND_LIST_INVENTORY }) { }

    // Fires at the start of WorldSession::SendListInventory, before the list is built and sent.
    void OnPlayerSendListInventory(Player* player, ObjectGuid vendorGuid, uint32& vendorEntry) override
    {
        if (!enabled)
            return;

        WorldSession* session = player->GetSession();
        if (session->IsHeadless()) // a bot has no client to draw anything
            return;

        // The same stock the core is about to list: a vendor entry handed over by a gossip option,
        // otherwise the creature's own.
        VendorItemData const* stock = nullptr;
        if (vendorEntry)
            stock = sObjectMgr->GetNpcVendorItemList(vendorEntry);
        else if (Creature* vendor = player->GetNPCIfCanInteractWith(vendorGuid, UNIT_NPC_FLAG_VENDOR))
            stock = vendor->GetVendorItems();

        if (!stock)
            return;

        SentItems* sent = player->CustomData.GetDefault<SentItems>(SENT_ITEMS_KEY);
        auto sendDetails = [session, sent](uint32 itemId)
        {
            if (!itemId || !sObjectMgr->GetItemTemplate(itemId) || !sent->items.insert(itemId).second)
                return;

            WorldPacket request(CMSG_ITEM_QUERY_SINGLE, 4);
            request << uint32(itemId);
            session->HandleItemQuerySingleOpcode(request);
        };

        // Everything in stock, not only the rows this player will see: the core's own filters
        // (class, faction, conditions, sold out) stay in one place, and an extra item costs one
        // small packet once.
        for (uint8 slot = 0; slot < stock->GetItemCount(); ++slot)
        {
            VendorItem const* item = stock->GetItem(slot);
            if (!item)
                continue;

            sendDetails(item->item);

            // the window shows the icon of each item a price is paid in
            if (ItemExtendedCostEntry const* cost = sItemExtendedCostStore.LookupEntry(item->ExtendedCost))
                for (uint32 costItem : cost->reqitem)
                    sendDetails(costItem);
        }
    }
};

class VendorItemPreloadWorldScript : public WorldScript
{
public:
    VendorItemPreloadWorldScript() : WorldScript("VendorItemPreloadWorldScript", { WORLDHOOK_ON_AFTER_CONFIG_LOAD }) { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        enabled = sConfigMgr->GetOption<bool>("VendorItemPreload.Enable", true);
    }
};

void AddVendorItemPreloadScripts()
{
    new VendorItemPreloadPlayerScript();
    new VendorItemPreloadWorldScript();
}
