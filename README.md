# Vendor Item Preload

An [AzerothCore](https://www.azerothcore.org/) (WotLK 3.3.5a) module that makes vendor windows
show every item name the first time they open.

A vendor's list tells the client each item's icon and price, but not its name. The name comes from
the client's item cache (`Cache/WDB`). For an item the cache doesn't have, the 3.3.5a client asks
the server, and the answer arrives after the window is already drawn. Nothing draws the list
again, so those rows stay nameless until the player closes the window and opens it a second time.

Players run into it on every vendor right after their `Cache` folder was emptied: a new install,
a cache deleted by hand, or a launcher that clears it when the server's data changes (see
[mod-cache-version](https://github.com/buildthehomelab/wow-mod-cache-version)).

With this module:

- **Before a vendor's list goes out, the server sends the details of everything the vendor
  stocks**, in the same form as its answer to the client's own request. The client has the names
  by the time it draws the window.
- **The items prices are paid in are sent too** (badges, tokens, coins), so their icons show beside
  the prices.
- **Each item is sent once per character login.** After that the client has it, and opening the
  same vendor again costs nothing extra.
- **Every vendor is covered**, including vendor pages opened from a gossip option.
- **Bots are skipped.** They have no window to draw.
- **Server-side only.** No addon and no client patch.

## Patch Notes: Vendor Item Preload

Category: User Interface

- Vendor windows now show every item's name the first time you open them. Previously some rows
  stayed blank after the game's cache was cleared, until you closed and reopened the window.

## Requirements

- AzerothCore WotLK (tested against the mod-playerbots branch at `2a2211c`). Needs a core with
  `WorldSession::IsHeadless()`.

## Installation

1. Clone into the server's `modules/` folder as `mod-vendor-item-preload` and rebuild:

   ```bash
   cd modules && git clone https://github.com/buildthehomelab/wow-mod-vendor-item-preload.git mod-vendor-item-preload
   ```

2. Copy `conf/mod_vendor_item_preload.conf.dist` to `mod_vendor_item_preload.conf` in your config
   folder. The default works as-is.

There is no SQL.

## Configuration

| Option | Default | What it does |
| --- | --- | --- |
| `VendorItemPreload.Enable` | `1` | Master switch. |

## Cost

The first time a character opens a vendor, the server sends one item packet (up to about 600
bytes) for each item the vendor stocks, at most 255. Items already sent since that character
logged in are skipped, so a second visit, or another vendor with the same goods, sends nothing.

The module sends a vendor's whole stock, not only the rows the player will see. A class-filtered
vendor therefore sends the other classes' items too, once. That keeps the core's own list filters
in one place.

## Troubleshooting

- **A name is still blank:** check that the module is loaded (`.server debug` lists modules) and
  that `VendorItemPreload.Enable` is `1`. An item with no row in `item_template` has no name to
  send.
- **Blank prices or a question-mark currency icon:** that is the client missing an
  `ItemExtendedCost.dbc` or `Item.dbc` row from a client patch, not something this module sends.

## Credits

Written for the Vaultrona / Evermore realm.

## License

MIT, see [LICENSE](LICENSE).
